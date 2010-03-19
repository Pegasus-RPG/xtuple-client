/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "importhelper.h"

#include <QDate>
#include <QDateTime>
#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QScriptEngine>
#include <QScriptValue>
#include <QSqlError>
#include <QTemporaryFile>
#include <QVariant>

#include <xsqlquery.h>

#include "exporthelper.h"

#define DEBUG false

bool ImportHelper::importXML(const QString &pFileName, QString &errmsg)
{
  QString xmldir;
  QString xsltdir;
  QString xsltcmd;
  QString xmlsuccessdir;
  QString xmlsuccesssuffix;
  QString xmlsuccesstreatment;

  XSqlQuery q;
  q.prepare("SELECT fetchMetricText(:xmldir)  AS xmldir,"
            "       fetchMetricText('XMLSuccessDir') AS successdir,"
            "       fetchMetricText('XMLSuccessSuffix') AS successsuffix,"
            "       fetchMetricText('XMLSuccessTreatment') AS successtreatment,"
            "       fetchMetricText(:xsltdir) AS xsltdir,"
            "       fetchMetricText(:xsltcmd) AS xsltcmd;");
#if defined Q_WS_MACX
  q.bindValue(":xmldir",  "XMLDefaultDirMac");
  q.bindValue(":xsltdir", "XSLTDefaultDirMac");
  q.bindValue(":xsltcmd", "XSLTProcessorMac");
#elif defined Q_WS_WIN
  q.bindValue(":xmldir",  "XMLDefaultDirWindows");
  q.bindValue(":xsltdir", "XSLTDefaultDirWindows");
  q.bindValue(":xsltcmd", "XSLTProcessorWindows");
#elif defined Q_WS_X11
  q.bindValue(":xmldir",  "XMLDefaultDirLinux");
  q.bindValue(":xsltdir", "XSLTDefaultDirLinux");
  q.bindValue(":xsltcmd", "XSLTProcessorLinux");
#endif
  q.exec();
  if (q.first())
  {
    xmldir  = q.value("xmldir").toString();
    xsltdir = q.value("xsltdir").toString();
    xsltcmd = q.value("xsltcmd").toString();
    xmlsuccessdir       = q.value("successdir").toString();
    xmlsuccesssuffix    = q.value("successsuffix").toString();
    xmlsuccesstreatment = q.value("successtreatment").toString();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    errmsg = q.lastError().text();
    return false;
  }
  else
  {
    errmsg = tr("Could not find the XSLT directory and command metrics.");
    return false;
  }

  if (xmldir.isEmpty())
    xmldir = ".";

  QDomDocument doc(pFileName);
  if (!openDomDocument(pFileName, doc, errmsg))
    return false;

  QString tmpfileName; // only set if we translate the file with XSLT
  if (DEBUG)
      qDebug("importXML::importOne(%s) doctype = %s",
             qPrintable(pFileName), qPrintable(doc.doctype().name()));
  if (doc.doctype().name() != "xtupleimport")
  {
    QString xsltfile;
    XSqlQuery q;
    q.prepare("SELECT xsltmap_import FROM xsltmap "
              "WHERE ((xsltmap_doctype=:doctype OR xsltmap_doctype='')"
              "   AND (xsltmap_system=:system   OR xsltmap_system=''));");
    q.bindValue(":doctype", doc.doctype().name());
    q.bindValue(":system", doc.doctype().systemId());
    q.exec();
    if (q.first())
      xsltfile = q.value("xsltmap_import").toString();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      errmsg = q.lastError().databaseText();
      return false;
    }
    else
    {
      errmsg = tr("<p>Could not find a map for doctype '%1' and system id '%2'"
                  ". Write an XSLT stylesheet to convert this to valid xtuple "
                  "import XML and add it to the Map of XSLT Import Filters.")
                    .arg(doc.doctype().name(), doc.doctype().systemId());
      return false;
    }

    tmpfileName = xmldir + QDir::separator() +
                  doc.doctype().name() + "TOxtupleimport";

    if (! ExportHelper::XSLTConvertFile(pFileName, tmpfileName,
                                        q.value("xsltmap_import").toString(),
                                        errmsg))
      return false;

    if (! openDomDocument(tmpfileName, doc, errmsg))
      return false;
  }

  /* xtupleimport format is very straightforward:
      top level element is xtupleimport
        second level elements are all table/view names (default to api schema)
          third level elements are all column names
     and there are no text nodes until third level

     wrap the import of an entire file in a single transaction so
     we can reimport files which have failures. however, if a
     view-level element has the ignore attribute set to true then
     rollback just that view-level element if it generates an error.
  */

  // the silent attribute provides the user the option to turn off 
  // the interactive message for the view-level element


  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    errmsg = q.lastError().databaseText();
    return false;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QRegExp apos("\\\\?'");

  for (QDomElement viewElem = doc.documentElement().firstChildElement();
       ! viewElem.isNull();
       viewElem = viewElem.nextSiblingElement())
  {
    QStringList columnNameList;
    QStringList columnValueList;

    bool ignoreErr = (viewElem.attribute("ignore", "false").isEmpty() ||
                      viewElem.attribute("ignore", "false") == "true");

    bool silent = (viewElem.attribute("silent", "false").isEmpty() ||
                   viewElem.attribute("silent", "false") == "true");

    QString mode = viewElem.attribute("mode", "insert");
    QStringList keyList;
    if (! viewElem.attribute("key").isEmpty())
      keyList = viewElem.attribute("key").split(QRegExp(",\\s*"));

    QString viewName = viewElem.tagName();
    if (viewName.indexOf(".") > 0)
      ; // viewName contains . so accept that it's schema-qualified
    else if (! viewElem.attribute("schema").isEmpty())
      viewName = viewElem.attribute("schema") + "." + viewName;
    else // backwards compatibility - must be in the api schema
      viewName = "api." + viewName;

    // TODO: fix QtXML classes so they read default attribute values from the DTD
    // then remove this code
    if (mode.isEmpty())
      mode = "insert";
    else if (mode == "update" && keyList.isEmpty())
    {
      if (! viewElem.namedItem(viewName + "_number").isNull())
        keyList.append(viewName + "_number");
      else if (! viewElem.namedItem("order_number").isNull())
        keyList.append("order_number");
      else if (! ignoreErr)
      {
        rollback.exec();
        errmsg = tr("Cannot process %1 element without a key attribute");
        return false;
      }
      if (! viewElem.namedItem("line_number").isNull())
        keyList.append("line_number");
    }
    // end of code to remove

    QString savepointName = viewName;
    savepointName.remove(".");
    if (ignoreErr)
      q.exec("SAVEPOINT " + savepointName + ";");

    for (QDomElement columnElem = viewElem.firstChildElement();
         ! columnElem.isNull();
         columnElem = columnElem.nextSiblingElement())
    {
      columnNameList.append(columnElem.tagName());
      if (columnElem.attribute("value") == "[NULL]")
        columnValueList.append("NULL");
      else if (! columnElem.attribute("value").isEmpty())
      {
        QString val = columnElem.attribute("value");
        if (val.contains(apos))
          val.replace(apos, "''");
        columnValueList.append("'" + val + "'");
      }
      else if (columnElem.text().trimmed().startsWith("SELECT"))
        columnValueList.append("(" + columnElem.text() + ")");
      else if (columnElem.text().trimmed() == "[NULL]")
        columnValueList.append("NULL");
      else if (columnElem.attribute("quote") == "false")
        columnValueList.append(columnElem.text());
      else
      {
        QString val = columnElem.text();
        if (val.contains(apos))
          val.replace(apos, "''");
        columnValueList.append("'" + val + "'");
      }
    }

    QString sql;
    if (mode == "update")
    {
      QStringList whereList;
      for (int i = 0; i < keyList.size(); i++)
        whereList.append("(" + keyList[i] + "=" +
                         columnValueList[columnNameList.indexOf(keyList[i])] + ")");

      for (int i = 0; i < columnNameList.size(); i++)
        columnNameList[i].append("=" + columnValueList[i]);

      sql = "UPDATE " + viewName + " SET " +
            columnNameList.join(", ") +
            " WHERE (" + whereList.join(" AND ") + ");";
    }
    else if (mode == "insert")
      sql = "INSERT INTO " + viewName + " (" +
            columnNameList.join(", ") +
            " ) SELECT " +
            columnValueList.join(", ") + ";" ;
    else
    {
      if (ignoreErr)
        q.exec("ROLLBACK TO SAVEPOINT " + savepointName + ";");
      else
      {
        rollback.exec();
        errmsg = tr("Could not process %1: invalid mode %2")
                    .arg(viewElem.tagName(), mode);
        return false;
      }
    }

    if (DEBUG) qDebug("About to run this: %s", qPrintable(sql));
    q.exec(sql);
    if (q.lastError().type() != QSqlError::NoError)
    {
      if (ignoreErr)
      {
        QString warning = q.lastError().databaseText();
        q.exec("ROLLBACK TO SAVEPOINT " + savepointName + ";");
        if (! silent)
        {
          QMessageBox::warning(0, tr("Ignoring Error"),
                               tr("Ignoring database error while importing %1"
                                  ":\n%2")
                              .arg(viewElem.tagName())
                              .arg(warning));
        }                  
      }
      else
      {
        rollback.exec();
        errmsg = tr("Error importing %1 %2: %3")
                    .arg(pFileName, tmpfileName, q.lastError().databaseText());
        return false;
      }
    }
    else if (ignoreErr)
      q.exec("RELEASE SAVEPOINT " + savepointName + ";");
  }

  q.exec("COMMIT;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    errmsg = q.lastError().databaseText();
    return false;
  }

  QFile file(pFileName);
  if (xmlsuccesstreatment == "Delete")
  {
    if (! file.remove())
    {
      errmsg = tr("Could not remove %1 after successful processing (%2).")
                        .arg(pFileName, file.error());
      return false;
    }
  }
  else if (xmlsuccesstreatment == "Rename")
  {
    if (xmlsuccesssuffix.isEmpty())
      xmlsuccesssuffix = ".done";

    QString newname = pFileName + xmlsuccesssuffix;
    for (int i = 0; QFile::exists(newname) ; i++)
      newname = pFileName + xmlsuccesssuffix + "." + QString::number(i);

    if (! file.rename(newname))
    {
      errmsg = tr("Could not rename %1 to %2 after successful processing (%3).")
                        .arg(pFileName, newname).arg(file.error());
      return false;
    }
  }
  else if (xmlsuccesstreatment == "Move")
  {
    if (xmlsuccessdir.isEmpty())
      xmlsuccessdir = "done";
    if (QDir::isRelativePath(xmlsuccessdir))
      xmlsuccessdir = xmldir + QDir::separator() + xmlsuccessdir;

    QDir donedir(xmlsuccessdir);
    if (! donedir.exists())
      donedir.mkpath(xmlsuccessdir);

    QString newname = xmlsuccessdir + QDir::separator() + QFileInfo(file).fileName(); 
    if (QFile::exists(newname))
      newname = newname + QDate::currentDate().toString(".yyyy.MM.dd");
    if (QFile::exists(newname))
      newname = newname + QDateTime::currentDateTime().toString(".hh.mm");
    if (QFile::exists(newname))
      newname = newname + QDateTime::currentDateTime().toString(".ss");

    if (! file.rename(newname))
    {
      errmsg = tr("<p>Could not move %1 to %2 after successful processing (%3).")
                        .arg(pFileName, newname).arg(file.error());
      return false;
    }
  }

  // else if (xmlsuccesstreatment == "None") {}

  return true;
}

