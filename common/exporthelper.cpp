/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "exporthelper.h"

#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QSqlError>
#include <QTemporaryFile>

#include "metasql.h"
#include "mqlutil.h"
#include "xsqlquery.h"

#define DEBUG true

/** \brief Export the results of a query set to an XML file.

  Run all of the queries in the given Query Set in the given order and write
  the results to an XML file. The XML file is constructed very simply:
  <tablename>
    <column1name>columnvalue</column1name>
    <column2name>columnvalue</column2name>
    ...
  </tablename>

  If the caller passes in an XSLT map id, the simple XML will be processed
  using the export XSLT.

  \param qryheadid   The internal ID of the query set (qryhead record) to run.
  \param params      A list of parameters and values to use when building SQL
                     statements from MetaSQL statements.
  \param filename[in,out] The name of the file to create. If passed in empty,
                          a file named after the query set will be created
                          in the current directory (context-dependent) and
                          this filename will be passed back out.
  \param errmsg[out] An message describing why the processing failed if there
                     was a problem.
  \param xsltmapid   An optional parameter. If this is set, it should be
                     the internal ID of an xsltmap record. The xsltmap_export
                     field of this record and the XSLTDefaultDir will be used
                     to find the XSLT script to run on the generated XML.
  */
bool ExportHelper::exportXML(const int qryheadid, ParameterList &params, QString &filename, QString &errmsg, const int xsltmapid)
{
  bool returnVal = false;

  XSqlQuery setq;
  setq.prepare("SELECT * FROM qryhead WHERE qryhead_id=:id;");
  setq.bindValue(":id", qryheadid);
  setq.exec();
  if (setq.first())
  {
    if (filename.isEmpty())
    {
      QFileInfo fileinfo(setq.value("qryhead_name").toString());
      filename = fileinfo.absoluteFilePath();
    }

    QTemporaryFile exportfile(filename);
    exportfile.setAutoRemove(false);
    if (! exportfile.open())
      errmsg = tr("Could not open %1 (%2).").arg(filename,exportfile.error());
    else
    {
      QDomDocument xmldoc("xtupleimport");
      QDomElement rootelem = xmldoc.createElement("xtupleimport");
      xmldoc.appendChild(rootelem);

      XSqlQuery itemq;
      QString tableElemName;
      QString schemaName;
      itemq.prepare("SELECT * FROM qryitem WHERE qryitem_qryhead_id=:id;");
      itemq.bindValue(":id", qryheadid);
      itemq.exec();
      while (itemq.next())
      {
        QString qtext;
        tableElemName = itemq.value("qryitem_name").toString();
        if (itemq.value("qryitem_src").toString() == "REL")
        {
          schemaName = itemq.value("qryitem_group").toString();
          qtext = "SELECT * FROM " +
                  (schemaName.isEmpty() ? QString("") : schemaName + QString(".")) +
                  itemq.value("qryitem_detail").toString();
        }
        else if (itemq.value("qryitem_src").toString() == "MQL")
        {
          QString tmpmsg;
          bool valid;
          qtext = MQLUtil::mqlLoad(itemq.value("qryitem_group").toString(),
                                   itemq.value("qryitem_detail").toString(),
                                   tmpmsg, &valid);
          if (! valid)
            errmsg = tmpmsg;
        }
        else if (itemq.value("qryitem_src").toString() == "CUSTOM")
          qtext = itemq.value("qryitem_detail").toString();

        if (! qtext.isEmpty())
        {
          MetaSQLQuery mql(qtext);
          XSqlQuery qry = mql.toQuery(params);
          if (qry.first())
          {
            do {
              QDomElement tableElem = xmldoc.createElement(tableElemName);

              if (DEBUG)
                qDebug("exportXML starting %s", qPrintable(tableElemName));
              if (! schemaName.isEmpty())
                tableElem.setAttribute("schema", schemaName);
              for (int i = 0; i < qry.record().count(); i++)
              {
                QDomElement fieldElem = xmldoc.createElement(qry.record().fieldName(i));
                fieldElem.appendChild(xmldoc.createTextNode(qry.record().value(i).toString()));
                tableElem.appendChild(fieldElem);
                if (DEBUG)
                  qDebug("exportXML added %s %s",
                         qPrintable(tableElem.nodeName()),
                         qPrintable(tableElem.nodeValue()));
              }
              rootelem.appendChild(tableElem);
            } while (qry.next());
          }
          if (qry.lastError().type() != QSqlError::NoError)
            errmsg = qry.lastError().text();
        }
      }
      if (itemq.lastError().type() != QSqlError::NoError)
        errmsg = itemq.lastError().text();

      exportfile.write(xmldoc.toString());
      exportfile.close();

      if (xsltmapid < 0)
      {
        // rm pre-existing files with the desired name then rename the tmp file
        QFile newFile(filename);
        returnVal = (! newFile.exists() || newFile.remove()) &&
                    exportfile.rename(filename);
        if (! returnVal)
          errmsg = tr("Could not rename temporary file to %1 (%2).")
                    .arg(filename).arg(exportfile.error());
      }
      else
      {
        returnVal = XSLTConvert(QFileInfo(exportfile).absoluteFilePath(),
                                filename, xsltmapid, errmsg);
        if (returnVal)
          exportfile.remove();
      }
    }
  }
  else if (setq.lastError().type() != QSqlError::NoError)
    errmsg = setq.lastError().text();
  else
    errmsg = tr("<p>Cannot export data because the query set with "
                "id %1 was not found.").arg(qryheadid);

  return returnVal;
}

