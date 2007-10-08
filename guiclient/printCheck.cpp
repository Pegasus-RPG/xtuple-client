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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "printCheck.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>

#include "storedProcErrorLookup.h"

printCheck::printCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
  connect(_print,     SIGNAL(clicked()),  this, SLOT(sPrint()));

  _captive = FALSE;

  _check->setAllowNull(TRUE);

  _bankaccnt->setType(XComboBox::APBankAccounts);
}

printCheck::~printCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void printCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printCheck::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("check_id", &valid);
  if (valid)
  {
    populate(param.toInt());
    _bankaccnt->setEnabled(FALSE);
    _check->setEnabled(FALSE);
  }

  return NoError;
}

void printCheck::sPrint()
{
  q.prepare( "SELECT checkhead_printed, report_name, bankaccnt_id "
             "FROM checkhead, bankaccnt, form, report "
             "WHERE ((checkhead_bankaccnt_id=bankaccnt_id)"
             "  AND  (bankaccnt_check_form_id=form_id)"
             "  AND  (form_report_id=report_id)"
             "  AND  (checkhead_id=:checkhead_id) );" );
  q.bindValue(":checkhead_id", _check->id());
  q.exec();
  if (q.first())
  {
    if(q.value("checkhead_printed").toBool())
    {
      QMessageBox::information( this, tr("Check Already Printed"),
		    tr("<p>The selected Check has already been printed.") );
      return;
    }

    ParameterList params;

    params.append("checkhead_id", _check->id());
    params.append("apchk_id", _check->id());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();

    omfgThis->sChecksUpdated(q.value("bankaccnt_id").toInt(), _check->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else // join failed
  {
    QMessageBox::critical(this, tr("Cannot Print Check"),
                          tr("<p>The selected Check cannot be printed as the "
			     "Bank Account that it is to draw upon does not "
			     "have a valid Check Format assigned to it. "
			     "Please assign a valid Check Format to this Bank "
			     "Account before attempting to print this Check."));
    return;
  }

  if ( QMessageBox::question( this, tr("Check Printed"),
                             tr("Was the selected Check printed successfully?"),
			     QMessageBox::Yes,
			     QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "SELECT checkhead_bankaccnt_id,"
	       "       markCheckAsPrinted(checkhead_id) AS result "
               "FROM checkhead "
               "WHERE (checkhead_id=:checkhead_id);" );
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("markCheckAsPrinted", result),
		    __FILE__, __LINE__);
	return;
      }
      omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(),
			      _check->id(), TRUE);

      if (_captive)
        accept();
      else
      {
        sHandleBankAccount(_bankaccnt->id());
        _close->setText(tr("&Close"));
      }
    }
  }
  else if ( QMessageBox::question(this, tr("Mark Check as Voided"),
                                  tr("<p>Would you like to mark the selected "
				     "Check as Void and create a replacement "
				     "check?"),
				   QMessageBox::Yes,
				   QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT voidCheck(:checkhead_id) AS result;");
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("voidCheck", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT checkhead_bankaccnt_id,"
	       "       replaceVoidedCheck(checkhead_id) AS result "
               "FROM checkhead "
               "WHERE (checkhead_id=:checkhead_id);" );
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("replaceVoidedCheck", result),
		    __FILE__, __LINE__);
	return;
      }
      omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(),
			       _check->id(), TRUE);

      sHandleBankAccount(_bankaccnt->id());
      _print->setFocus();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void printCheck::sHandleBankAccount(int pBankaccntid)
{
  q.prepare( "SELECT checkhead_id,"
	     "       (TEXT(checkhead_number) || '-' || checkrecip_name) "
             "FROM checkhead LEFT OUTER JOIN "
	     "      checkrecip ON ((checkhead_recip_id=checkrecip_id)"
	     "                AND  (checkhead_recip_type=checkrecip_type))"
             "WHERE ((NOT checkhead_void)"
             "  AND  (NOT checkhead_printed)"
             "  AND  (NOT checkhead_posted)"
             "  AND  (checkhead_bankaccnt_id=:bankaccnt_id) ) "
             "ORDER BY checkhead_number;" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  _check->populate(q);
  _check->setNull();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printCheck::populate(int pcheckid)
{
  q.prepare( "SELECT checkhead_bankaccnt_id "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  q.bindValue(":checkhead_id", pcheckid);
  q.exec();
  if (q.first())
  {
    _bankaccnt->setId(q.value("checkhead_bankaccnt_id").toInt());
    _check->setId(pcheckid);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

