/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoList.h"
#include "xdialog.h"
#include "mqlutil.h"
#include "todoItem.h"
#include "incident.h"
#include "customer.h"
#include "project.h"
#include "storedProcErrorLookup.h"
#include "task.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>


todoList::todoList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

	QSqlDatabase db = QSqlDatabase::database();

  _parameterWidget->append(tr("Assigned"), "assigned_username", ParameterWidget::User, db.userName());
  _parameterWidget->append(tr("Assigned Pattern"), "assigned_usr_pattern", ParameterWidget::Text);
  _parameterWidget->append(tr("Owner"), "owner_username", ParameterWidget::User);
  _parameterWidget->append(tr("Owner Pattern"), "owner_usr_pattern", ParameterWidget::Text);
  _parameterWidget->append(tr("CRM Account"), "crmAccountId", ParameterWidget::Crmacct);
  _parameterWidget->append(tr("Start Date Before"), "startStartDate", ParameterWidget::Date);
  _parameterWidget->append(tr("Start Date After"), "startEndDate", ParameterWidget::Date);
  _parameterWidget->append(tr("Due Date Before"), "dueStartDate", ParameterWidget::Date);
  _parameterWidget->append(tr("Due Date After"), "dueEndDate", ParameterWidget::Date);
  _parameterWidget->applyDefaultFilterSet();

  _crmAccount->hide();
  
  q.exec("SELECT current_user;");

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_parameterWidget, SIGNAL(updated()), this, SLOT(sFillList()));

	connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_completed,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));

  connect(_todolist,    SIGNAL(toggled(bool)),  this,   SLOT(sFillList()));
  connect(_incidents,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_projects,	SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_todoList,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_todoList,	SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)),
	    this,	SLOT(sPopulateMenu(QMenu*)));
  connect(_todoList,	SIGNAL(itemSelectionChanged()),	this,	SLOT(handlePrivs()));

  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));
  

  _todoList->addColumn(tr("Type"),      _userColumn,  Qt::AlignCenter, true, "type");
  _todoList->addColumn(tr("Seq"),        _seqColumn,  Qt::AlignRight,  false, "seq");
  _todoList->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  _todoList->addColumn(tr("Assigned To"),_userColumn,  Qt::AlignLeft,   true, "usr");
  _todoList->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  _todoList->addColumn(tr("Description"),        -1,  Qt::AlignLeft,   true, "descrip");
  _todoList->addColumn(tr("Status"),  _statusColumn,  Qt::AlignLeft,   true, "status");
  _todoList->addColumn(tr("Start Date"),_dateColumn,  Qt::AlignLeft,   false, "start");
  _todoList->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  _todoList->addColumn(tr("Parent#"),  _orderColumn,  Qt::AlignLeft,   true, "number");
  _todoList->addColumn(tr("Customer#"),_orderColumn,  Qt::AlignLeft,   false, "cust");
  _todoList->addColumn(tr("Account#"), _orderColumn,  Qt::AlignLeft,   false, "crmacct_number");
  _todoList->addColumn(tr("Account Name"),      100,  Qt::AlignLeft,   true, "crmacct_name");
  _todoList->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");

  int menuItem;
  QMenu * todoMenu = new QMenu;
  menuItem = todoMenu->insertItem(tr("Incident"), this, SLOT(sNewIncdt()));
  if (!_privileges->check("MaintainIncidents"))
    todoMenu->setItemEnabled(menuItem, FALSE);
  menuItem = todoMenu->insertItem(tr("To-Do Item"),   this, SLOT(sNew()));
  if (!_privileges->check("MaintainPersonalTodoList") &&
      !_privileges->check("MaintainOtherTodoLists"))
    todoMenu->setItemEnabled(menuItem, FALSE);
  _new->setMenu(todoMenu);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _incidents->setChecked(true);

  handlePrivs();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

void todoList::languageChange()
{
    retranslateUi(this);
}