bool ExportHelper::XSLTConvert(QString inputfilename, QString outputfilename, int xsltmapid, QString &errmsg)
{
  bool returnVal = false;

  XSqlQuery xsltq;
  xsltq.prepare("SELECT xsltmap_export, fetchMetricText(:xsltdir) AS dir,"
                "       fetchMetricText(:xsltproc) AS proc"
                "  FROM xsltmap"
                " WHERE xsltmap_id=:id;");

#if defined Q_WS_MACX
  xsltq.bindValue(":xsltdir",  "XSLTDefaultDirMac");
  xsltq.bindValue(":xsltproc", "XSLTProcessorMac");
#elif defined Q_WS_WIN
  xsltq.bindValue(":xsltdir",  "XSLTDefaultDirWindows");
  xsltq.bindValue(":xsltproc", "XSLTProcessorWindows");
#elif defined Q_WS_X11
  xsltq.bindValue(":xsltdir",  "XSLTDefaultDirLinux");
  xsltq.bindValue(":xsltproc", "XSLTProcessorLinux");
#endif

  xsltq.bindValue(":id", xsltmapid);
  xsltq.exec();
  if (xsltq.first())
  {
    QStringList args = xsltq.value("proc").toString().split(" ", QString::SkipEmptyParts);
    QString xsltfile = xsltq.value("xsltmap_export").toString();
    QString defaultXSLTDir = xsltq.value("dir").toString();
    QString command = args[0];
    args.removeFirst();
    args.replaceInStrings("%f", inputfilename);
    if (QFile::exists(xsltfile))
      args.replaceInStrings("%x", xsltfile);
    else if (QFile::exists(defaultXSLTDir + QDir::separator() + xsltfile))
      args.replaceInStrings("%x", defaultXSLTDir + QDir::separator() + xsltfile);
    else
    {
      errmsg = tr("Cannot find the XSLT file as either %1 or %2")
                  .arg(xsltfile, defaultXSLTDir + QDir::separator() + xsltfile);
      return false;
    }

    QProcess xslt;
    xslt.setStandardOutputFile(outputfilename);
    xslt.start(command, args);
    QString commandline = command + " " + args.join(" ");
    errmsg = "";
    if (! xslt.waitForStarted())
      errmsg = tr("Error starting XSLT Processing: %1\n%2")
                        .arg(commandline)
                        .arg(QString(xslt.readAllStandardError()));
    if (! xslt.waitForFinished())
      errmsg = tr("The XSLT Processor encountered an error: %1\n%2")
                        .arg(commandline)
                        .arg(QString(xslt.readAllStandardError()));
    if (xslt.exitStatus() !=  QProcess::NormalExit)
      errmsg = tr("The XSLT Processor did not exit normally: %1\n%2")
                        .arg(commandline)
                        .arg(QString(xslt.readAllStandardError()));
    if (xslt.exitCode() != 0)
      errmsg = tr("The XSLT Processor returned an error code: %1\nreturned %2\n%3")
                        .arg(commandline)
                        .arg(xslt.exitCode())
                        .arg(QString(xslt.readAllStandardError()));

    returnVal = errmsg.isEmpty();
  }
  else  if (xsltq.lastError().type() != QSqlError::NoError)
    errmsg = xsltq.lastError().text();
  else
    errmsg = tr("Could not find XSLT mapping with internal id %1.")
               .arg(xsltmapid);

  return returnVal;
}
