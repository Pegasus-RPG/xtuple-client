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

#include "todoList.h"

#include "xdialog.h"
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>
#include <openreports.h>

#include "todoItem.h"
#include "incident.h"
#include "dspCustomerInformation.h"
#include "storedProcErrorLookup.h"
#include "task.h"

todoList::todoList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _dueDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dueDates->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   TRUE);

  _usr->setEnabled(_privileges->check("MaintainOtherTodoLists"));
  _usr->setType(ParameterGroup::User);
  q.prepare("SELECT usr_id, current_user "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _myUsername = q.value("current_user").toInt();
    _usr->setId(q.value("usr_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_completed,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
  connect(_dueDates,	SIGNAL(updated()),	this,   SLOT(sFillList()));
  connect(_incidents,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_projects,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_todoList,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_todoList,	SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)),
	    this,	SLOT(sPopulateMenu(QMenu*)));
  connect(_todoList,	SIGNAL(itemSelectionChanged()),	this,	SLOT(handlePrivs()));
  connect(_usr,		SIGNAL(updated()),	this,	SLOT(sFillList()));
  connect(_usr,		SIGNAL(updated()),	this,	SLOT(handlePrivs()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  _todoList->addColumn(tr("Type"),    _statusColumn,  Qt::AlignCenter, true, "type");
  _todoList->addColumn(tr("Seq"),        _seqColumn,  Qt::AlignRight,  false, "seq");
  _todoList->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  _todoList->addColumn(tr("User"),      _userColumn,  Qt::AlignLeft,   true, "usr");
  _todoList->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  _todoList->addColumn(tr("Description"),        -1,  Qt::AlignLeft,   true, "descrip");
  _todoList->addColumn(tr("Status"),  _statusColumn,  Qt::AlignLeft,   true, "status");
  _todoList->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  _todoList->addColumn(tr("Number"),    _orderColumn,  Qt::AlignLeft,  true, "number");
  _todoList->addColumn(tr("Customer"), _orderColumn,  Qt::AlignLeft,   true, "cust");
  _todoList->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _incidents->setChecked(true);

  handlePrivs();
  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

void todoList::languageChange()
{
    retranslateUi(this);
}

void todoList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem; 

  if (_todoList->currentItem()->altId() == 1)
  {
    bool editPriv =
	(_myUsername == _todoList->currentItem()->text(3) && _privileges->check("MaintainPersonalTodoList")) ||
	(_myUsername != _todoList->currentItem()->text(3) && _privileges->check("MaintainOtherTodoLists"));

    bool viewPriv =
	(_myUsername == _todoList->currentItem()->text(3) && _privileges->check("ViewPersonalTodoList")) ||
	(_myUsername != _todoList->currentItem()->text(3) && _privileges->check("ViewOtherTodoLists"));

    menuItem = pMenu->insertItem(tr("New To-do..."), this, SLOT(sNew()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);

    menuItem = pMenu->insertItem(tr("Edit To-do..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);

    menuItem = pMenu->insertItem(tr("View To-Do..."), this, SLOT(sView()), 0);
    pMenu->setItemEnabled(menuItem, viewPriv);

    menuItem = pMenu->insertItem(tr("Delete To-Do"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);
  }

  if (_todoList->altId() == 1 && !_todoList->currentItem()->text(8).isEmpty())
    pMenu->addSeparator();
  
  if ((_todoList->altId() == 1 && !_todoList->currentItem()->text(8).isEmpty()) ||
       _todoList->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Edit Incident"), this, SLOT(sEditIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainIncidents"));
    menuItem = pMenu->insertItem(tr("View Incident"), this, SLOT(sViewIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") ||
				    _privileges->check("MaintainIncidents"));
  }
  
  if (_todoList->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("Edit Task"), this, SLOT(sEditTask()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainProjects"));
    menuItem = pMenu->insertItem(tr("View Task"), this, SLOT(sViewTask()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewProjects") ||
      _privileges->check("MaintainProjects"));
  }

  if (!_todoList->currentItem()->text(9).isEmpty())
  {
    pMenu->addSeparator();
    menuItem = pMenu->insertItem(tr("Customer Workbench"), this, SLOT(sCustomerInfo()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCustomerMasters"));
  }
}

enum SetResponse todoList::set(const ParameterList& pParams)
{
  QVariant param;
  bool	   valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _usr->setId(param.toInt());
    handlePrivs();
    sFillList();
  }

  return NoError;
}

void todoList::handlePrivs()
{
  //bool editNewPriv  = false;
  bool editTodoPriv = false;
  bool viewTodoPriv = false;

  if (! _todoList->currentItem())
  {
  }
  else if (_todoList->altId() == 1)
  {
    editTodoPriv =
      (_myUsername == _todoList->currentItem()->text(3) && _privileges->check("MaintainPersonalTodoList")) ||
      (_privileges->check("MaintainOtherTodoLists"));

    viewTodoPriv =
      (_myUsername == _todoList->currentItem()->text(3) && _privileges->check("ViewPersonalTodoList")) ||
      (_privileges->check("ViewOtherTodoLists"));
  }
  else if (_todoList->altId() == 2)
  {
    editTodoPriv = _privileges->check("MaintainIncidents");
    viewTodoPriv = (_privileges->check("ViewIncidents") ||
				            _privileges->check("MaintainIncidents"));
  }
  else if (_todoList->altId() == 3)
  {
    editTodoPriv = _privileges->check("MaintainProjects");
    viewTodoPriv = (_privileges->check("ViewProjects") ||
				            _privileges->check("MaintainProjects"));
  }

  _usr->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
		   _privileges->check("ViewOtherTodoLists"));
  _new->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
                   _privileges->check("ViewOtherTodoLists"));
  _edit->setEnabled(editTodoPriv && _todoList->id() > 0);
  _view->setEnabled((editTodoPriv || viewTodoPriv) && _todoList->id() > 0);
  _delete->setEnabled(editTodoPriv && _todoList->id() > 0 && _todoList->altId() != 2);

  if (editTodoPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_view, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else if (viewTodoPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_edit, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
}

void todoList::sClose()
{
  close();
}

void todoList::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_usr->isSelected())
    params.append("usr_username", _myUsername);

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEdit()
{
  if (_todoList->altId() ==2)
    sEditIncident();
  else if (_todoList->altId() == 3)
    sEditTask();
  else
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("todoitem_id", _todoList->id());

    todoItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void todoList::sView()
{
  if (_todoList->altId() ==2)
    sViewIncident();
  else if (_todoList->altId() == 3)
    sViewTask();
  else
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("todoitem_id", _todoList->id());

    todoItem newdlg(this, "", TRUE);
    newdlg.set(params);

    newdlg.exec();
  }
}

void todoList::sDelete()
{
  if (_todoList->altId() == 1)
    q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  else
    q.prepare("DELETE FROM prjtask"
              " WHERE (prjtask_id=:todoitem_id); ");
  q.bindValue(":todoitem_id", _todoList->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();

}

void todoList::setParams(ParameterList &params)
{
  if (_completed->isChecked())
    params.append("completed");
  if (_incidents->isChecked())
    params.append("incidents");
  if (_projects->isChecked())
    params.append("projects");
  _usr->appendValue(params);
  _dueDates->appendValue(params);
  params.append("todo", tr("To-do"));
  params.append("incident", tr("Incident"));
  params.append("task", tr("Task"));
}

void todoList::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("TodoList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void todoList::sFillList()
{
  QString sql = "SELECT todoitem_id AS id, 1 AS altId, todoitem_owner_username AS owner, "
		"       <? value(\"todo\") ?> AS type, incdtpriority_order AS seq, incdtpriority_name AS priority, "
		"       todoitem_name AS name, "
		"       firstLine(todoitem_description) AS descrip, "
		"       todoitem_status AS status, todoitem_due_date AS due, "
		"       usr_username AS usr, CAST(incdt_number AS text) AS number, cust_number AS cust, "
    "       CASE WHEN (todoitem_status != 'C'AND "
    "                  todoitem_due_date < CURRENT_DATE) THEN 'expired'"
    "            WHEN (todoitem_status != 'C'AND "
    "                  todoitem_due_date > CURRENT_DATE) THEN 'future'"
    "       END AS due_qtforegroundrole "
		"FROM usr, todoitem LEFT OUTER JOIN incdt ON (incdt_id=todoitem_incdt_id) "
		"                   LEFT OUTER JOIN crmacct ON (crmacct_id=todoitem_crmacct_id) "
		"                   LEFT OUTER JOIN cust ON (cust_id=crmacct_cust_id) "
    "                   LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
		"WHERE ( (todoitem_usr_id=usr_id)"
		"  AND   (todoitem_due_date BETWEEN <? value(\"startDate\") ?> "
		"                               AND <? value(\"endDate\") ?>) "
		"  <? if not exists(\"completed\") ?>"
		"  AND   (todoitem_status != 'C')"
		"  <? endif ?>"
		"  <? if exists(\"usr_id\") ?> "
		"  AND (usr_id=<? value(\"usr_id\") ?>) "
		"  <? elseif exists(\"usr_pattern\" ?>"
		"  AND (usr_username ~ <? value(\"usr_pattern\") ?>) "
		"  <? endif ?>"
		"  <? if not exists(\"completed\") ?>AND (todoitem_active) <? endif ?>"
		"       ) "
		"<? if exists(\"incidents\")?>"
		"UNION "
		"SELECT incdt_id AS id, 2 AS altId, incdt_owner_username AS owner, "
		"       <? value(\"incident\") ?> AS type, incdtpriority_order AS seq, incdtpriority_name AS priority, "
		"       incdt_summary AS name, "
		"       firstLine(incdt_descrip) AS descrip, "
		"       incdt_status AS status,  incdt_timestamp AS due, "
		"       incdt_assigned_username AS usr, CAST(incdt_number AS text) AS number, cust_number AS cust, "
                "       NULL AS due_qtforegroundrole "
		"FROM incdt LEFT OUTER JOIN usr ON (usr_username=incdt_assigned_username)"
		"           LEFT OUTER JOIN crmacct ON (crmacct_id=incdt_crmacct_id) "
		"           LEFT OUTER JOIN cust ON (cust_id=crmacct_cust_id) "
    "           LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=incdt_incdtpriority_id) "
		"WHERE ((incdt_timestamp BETWEEN <? value(\"startDate\") ?>"
		"                            AND <? value(\"endDate\") ?>)"
		"  <? if not exists(\"completed\") ?> "
		"   AND (incdt_status != 'L')"
		"  <? endif ?>"
		"  <? if exists(\"usr_id\") ?> "
		"  AND (usr_id=<? value(\"usr_id\") ?>) "
		"  <? elseif exists(\"usr_pattern\" ?>"
		"  AND (usr_username ~ <? value(\"usr_pattern\") ?>) "
		"  <? endif ?>"
		"       ) "
		"<? endif ?>"
		"<? if exists(\"projects\")?>"
		"UNION "
		"SELECT prjtask_id AS id, 3 AS altId, prjtask_owner_username AS owner, "
		"       <? value(\"task\") ?> AS type, NULL AS seq, NULL AS priority, "
		"       prjtask_name AS name, "
		"       firstLine(prjtask_descrip) AS descrip, "
		"       prjtask_status AS status,  prjtask_due_date AS due, "
		"       usr_username AS usr, prjtask_number AS number, '' AS cust, "
    "       NULL AS due_qtforegroundrole "
		"FROM prjtask LEFT OUTER JOIN usr ON (usr_id=prjtask_usr_id)"
		"WHERE ((prjtask_due_date BETWEEN <? value(\"startDate\") ?>"
		"                             AND <? value(\"endDate\") ?>)"
		"  <? if not exists(\"completed\") ?> "
		"   AND (prjtask_status != 'L')"
		"  <? endif ?>"
		"  <? if exists(\"usr_username\") ?> "
		"  AND (usr_id=<? value(\"usr_id\") ?>) "
		"  <? elseif exists(\"usr_pattern\" ?>"
		"  AND (usr_username ~ <? value(\"usr_pattern\") ?>) "
		"  <? endif ?>"
		"       ) "
		"<? endif ?>"
		"ORDER BY due, seq, usr;";

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql(sql);
  XSqlQuery itemQ = mql.toQuery(params);

  _todoList->populate(itemQ, true);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  handlePrivs();

  _total->setText(QString::number(_todoList->topLevelItemCount()));
}

void todoList::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

int todoList::getIncidentId()
{
  int returnVal = -1;

  if (_todoList->currentItem()->altId() == 2)
    returnVal = _todoList->id();
  else if (! _todoList->currentItem()->text(8).isEmpty())
  {
    XSqlQuery incdt;
    incdt.prepare("SELECT incdt_id FROM incdt WHERE (incdt_number=:number);");
    incdt.bindValue(":number", _todoList->currentItem()->text(8).toInt());
    if (incdt.exec() && incdt.first())
     returnVal = incdt.value("incdt_id").toInt();
    else if (incdt.lastError().type() != QSqlError::NoError)
      systemError(this, incdt.lastError().databaseText(), __FILE__, __LINE__);
  }

  return returnVal;
}

void todoList::sEditIncident()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", getIncidentId());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sEditTask()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtask_id", _todoList->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewTask()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", _todoList->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void todoList::sCustomerInfo()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", _todoList->currentItem()->text(9));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());

    dspCustomerInformation *newdlg = new dspCustomerInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);

}