bool ImportHelper::openDomDocument(const QString &pFileName, QDomDocument &pDoc, QString &errmsg)
{
  QFile file(pFileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    errmsg = tr("<p>Could not open file %1 (error %2)")
                      .arg(pFileName, file.error());
    return false;
  }

  QString errMsg;
  int errLine;
  int errColumn;
  if (!pDoc.setContent(&file, false, &errMsg, &errLine, &errColumn))
  {
    file.close();
    errmsg = tr("Problem reading %1, line %2 column %3:<br>%4")
                      .arg(pFileName, errLine).arg(errColumn).arg(errMsg);
    return false;
  }

  file.close();

  return true;
}

// scripting exposure //////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(QDomDocument)

static QScriptValue openDomDocument(QScriptContext *context,
                                    QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() < 2)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to openDomDocument");

  QDomDocument domdoc = qscriptvalue_cast<QDomDocument>(context->argument(1));
  QString errmsg;

  bool result = ImportHelper::openDomDocument(context->argument(0).toString(),
                                              domdoc, errmsg);
  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

static QScriptValue importXML(QScriptContext *context,
                              QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() < 1)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to importXML");
  QString errmsg;
  bool result = ImportHelper::importXML(context->argument(0).toString(),
                                        errmsg);

  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

void setupImportHelper(QScriptEngine *engine)
{
  QScriptValue obj = engine->newObject();
  obj.setProperty("importXML", engine->newFunction(importXML),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("openDomDocument", engine->newFunction(openDomDocument),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ImportHelper", obj, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
