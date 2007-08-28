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

#include "miscAPCheck.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>

/*
 *  Constructs a miscAPCheck as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
miscAPCheck::miscAPCheck(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sPopulateBankInfo(int)));

  _captive = FALSE;

  statusBar()->hide();

  _bankaccnt->setType(XComboBox::APBankAccounts);
}

/*
 *  Destroys the object and frees any allocated resources
 */
miscAPCheck::~miscAPCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void miscAPCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse miscAPCheck::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  param = pParams.value("apchk_id", &valid);
  if (valid)
  {
    _apchkid = param.toInt();
    populate();
  }

  if (pParams.inList("new"))
  {
    _mode = cNew;

    _save->setText(tr("C&reate"));

    _vend->setFocus();
  }
  else if (pParams.inList("edit"))
  {
    _mode = cEdit;

    _bankaccnt->setEnabled(FALSE);

    _save->setFocus();
  }

  return NoError;
}

void miscAPCheck::sSave()
{

  if (!_date->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("You must enter a date for this check.") );
    _date->setFocus();
    return;
  }

  if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("You must enter an amount for this check.") );
    _date->setFocus();
    return;
  }

  if ( (_expense->isChecked()) && (_expcat->id() == -1) )
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("You must select an Expense Category for this expensed check.") );
    _expcat->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT createMiscAPCheck( :bankaccnt_id, :vend_id, :checkDate, "
				         ":amount, :curr_id, "
               "                          :expcat_id, :for, :notes) AS result;" );
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE apchk "
               "SET apchk_vend_id=:vend_id, apchk_checkdate=:checkDate, "
	       "    apchk_amount=:amount, apchk_curr_id=:curr_id, "
               "    apchk_expcat_id=:expcat_id, apchk_for=:for, apchk_notes=:notes "
               "WHERE (apchk_id=:apchk_id);" );
    q.bindValue(":apchk_id", _apchkid);
  }

  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":checkDate", _date->date());
  q.bindValue(":amount", _amount->localValue());
  q.bindValue(":curr_id", _amount->id());
  q.bindValue(":for", _for->text().stripWhiteSpace());
  q.bindValue(":notes", _notes->text().stripWhiteSpace());

  if (_cm->isChecked())
    q.bindValue(":expcat_id", -1);
  else
    q.bindValue(":expcat_id", _expcat->id());

  q.exec();
  if ( (_mode == cNew) && (q.first()) )
  {
    _apchkid = q.value("result").toInt();

    q.prepare( "SELECT apchk_number "
               "FROM apchk "
               "WHERE (apchk_id=:apchk_id);" );
    q.bindValue(":apchk_id", _apchkid);
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("New A/P Check Created"),
                                tr("A new A/P Check has been created and assigned check #%1")
                                .arg(q.value("apchk_number").toString()) );
//  ToDo
  }
//  ToDo

  omfgThis->sAPChecksUpdated(_bankaccnt->id(), _apchkid, TRUE);

  if (_captive)
    close();
}

void miscAPCheck::sPopulateBankInfo(int pBankaccntid)
{
  if ( pBankaccntid != -1 )
  {
    XSqlQuery checkNumber;
    /* this doesn't look right...
       If we're editing then we shouldn't need the next check number because
       we're editing an existing check.  If this is a NEW check then we should
       need both the next check number and the default currency for the bank
       account.
       ToDo: compare this with the base of the B1_2_0_2MC branch and see what
       the original said.
     */
    if (_mode == cEdit)
	checkNumber.prepare( "SELECT bankaccnt_nextchknum "
			     "FROM bankaccnt "
			     "WHERE (bankaccnt_id=:bankaccnt_id);" );
    else
	checkNumber.prepare( "SELECT bankaccnt_curr_id "
			     "FROM bankaccnt "
			     "WHERE (bankaccnt_id=:bankaccnt_id);" );

    checkNumber.bindValue(":bankaccnt_id", pBankaccntid);
    checkNumber.exec();
    if (checkNumber.first())
    {
      if (_mode == cEdit)
	  _checkNum->setText(checkNumber.value("bankaccnt_nextchknum").toString());
      else
	  _amount->setId(checkNumber.value("bankaccnt_curr_id").toInt());
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
  else
    _checkNum->clear();
}

void miscAPCheck::populate()
{
  q.prepare( "SELECT apchk_vend_id, apchk_bankaccnt_id, apchk_number,"
             "       apchk_checkdate, apchk_expcat_id,"
             "       apchk_for, apchk_notes,"
             "       apchk_amount, apchk_curr_id "
             "FROM apchk "
             "WHERE apchk_id=:apchk_id;");
  q.bindValue(":apchk_id", _apchkid);
  q.exec();
  if (q.first())
  {
    _vend->setId(q.value("apchk_vend_id").toInt());
    _bankaccnt->setId(q.value("apchk_bankaccnt_id").toInt());
    _checkNum->setText(q.value("apchk_number").toString());
    _date->setDate(q.value("apchk_checkdate").toDate(), true);
    _amount->set(q.value("apchk_amount").toDouble(),
		 q.value("apchk_curr_id").toInt(),
		 q.value("apchk_checkdate").toDate(), false);
    _for->setText(q.value("apchk_for"));
    _notes->setText(q.value("apchk_notes").toString());

    if (q.value("apchk_expcat_id").toInt() == -1)
      _cm->setChecked(TRUE);
    else
    {
      _expense->setChecked(TRUE);
      _expcat->setId(q.value("apchk_expcat_id").toInt());
    }
  }
}

