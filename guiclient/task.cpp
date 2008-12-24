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

#include "task.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include "userList.h"

task::task(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_actualExp, SIGNAL(lostFocus()), this, SLOT(sExpensesAdjusted()));
  connect(_budgetExp, SIGNAL(lostFocus()), this, SLOT(sExpensesAdjusted()));
  connect(_actualHours, SIGNAL(lostFocus()), this, SLOT(sHoursAdjusted()));
  connect(_budgetHours, SIGNAL(lostFocus()), this, SLOT(sHoursAdjusted()));
  
  _budgetHours->setValidator(omfgThis->qtyVal());
  _actualHours->setValidator(omfgThis->qtyVal());
  _budgetExp->setValidator(omfgThis->qtyVal());
  _actualExp->setValidator(omfgThis->qtyVal());
  _balanceHours->setPrecision(omfgThis->qtyVal());
  _balanceExp->setPrecision(omfgThis->qtyVal());
   
//_usr->addColumn( tr("Username"),_itemColumn, AlignLeft, true, "usr_username");
//_usr->addColumn( tr("Proper Name"),      -1, AlignLeft, true, "usr_propername");

  _prjid = -1;
  _prjtaskid = -1;
  
  adjustSize();
}

task::~task()
{
  // no need to delete child widgets, Qt does it all for us
}

void task::languageChange()
{
  retranslateUi(this);
}

enum SetResponse task::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
    _prjid = param.toInt();

  param = pParams.value("prj_owner_username", &valid);
  if (valid)
    _owner->setUsername(param.toString());

  param = pParams.value("prj_usr_id", &valid);
  if (valid)
    _assignedTo->setId(param.toInt());

  param = pParams.value("prj_start_date", &valid);
  if (valid)
    _started->setDate(param.toDate());

  param = pParams.value("prj_assigned_date", &valid);
  if (valid)
    _assigned->setDate(param.toDate());

  param = pParams.value("prj_due_date", &valid);
  if (valid)
    _due->setDate(param.toDate());

  param = pParams.value("prj_completed_date", &valid);
  if (valid)
    _completed->setDate(param.toDate());

  param = pParams.value("prjtask_id", &valid);
  if (valid)
  {
    _prjtaskid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('prjtask_prjtask_id_seq') AS prjtask_id;");
      if (q.first())
        _prjtaskid = q.value("prjtask_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
      }

      _alarms->setId(_prjtaskid);
    }
    if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(false);
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _status->setEnabled(false);
      _budgetHours->setEnabled(false);
      _actualHours->setEnabled(false);
      _budgetExp->setEnabled(false);
      _actualExp->setEnabled(false);
      _owner->setEnabled(false);
      _assignedTo->setEnabled(false);
      _due->setEnabled(false);
      _assigned->setEnabled(false);
      _started->setEnabled(false);
      _completed->setEnabled(false);
      _alarms->setEnabled(false);
      _save->hide();
    }
  }

  return NoError;
}

void task::populate()
{
  q.prepare( "SELECT prjtask.* "
             "FROM prjtask "
             "WHERE (prjtask_id=:prjtask_id);" );
  q.bindValue(":prjtask_id", _prjtaskid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("prjtask_number"));
    _name->setText(q.value("prjtask_name"));
    _descrip->setText(q.value("prjtask_descrip").toString());
    _owner->setUsername(q.value("prjtask_owner_username").toString());
    _assignedTo->setId(q.value("prjtask_usr_id").toInt());
    _started->setDate(q.value("prjtask_start_date").toDate());
    _assigned->setDate(q.value("prjtask_assigned_date").toDate());
    _due->setDate(q.value("prjtask_due_date").toDate());
    _completed->setDate(q.value("prjtask_completed_date").toDate());

    QString status = q.value("prjtask_status").toString();
    if("P" == status)
      _status->setCurrentIndex(0);
    else if("O" == status)
      _status->setCurrentIndex(1);
    else if("C" == status)
      _status->setCurrentIndex(2);

    _budgetHours->setText(formatQty(q.value("prjtask_hours_budget").toDouble()));
    _actualHours->setText(formatQty(q.value("prjtask_hours_actual").toDouble()));
    _budgetExp->setText(formatCost(q.value("prjtask_exp_budget").toDouble()));
    _actualExp->setText(formatCost(q.value("prjtask_exp_actual").toDouble()));

    _alarms->setId(_prjtaskid);
    sHoursAdjusted();
    sExpensesAdjusted();

    //if (q.value("prjtask_anyuser").toBool())
    //  _anyUser->setChecked(TRUE);
    //else
    //  _userList->setChecked(TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  //sFillUserList();
}

