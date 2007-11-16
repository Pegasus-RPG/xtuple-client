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

#include "returnAuthCheck.h"
#include "storedProcErrorLookup.h"

#include <qvariant.h>
#include <QMessageBox>
#include <QSqlError>

/*
 *  Constructs a returnAuthCheck as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
returnAuthCheck::returnAuthCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_bankaccnt, SIGNAL(newID(int)),    this, SLOT(sPopulateBankInfo(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
returnAuthCheck::~returnAuthCheck()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void returnAuthCheck::languageChange()
{
    retranslateUi(this);
}


void returnAuthCheck::init()
{
  _bankaccnt->setAllowNull(TRUE);
}

enum SetResponse returnAuthCheck::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccntid = param.toInt();
  else
    _bankaccntid = -1;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  param = pParams.value("rahead_id", &valid);
  if (valid)
    _raheadid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  param = pParams.value("memo", &valid);
  if (valid)
    _for->setText(param.toString());

  populate();

  return NoError;
}

void returnAuthCheck::sSave()
{
  if (!_date->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter a date for this check.") );
    _date->setFocus();
    return;
  }

  else if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Cannot Create Miscellaneous Check"),
                          tr("<p>You must enter an amount for this check.") );
    _date->setFocus();
    return;
  }

  else
  {
    q.prepare("SELECT createCheck(:bankaccnt_id, 'C', :recipid,"
	      "                   :checkDate, :amount, :curr_id, NULL,"
		  "                   NULL, :for, :notes, TRUE, :rahead_id) AS result; "
		  "    SELECT apply" );
	q.bindValue(":rahead_id", _raheadid);
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.bindValue(":recipid",	_cust->id());
    q.bindValue(":checkDate", _date->date());
    q.bindValue(":amount",	_amount->localValue());
    q.bindValue(":curr_id",	_amount->id());
    q.bindValue(":for",	_for->text().stripWhiteSpace());
    q.bindValue(":notes", _notes->text().stripWhiteSpace());
  }
  if (q.first())
  {
    _checkid = q.value("result").toInt();
    if (_checkid < 0)
    {
      systemError(this, storedProcErrorLookup("createCheck", _checkid),
		  __FILE__, __LINE__);
      return;
    }
    q.prepare( "SELECT checkhead_number "
               "FROM checkhead "
               "WHERE (checkhead_id=:check_id);" );
    q.bindValue(":check_id", _checkid);
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("New Check Created"),
                                tr("<p>A new Check has been created and "
				   "assigned #%1")
                                .arg(q.value("checkhead_number").toString()) );
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthCheck::sClose()
{
  done(-1);
}

void returnAuthCheck::sPopulateBankInfo(int pBankaccntid)
{
  if ( pBankaccntid != -1 )
  {
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT bankaccnt_nextchknum, bankaccnt_curr_id "
			 "FROM bankaccnt "
			 "WHERE (bankaccnt_id=:bankaccnt_id);" );
    checkNumber.bindValue(":bankaccnt_id", pBankaccntid);
    checkNumber.exec();
    if (checkNumber.first())
    {
      _checkNum->setText(checkNumber.value("bankaccnt_nextchknum").toString());
      _amount->setId(checkNumber.value("bankaccnt_curr_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, checkNumber.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _checkNum->clear();
}


void returnAuthCheck::populate()
{
    _bankaccnt->populate( "SELECT bankaccnt_id, (bankaccnt_name || '-' || bankaccnt_descrip) "
                          "FROM bankaccnt "
                          "ORDER BY bankaccnt_name;" );

  if (_bankaccntid != -1)
    _bankaccnt->setId(_bankaccntid);
}

