/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printChecks.h"

#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QPrintDialog>
#include <QSqlError>
#include <QVariant>

#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>
#include "mqlutil.h"

#include "xtsettings.h"
#include "confirmAchOK.h"
#include "printCheck.h"
#include "printChecksReview.h"
#include "storedProcErrorLookup.h"

printChecks::printChecks(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
  connect(_createACH, SIGNAL(clicked()),  this, SLOT(sCreateACH()));
  connect(_print,     SIGNAL(clicked()),  this, SLOT(sPrint()));

  _setCheckNumber = -1;

  _bankaccnt->setAllowNull(TRUE);
  _bankaccnt->setType(XComboBox::APBankAccounts);

  _createACH->setVisible(_metrics->boolean("ACHEnabled"));
}

printChecks::~printChecks()
{
  // no need to delete child widgets, Qt does it all for us
}

void printChecks::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printChecks::set(const ParameterList & pParams )
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  return NoError;
}

void printChecks::sPrint()
{
  if (_somerecips_ach_enabled &&
      QMessageBox::question(this, tr("Print Anyway?"),
                            tr("<p>Some of the recipients of checks in this "
                               "check run have been configured for ACH "
                               "transactions. Do you want to print checks "
                               "for them anyway?<p>If you answer 'Yes' then "
                               "a check will be printed. If you say 'No' then "
                               "you should click Create ACH File first and "
                               "<i>then</i> click Print."),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::No)
    return;

  QList<int> printedChecks;
  bool firstRun = TRUE;

  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());

  bool setup = true;

  QList<ORODocument*> singleCheckPrerendered;
  XSqlQuery checks;
  MetaSQLQuery mql = mqlLoad("checks", "detail");

  params.append("toPrintOnly");
  params.append("numtoprint", _numberOfChecks->value());
  if (_orderByName->isChecked())
    params.append("orderByName");
  
  checks = mql.toQuery(params);
  QDomDocument docReport;

  while (checks.next())
  {
    if (setup)
    {
      // get the report definition out of the database
      // this should somehow be condensed into a common code call or something
      // in the near future to facilitate easier conversion in other places
      // of the application to use the new rendering engine directly
      XSqlQuery report;
      report.prepare( "SELECT report_source "
                      "  FROM report "
                      " WHERE (report_name=:report_name) "
                      "ORDER BY report_grade DESC LIMIT 1;" );
      report.bindValue(":report_name", checks.value("report_name").toString());
      report.exec();
      if (report.first())
      {
        QString errorMessage;
        int     errorLine;

        if (!docReport.setContent(report.value("report_source").toString(), &errorMessage, &errorLine))
        {
          systemError(this, errorMessage, __FILE__, __LINE__);
          return;
        }
      }
      else
      {
        systemError(this, report.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      // end getting the report definition out the database

      if(_setCheckNumber != -1 && _setCheckNumber != _nextCheckNum->text().toInt() && firstRun)
      {
        q.prepare("SELECT setNextCheckNumber(:bankaccnt_id, :nextCheckNumber) AS result;");
        q.bindValue(":bankaccnt_id", _bankaccnt->id());
        q.bindValue(":nextCheckNumber", _nextCheckNum->text().toInt());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("setNextCheckNumber", result),
                        __FILE__, __LINE__);
            return;
          }
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
       firstRun = FALSE;
      }
    }

    q.prepare("UPDATE checkhead SET checkhead_number=fetchNextCheckNumber(checkhead_bankaccnt_id)"
              " WHERE(checkhead_id=:checkhead_id);");
    q.bindValue(":checkhead_id", checks.value("checkhead_id").toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    ParameterList params;
    params.append("checkhead_id", checks.value("checkhead_id").toInt());
    params.append("apchk_id", checks.value("checkhead_id").toInt());

    ORPreRender pre;
    pre.setDom(docReport);
    pre.setParamList(params);
    ORODocument * doc = pre.generate();

    printedChecks.append(checks.value("checkhead_id").toInt());

    int page_num = 1;
    while(page_num < doc->pages())
    {
      page_num++;

      XSqlQuery qq;
      qq.prepare("INSERT INTO checkhead"
                 "      (checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, checkhead_printed,"
                 "       checkhead_checkdate, checkhead_number,"
                 "       checkhead_amount, checkhead_void,"
                 "       checkhead_misc,"
                 "       checkhead_for, checkhead_notes,"
                 "       checkhead_curr_id, checkhead_deleted) "
                 "SELECT checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, TRUE,"
                 "       checkhead_checkdate, fetchNextCheckNumber(checkhead_bankaccnt_id),"
                 "       checkhead_amount, TRUE, TRUE,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       checkhead_curr_id, TRUE"
                 "  FROM checkhead"
                 " WHERE(checkhead_id=:checkhead_id);");
      qq.bindValue(":checkhead_id", checks.value("checkhead_id").toInt());
      if(!qq.exec())
      {
        systemError(this, "Received error but will continue anyway:\n"+qq.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
    singleCheckPrerendered.append(doc);
  }
  if (checks.lastError().type() != QSqlError::NoError)
  {
    systemError(this, checks.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(!printedChecks.empty())
  {
    QPrinter printer(QPrinter::HighResolution);
    ORODocument entireCheckRunPrerendered;
    for (int j = 0; j < singleCheckPrerendered.size(); j++)
    {
      for (int i = 0; i < singleCheckPrerendered.at(j)->pages(); i++)
      {
        entireCheckRunPrerendered.addPage(singleCheckPrerendered.at(j)->page(i));
      }
    }

    ORPrintRender render;
    render.setupPrinter(&entireCheckRunPrerendered, &printer);

    QPrintDialog pd(&printer);
    pd.setMinMax(1, entireCheckRunPrerendered.pages());
    if(pd.exec() != XDialog::Accepted)
      return;
    render.setPrinter(&printer);
    render.render(&entireCheckRunPrerendered);

    QList<int>::iterator it;

    if ( QMessageBox::question(this, tr("All Checks Printed"),
			       tr("<p>Did all the Checks print successfully?"),
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      XSqlQuery postCheck;
      postCheck.prepare( "SELECT markCheckAsPrinted(:checkhead_id) AS result;");
      for( it = printedChecks.begin(); it != printedChecks.end(); ++it)
      {
        postCheck.bindValue(":checkhead_id", (*it));
        postCheck.exec();
	if (postCheck.first())
	{
	  int result = postCheck.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("markCheckAsPrinted",
						   result), __FILE__, __LINE__);
	    return;
	  }
	}
	else if (postCheck.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, postCheck.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
    else
    {
      printChecksReview newdlg(this, "", TRUE);
      QString query = QString( "SELECT checkhead_id, checkhead_number"
                               "  FROM checkhead"
                               " WHERE (checkhead_id IN (" );
      bool first = true;
      for( it = printedChecks.begin(); it != printedChecks.end(); ++it)
      {
        if(!first)
          query += ",";
        query += QString::number((*it)); 
        first = false;
      }
      query += ") ); ";
      newdlg._checks->populate(query);
      newdlg.sSelectAll();
      newdlg.sMarkPrinted();
      newdlg._checks->clearSelection();
      newdlg.exec();
    }
  }
  else
    QMessageBox::information( this, tr("No Checks Printed"),
			     tr("<p>No Checks were printed for the selected "
				"Bank Account.") );

  omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);
  sHandleBankAccount(_bankaccnt->id());
}

void printChecks::sHandleBankAccount(int pBankaccntid)
{
  q.prepare( "SELECT bankaccnt_nextchknum,"
             "       BOOL_OR(bankaccnt_ach_enabled) AS bank_ach_enabled,"
             "       COUNT(*) AS numofchecks,"
             "       BOOL_OR(COALESCE(vend_ach_enabled, false)) AS somerecip_ach_enabled,"
             "       BOOL_AND(COALESCE(vend_ach_enabled, false)) AS allrecip_ach_enabled "
             "  FROM bankaccnt, checkhead"
             "       LEFT OUTER JOIN vendinfo ON (checkhead_recip_type='V'"
             "                                AND checkhead_recip_id=vend_id) "
             " WHERE((NOT checkhead_void)"
             "   AND (NOT checkhead_printed)"
             "   AND (checkhead_bankaccnt_id=bankaccnt_id)"
             "   AND (checkhead_bankaccnt_id=:bankaccnt_id))"
             " GROUP BY bankaccnt_nextchknum;" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  if (q.first())
  {
    _setCheckNumber = q.value("bankaccnt_nextchknum").toInt();
    _nextCheckNum->setText(q.value("bankaccnt_nextchknum").toString());
    _numberOfChecks->setMaxValue(q.value("numofchecks").toInt());
    _numberOfChecks->setValue(q.value("numofchecks").toInt());
    _allrecips_ach_enabled = q.value("allrecip_ach_enabled").toBool();
    _somerecips_ach_enabled = q.value("somerecip_ach_enabled").toBool();
    _print->setEnabled(q.value("numofchecks").toInt() > 0);
    _createACH->setEnabled(q.value("bank_ach_enabled").toBool() &&
                           q.value("somerecip_ach_enabled").toBool() &&
                           q.value("numofchecks").toInt() > 0);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else // not found
  {
    _setCheckNumber = -1;
    _nextCheckNum->clear();
    _numberOfChecks->clear();
    _allrecips_ach_enabled = false;
    _somerecips_ach_enabled = false;
    _print->setEnabled(false);
    _createACH->setEnabled(false);
  }
}

void printChecks::sCreateACH()
{
  if (_somerecips_ach_enabled && !_allrecips_ach_enabled &&
      QMessageBox::question(this, tr("Print Anyway?"),
                            tr("<p>Some but not all of the checks in this run "
                               "are for Vendors configured to receive ACH "
                               "transactions. Do you want to create the ACH "
                               "file anyway?<p>If you answer 'Yes' then an "
                               "ACH file will be created but you will have to "
                               "click Print to get the remainder of the "
                               "checks in this run. If you say 'No' then you "
                               "will get a warning when you click Print "
                               "asking whether you want to print checks for "
                               "ACH recipients."),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::No)
    return;

  XSqlQuery releasenum;
  releasenum.prepare("SELECT releaseNumber('ACHBatch', :batch);");

  QString batch;

  q.prepare("SELECT * FROM formatACHChecks(:bankaccnt_id, NULL, :key);");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.bindValue(":key",          omfgThis->_key);
  q.exec();
  if (q.first())
  {
    batch = q.value("achline_batch").toString();
    releasenum.bindValue(":batch", batch);
    if (printCheck::achFileDir.isEmpty())
    {
      printCheck::achFileDir = xtsettingsValue("ACHOutputDirectory").toString();
    }
    QString suffixes = "*.ach *.dat *.txt";
    if (! suffixes.contains(_metrics->value("ACHDefaultSuffix")))
      suffixes = "*" + _metrics->value("ACHDefaultSuffix") + " " + suffixes;
    QString filename = QFileDialog::getSaveFileName(this, tr("ACH Output File"),
                            printCheck::achFileDir + QDir::separator() +
                            "ach" + batch + _metrics->value("ACHDefaultSuffix"),
                            "(" + suffixes + ")");
    if (filename.isEmpty())
    {
      releasenum.exec();
      return;
    }
    QFileInfo fileinfo(filename);
    printCheck::achFileDir = fileinfo.absolutePath();
    QFile achfile(filename);
    if (! achfile.open(QIODevice::WriteOnly))
    {
      releasenum.exec();
      QMessageBox::critical(this, tr("Could Not Open File"),
                            tr("Could not open %1 for writing ACH data.")
                            .arg(filename));
      return;
    }
    do
    {
      achfile.write(q.value("achline_value").toString().toAscii());
      achfile.write("\n");
    } while (q.next());
    achfile.close();
    if (q.lastError().type() != QSqlError::NoError)
    {
      releasenum.exec();
      achfile.remove();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (confirmAchOK::askOK(this, achfile))
      markChecksAsPrinted(batch);
    else
    {
      releasenum.exec();
      XSqlQuery clearq;
      clearq.prepare("UPDATE checkhead "
                     "SET checkhead_printed=false,"
                     "    checkhead_ach_batch=NULL "
                     "WHERE (checkhead_ach_batch=:checkhead_ach_batch);");
      clearq.bindValue(":checkhead_ach_batch", batch);
      clearq.exec();
      if (clearq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, clearq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    sHandleBankAccount(_bankaccnt->id());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printChecks::markChecksAsPrinted(const QString pbatch)
{
  XSqlQuery markq;
  markq.prepare("SELECT MIN(checkhead_bankaccnt_id) AS bankaccnt_id,"
                "       MIN(markCheckAsPrinted(checkhead_id)) AS result "
                "FROM checkhead "
                "WHERE (checkhead_ach_batch=:checkhead_ach_batch);" );
  markq.bindValue(":checkhead_ach_batch", pbatch);
  markq.exec();
  if (markq.first())
  {
    int result = markq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("markCheckAsPrinted", result),
                  __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(markq.value("bankaccnt_id").toInt(), -1, TRUE);
  }
}
