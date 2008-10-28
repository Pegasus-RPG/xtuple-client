/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "printChecks.h"

#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QPrintDialog>
#include <QSettings>
#include <QSqlError>
#include <QVariant>

#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>

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
  checks.prepare( "SELECT checkhead_id, checkhead_number, report_name, checkhead_recip_id, checkhead_recip_type "
                  "FROM checkhead "
                  "LEFT OUTER JOIN vendinfo ON((checkhead_recip_id = vend_id) "
                  "                       AND(checkhead_recip_type = 'V')) "
                  "LEFT OUTER JOIN custinfo ON((checkhead_recip_id = cust_id) "
                  "                         AND(checkhead_recip_type = 'C')) "
                  "LEFT OUTER JOIN taxauth ON((checkhead_recip_id = taxauth_id) "
                  "                        AND(checkhead_recip_type = 'T')), "
                  " bankaccnt, form, report "
                  "WHERE ( (checkhead_bankaccnt_id=bankaccnt_id)"
                  " AND (bankaccnt_check_form_id=form_id)"
                  " AND (form_report_id=report_id)"
                  " AND (NOT checkhead_printed) "
                  " AND (NOT checkhead_void) "
                  " AND (bankaccnt_id=:bankaccnt_id) ) "
                  "ORDER BY checkhead_recip_type DESC, COALESCE(vend_number, cust_number, taxauth_code) "
                  "LIMIT :numtoprint; " );
  checks.bindValue(":bankaccnt_id", _bankaccnt->id());
  checks.bindValue(":numtoprint", _numberOfChecks->value());
  checks.exec();
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
  if (checks.lastError().type() != QSqlError::None)
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
	else if (postCheck.lastError().type() != QSqlError::None)
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
  else if (q.lastError().type() != QSqlError::None)
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
      QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
      printCheck::achFileDir = settings.value("ACHOutputDirectory").toString();
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
    if (q.lastError().type() != QSqlError::None)
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
      if (clearq.lastError().type() != QSqlError::None)
      {
        systemError(this, clearq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    sHandleBankAccount(_bankaccnt->id());
  }
  else if (q.lastError().type() != QSqlError::None)
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