void task::sSave()
{
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Project Task"),
                            tr("You must enter a valid Number.") );
      _number->setFocus();
      return;
  }
  
  if (_name->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Project Task"),
                            tr("You must enter a valid Name.") );
      _name->setFocus();
      return;
  }
  
  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO prjtask "
               "( prjtask_id, prjtask_prj_id, prjtask_number,"
               "  prjtask_name, prjtask_descrip, prjtask_status,"
               "  prjtask_hours_budget, prjtask_hours_actual,"
               "  prjtask_exp_budget, prjtask_exp_actual,"
               "  prjtask_start_date, prjtask_due_date,"
               "  prjtask_assigned_date, prjtask_completed_date,"
               "  prjtask_owner_username, prjtask_usr_id ) "
               "VALUES "
               "( :prjtask_id, :prjtask_prj_id, :prjtask_number,"
               "  :prjtask_name, :prjtask_descrip, :prjtask_status,"
               "  :prjtask_hours_budget, :prjtask_hours_actual,"
               "  :prjtask_exp_budget, :prjtask_exp_actual,"
               "  :prjtask_start_date, :prjtask_due_date,"
               "  :prjtask_assigned_date, :prjtask_completed_date,"
               "  :prjtask_owner_username, :prjtask_usr_id );" );
    q.bindValue(":prjtask_prj_id", _prjid);
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE prjtask "
               "SET prjtask_number=:prjtask_number, prjtask_name=:prjtask_name,"
               "    prjtask_descrip=:prjtask_descrip, prjtask_status=:prjtask_status,"
               "    prjtask_hours_budget=:prjtask_hours_budget,"
               "    prjtask_hours_actual=:prjtask_hours_actual,"
               "    prjtask_exp_budget=:prjtask_exp_budget,"
               "    prjtask_exp_actual=:prjtask_exp_actual,"
               "    prjtask_owner_username=:prjtask_owner_username,"
               "    prjtask_usr_id=:prjtask_usr_id,"
               "    prjtask_start_date=:prjtask_start_date,"
               "    prjtask_due_date=:prjtask_due_date,"
               "    prjtask_assigned_date=:prjtask_assigned_date,"
               "    prjtask_completed_date=:prjtask_completed_date "
               "WHERE (prjtask_id=:prjtask_id);" );

  q.bindValue(":prjtask_id", _prjtaskid);
  q.bindValue(":prjtask_number", _number->text());
  q.bindValue(":prjtask_name", _name->text());
  q.bindValue(":prjtask_descrip", _descrip->toPlainText());
  q.bindValue(":prjtask_owner_username", _owner->username());
  q.bindValue(":prjtask_usr_id",   _assignedTo->id());
  q.bindValue(":prjtask_start_date",	_started->date());
  q.bindValue(":prjtask_due_date",	_due->date());
  q.bindValue(":prjtask_assigned_date",	_assigned->date());
  q.bindValue(":prjtask_completed_date",	_completed->date());
  //q.bindValue(":prjtask_anyuser", QVariant(_anyUser->isChecked()));
  q.bindValue(":prjtask_hours_budget", _budgetHours->text().toDouble());
  q.bindValue(":prjtask_hours_actual", _actualHours->text().toDouble());
  q.bindValue(":prjtask_exp_budget", _budgetExp->text().toDouble());
  q.bindValue(":prjtask_exp_actual", _actualExp->text().toDouble());

  switch(_status->currentIndex())
  {
    case 0:
    default:
      q.bindValue(":prjtask_status", "P");
      break;
    case 1:
      q.bindValue(":prjtask_status", "O");
      break;
    case 2:
      q.bindValue(":prjtask_status", "C");
      break;
  }

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_prjtaskid);
}

void task::sHoursAdjusted()
{
  _balanceHours->setText(formatQty(_budgetHours->text().toDouble() - _actualHours->text().toDouble()));
}

void task::sExpensesAdjusted()
{
  _balanceExp->setText(formatCost(_budgetExp->text().toDouble() - _actualExp->text().toDouble()));
}

void task::sNewUser()
{
/*
  userList newdlg(this, "", TRUE);
  int usrid = newdlg.exec();
  if (usrid != XDialog::Rejected)
  {
    q.prepare( "SELECT prjtaskuser_id "
               "FROM prjtaskuser "
               "WHERE ( (prjtaskuser_usr_id=:usr_id)"
               " AND (prjtaskuser_prjtask_id=:prjtask_id) );" );
    q.bindValue(":usr_id", usrid);
    q.bindValue(":prjtask_id", _prjtaskid);
    q.exec();
    if (!q.first())
    {
      q.prepare( "INSERT INTO prjtaskuser "
                 "( prjtaskuser_prjtask_id, prjtaskuser_usr_id ) "
                 "VALUES "
                 "( :prjtaskuser_prjtask_id, :prjtaskuser_usr_id );" );
      q.bindValue(":prjtaskuser_usr_id", usrid);
      q.bindValue(":prjtaskuser_prjtask_id", _prjtaskid);
      q.exec();
      sFillUserList();
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
*/
}

void task::sDeleteUser()
{
/*
  q.prepare( "DELETE FROM prjtaskuser "
             "WHERE ( (prjtaskuser_usr_id=:usr_id)"
             " AND (prjtaskuser_prjtask_id=:prjtask_id) );" );
  q.bindValue(":usr_id", _usr->id());
  q.bindValue(":prjtask_id", _prjtaskid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillUserList();
*/
}


void task::sFillUserList()
{
/*
  q.prepare( "SELECT usr_id, usr_username, usr_propername "
             "FROM prjtaskuser, usr "
             "WHERE ( (prjtaskuser_usr_id=usr_id)"
             " AND (prjtaskuser_prjtask_id=:prjtask_id) );" );
  q.bindValue(":prjtask_id", _prjtaskid);
  q.exec();
  _usr->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
*/
}