void todoList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  bool editPriv =
      (omfgThis->username() == _todoList->currentItem()->text(3) && _privileges->check("MaintainPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text(3) && _privileges->check("MaintainOtherTodoLists"));

  bool viewPriv =
      (omfgThis->username() == _todoList->currentItem()->text(3) && _privileges->check("ViewPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text(3) && _privileges->check("ViewOtherTodoLists"));

  menuItem = pMenu->insertItem(tr("New To-Do..."), this, SLOT(sNew()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  if (_todoList->currentItem()->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit To-Do..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);

    menuItem = pMenu->insertItem(tr("View To-Do..."), this, SLOT(sView()), 0);
    pMenu->setItemEnabled(menuItem, viewPriv);

    menuItem = pMenu->insertItem(tr("Delete To-Do"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, editPriv);
  }

  pMenu->addSeparator();

  menuItem = pMenu->insertItem(tr("New Incident..."), this, SLOT(sNewIncdt()), 0);
  pMenu->setItemEnabled(menuItem,  _privileges->check("MaintainIncidents"));

  if ((_todoList->altId() == 1 && !_todoList->currentItem()->text(9).isEmpty()) ||
       _todoList->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Edit Incident"), this, SLOT(sEditIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainIncidents"));
    menuItem = pMenu->insertItem(tr("View Incident"), this, SLOT(sViewIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") ||
				    _privileges->check("MaintainIncidents"));
  }
  pMenu->addSeparator();

  if (_todoList->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("Edit Task"), this, SLOT(sEditTask()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainProjects"));
    menuItem = pMenu->insertItem(tr("View Task"), this, SLOT(sViewTask()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewProjects") ||
      _privileges->check("MaintainProjects"));
    pMenu->addSeparator();
  }

  if (_todoList->altId() >= 3)
  {
    menuItem = pMenu->insertItem(tr("Edit Project"), this, SLOT(sEditProject()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainProjects"));
    menuItem = pMenu->insertItem(tr("View Project"), this, SLOT(sViewProject()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewProjects") ||
      _privileges->check("MaintainProjects"));
  }

  if (!_todoList->currentItem()->text(10).isEmpty())
  {
    pMenu->addSeparator();
    menuItem = pMenu->insertItem(tr("Edit Customer"), this, SLOT(sEditCustomer()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCustomerMasters") || _privileges->check("ViewCustomerMasters"));
    menuItem = pMenu->insertItem(tr("View Customer"), this, SLOT(sViewCustomer()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewCustomerMasters"));
  }
}

enum SetResponse todoList::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    //_usr->setId(param.toInt());
    handlePrivs();
    sFillList();
  }

  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

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
      (omfgThis->username() == _todoList->currentItem()->text(3) && _privileges->check("MaintainPersonalTodoList")) ||
      (_privileges->check("MaintainOtherTodoLists"));

    viewTodoPriv =
      (omfgThis->username() == _todoList->currentItem()->text(3) && _privileges->check("ViewPersonalTodoList")) ||
      (_privileges->check("ViewOtherTodoLists"));
  }
  else if (_todoList->altId() == 2)
  {
    editTodoPriv = _privileges->check("MaintainIncidents");
    viewTodoPriv = (_privileges->check("ViewIncidents") ||
				            _privileges->check("MaintainIncidents"));
  }
  else if (_todoList->altId() >= 3)
  {
    editTodoPriv = _privileges->check("MaintainProjects");
    viewTodoPriv = (_privileges->check("ViewProjects") ||
				            _privileges->check("MaintainProjects"));
  }

  //_usrGroup->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
		   //original - _privileges->check("ViewOtherTodoLists"));
		   _privileges->check("ViewOtherTodoLists");
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
  if (_crmAccount->isValid())
    params.append("crmacct_id", _crmAccount->id());
  params.append("mode", "new");
  //if (_selected->isChecked())
  //  params.append("username", _usr->username());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sNewIncdt()
{
  ParameterList params;
  if (_crmAccount->isValid())
    params.append("crmacct_id", _crmAccount->id());
  params.append("mode", "new");

  incident newdlg(this, "", true);
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
  else if (_todoList->altId() == 4)
    sEditProject();
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
  else if (_todoList->altId() == 4)
    sViewProject();
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
  QString recurstr;
  QString recurtype;
  if (_todoList->altId() == 1)
  {
    recurstr = "SELECT MAX(todoitem_due_date) AS max"
               "  FROM todoitem"
               " WHERE todoitem_recurring_todoitem_id=:id"
               "   AND todoitem_id!=:id;" ;
    recurtype = "TODO";
  }
  /* TODO: can't delete incidents from here. why not?
  else if (_todoList->altId() == 2)
  {
    recurstr = "SELECT MAX(incdt_timestamp) AS max"
               "   FROM incdt"
               " WHERE incdt_recurring_incdt_id=:id;" ;
    recurtype = "INCDT";
  }
   */

  bool deleteAll  = false;
  bool createMore = false;
  if (! recurstr.isEmpty())
  {
    XSqlQuery recurq;
    recurq.prepare(recurstr);
    recurq.bindValue(":id", _todoList->id());
    recurq.exec();
    if (recurq.first() && !recurq.value("max").isNull())
    {
      QMessageBox askdelete(QMessageBox::Question, tr("Delete Recurring Item?"),
                            tr("<p>This is a recurring item. Do you want to "
                               "delete just this one item or delete all open "
                               "items in this recurrence?"),
                            QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Cancel,
                            this);
      askdelete.setDefaultButton(QMessageBox::Cancel);
      int ret = askdelete.exec();
      if (ret == QMessageBox::Cancel)
        return;
      else if (ret == QMessageBox::YesToAll)
        deleteAll = true;
      // user said delete one but the only one that exists is the base
      else if (ret == QMessageBox::Yes && recurq.value("max").isNull())
        createMore = true;
    }
    else if (recurq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, recurq.lastError().text(), __FILE__, __LINE__);
      return;
    }
    else if (QMessageBox::warning(this, tr("Delete List Item?"),
                                  tr("<p>Are you sure that you want to "
                                     "completely delete the selected item?"),
	  		    QMessageBox::Yes | QMessageBox::No,
			    QMessageBox::No) == QMessageBox::No)
      return;
  }
  else if (QMessageBox::warning(this, tr("Delete List Item?"),
                                tr("<p>Are you sure that you want to "
                                   "completely delete the selected item?"),
	  		    QMessageBox::Yes | QMessageBox::No,
			    QMessageBox::No) == QMessageBox::No)
    return;

  QString procname;
  int procresult = 0;
  if (deleteAll)
  {
    procname = "deleteOpenRecurringItems";
    q.prepare("SELECT deleteOpenRecurringItems(:id, :type, NULL, TRUE)"
              "       AS result;");
    q.bindValue(":id",   _todoList->id());
    q.bindValue(":type", recurtype);
    q.exec();
    if (q.first())
      procresult = q.value("result").toInt();
  }
  if (procresult >= 0 && createMore)
  {
    procname = "createRecurringItems";
    q.prepare("SELECT createRecurringItems(:id, :type) AS result;");
    q.bindValue(":id",   _todoList->id());
    q.bindValue(":type", recurtype);
    q.exec();
    if (q.first())
      procresult = q.value("result").toInt();
  }

  // not elseif - error handling for 1 or 2 queries
  if (procresult < 0)
  {
    systemError(this, storedProcErrorLookup(procname, procresult));
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_todoList->altId() == 1)
    q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  else if (_todoList->altId() == 3)
    q.prepare("DELETE FROM prjtask"
              " WHERE (prjtask_id=:todoitem_id); ");
  else if (_todoList->altId() == 4)
    q.prepare("SELECT deleteProject(:todoitem_id) AS result");
  else
    return;
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
  if (_crmAccount->isValid())
    params.append("crmAccountId",_crmAccount->id());
  if (_todolist->isChecked())
    params.append("todoList");
  if (_completed->isChecked())
    params.append("completed");
  if (_incidents->isChecked())
    params.append("incidents");
  if (_projects->isChecked())
    params.append("projects");

  params.append("todo", tr("To-do"));
  params.append("incident", tr("Incident"));
  params.append("task", tr("Task"));
  params.append("project", tr("Project"));
  params.append("completed", tr("Completed"));
  params.append("deferred", tr("Deferred"));
  params.append("pending", tr("Pending"));
  params.append("inprocess", tr("InProcess"));
  params.append("new", tr("New"));
  _parameterWidget->appendValue(params);
}

void todoList::sPrint()
{
  ParameterList params;
  setParams(params);

  if (_todoList->topLevelItemCount() == 0)
  {
    QMessageBox::information( this, tr("Cannot Print To-do List"),
                              tr("The list is empty, the is nothing to print.")  );
    return;
  }

  orReport report("TodoList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void todoList::sFillList()
{
  if (!_todolist->isChecked() && !_incidents->isChecked() && !_projects->isChecked())
  {
    _todoList->clear();
    return;
  }

  MetaSQLQuery mql = mqlLoad("todolist", "detail");
  ParameterList params;
  setParams(params);

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
  else if (! _todoList->currentItem()->text(9).isEmpty())
  {
    XSqlQuery incdt;
    incdt.prepare("SELECT incdt_id FROM incdt WHERE (incdt_number=:number);");
    incdt.bindValue(":number", _todoList->currentItem()->text(9).toInt());
    if (incdt.exec() && incdt.first())
     returnVal = incdt.value("incdt_id").toInt();
    else if (incdt.lastError().type() != QSqlError::NoError)
      systemError(this, incdt.lastError().databaseText(), __FILE__, __LINE__);
  }

  return returnVal;
}

int todoList::getProjectId()
{
  int returnVal = -1;

  if (_todoList->currentItem()->altId() == 4)
    returnVal = _todoList->id();
  else if (_todoList->currentItem()->altId() == 3)
  {
    XSqlQuery prj;
    prj.prepare("SELECT prjtask_prj_id FROM prjtask WHERE (prjtask_id=:prjtask_id);");
    prj.bindValue(":prjtask_id", _todoList->id());
    if (prj.exec() && prj.first())
     returnVal = prj.value("prjtask_prj_id").toInt();
    else if (prj.lastError().type() != QSqlError::NoError)
     systemError(this, prj.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (! _todoList->currentItem()->text(9).isEmpty())
  {
    XSqlQuery prj;
    prj.prepare("SELECT prj_id FROM prj WHERE (prj_number=:number);");
    prj.bindValue(":number", _todoList->currentItem()->text(9));
    if (prj.exec() && prj.first())
     returnVal = prj.value("prj_id").toInt();
    else if (prj.lastError().type() != QSqlError::NoError)
     systemError(this, prj.lastError().databaseText(), __FILE__, __LINE__);
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

void todoList::sEditProject()
{

  ParameterList params;
  params.append("mode", "edit");
  params.append("prj_id", getProjectId());

  project newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sViewProject()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prj_id", getProjectId());

  project newdlg(this, "", TRUE);
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

void todoList::sEditCustomer()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", _todoList->currentItem()->text(10));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());
    params.append("mode","edit");

    customer *newdlg = new customer(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
}

void todoList::sViewCustomer()
{
  XSqlQuery cust;
  cust.prepare("SELECT cust_id FROM cust WHERE (cust_number=:number);");
  cust.bindValue(":number", _todoList->currentItem()->text(10));
  if (cust.exec() && cust.first())
  {
    ParameterList params;
    params.append("cust_id", cust.value("cust_id").toInt());
    params.append("mode","view");

    customer *newdlg = new customer(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (cust.lastError().type() != QSqlError::NoError)
    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
}


