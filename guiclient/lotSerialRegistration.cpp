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

#include "lotSerialRegistration.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QMenu>

#include "storedProcErrorLookup.h"
#include "todoItem.h"

/*
 *  Constructs a lotSerialRegistration as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
lotSerialRegistration::lotSerialRegistration(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_cancel,	SIGNAL(clicked()),	this,	SLOT(sCancel()));
  connect(_save,	SIGNAL(clicked()),	this,	SLOT(sSave()));

  _lotserial->setStrict(true);
  _contact->setAccountVisible(FALSE);
  _contact->setActiveVisible(FALSE);
}

/*
 *  Destroys the object and frees any allocated resources
 */
lotSerialRegistration::~lotSerialRegistration()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void lotSerialRegistration::languageChange()
{
    retranslateUi(this);
}

enum SetResponse lotSerialRegistration::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("lsreg_id", &valid);
  if (valid)
  {
    _lsregid = param.toInt();
    populate();
    _lotserial->setItemId(_item->id());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.exec("SELECT fetchLotSerialRegNumber() AS result;");
      if(q.first())
        _number->setText(q.value("result").toString());
        q.prepare(" INSERT INTO lsreg (lsreg_number,lsreg_ VALUES "
                  " (
      else
      {
        QMessageBox::critical( omfgThis, tr("Database Error"),
                               tr( "A Database Error occured in lotSerialRegistration::New.\n"
                                   "Contact your Systems Administrator." ));
        reject();
      }
      _comments->setReadOnly(true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _regDate->setEnabled(false);
      _soldDate->setEnabled(false);
      _expireDate->setEnabled(false);
      _crmacct->setEnabled(false);
      _cntct->setEnabled(false);
      _type->setEnabled(false);
      _item->setReadOnly(true);
      _lotserial->setEnabled(false);
      _newChar->setEnabled(false);
      _editChar->setEnabled(false);
      _deleteChar->setEnabled(false);
      _notes->setEnabled(false);

      _save->hide();
      _cancel->setText(tr("&Close"));
      _cancel->setFocus();
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  sHandleTodoPrivs();
  return NoError;
}

int lotSerialRegistration::saveContact(ContactCluster* pContact)
{
  pContact->setAccount(_crmacct->id());

  int answer = 2;	// Cancel
  int saveResult = pContact->save(AddressCluster::CHECK);

  if (-1 == saveResult)
    systemError(this, tr("There was an error saving a Contact (%1, %2).\n"
			 "Check the database server log for errors.")
		      .arg(pContact->label()).arg(saveResult),
		__FILE__, __LINE__);
  else if (-2 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple Contacts sharing this address (%1).\n"
		       "What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving %1").arg(pContact->label()),
		    tr("Would you like to update the existing Contact or create a new one?"),
		    tr("Create New"),
		    tr("Change Existing"),
		    tr("Cancel"),
		    2, 2);
  if (0 == answer)
    return pContact->save(AddressCluster::CHANGEONE);
  else if (1 == answer)
    return pContact->save(AddressCluster::CHANGEALL);

  return saveResult;
}

void lotSerialRegistration::sCancel()
{
  if (_saved && cNew == _mode)
  {
    q.prepare("DELETE FROM lsreg WHERE (lsreg_id=:lsreg_id");
    q.bindValue(":lsreg_id", _lsregid);
    q.exec();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  reject();
}

void lotSerialRegistration::sSave()
{
  if (! save(false)) // if error
    return;

  accept();
}

