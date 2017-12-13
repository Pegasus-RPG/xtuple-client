/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoList.h"
#include "xdialog.h"
#include "todoItem.h"
#include "incident.h"
#include "customer.h"
#include "project.h"
#include "opportunity.h"
#include "storedProcErrorLookup.h"
#include "task.h"
#include "parameterwidget.h"

#include <QMessageBox>
#include <QSqlError>
#include <QToolBar>
#include "errorReporter.h"

static const int TODO        = 1;
static const int INCIDENT    = 2;
static const int TASK        = 3;
static const int PROJECT     = 4;
static const int OPPORTUNITY = 5;

todoList::todoList(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "todoList", fl)
{
  _shown = false;
  _run = false;

  setupUi(optionsWidget());
  setWindowTitle(tr("To-Do Items"));
  setReportName("TodoList");
  setMetaSQLOptions("todolist", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("User"), "username", ParameterWidget::User, omfgThis->username());
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("Assigned To"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Start Date on or Before"), "startStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Start Date on or After"), "startEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Date on or Before"), "dueStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Date on or After"), "dueEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Show Completed"), "completed", ParameterWidget::Exists);
  parameterWidget()->append(tr("Show Completed Only"), "completedonly", ParameterWidget::Exists);

  connect(_opportunities, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_todolist, SIGNAL(toggled(bool)), this,   SLOT(sFillList()));
  connect(_incidents, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_projects, SIGNAL(toggled(bool)), this,	SLOT(sFillList()));
  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  list()->addColumn(tr("Type"),      _userColumn,  Qt::AlignCenter, true, "type");
  list()->addColumn(tr("Priority"),  _userColumn,  Qt::AlignLeft,   true, "priority");
  list()->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   false,"owner");
  list()->addColumn(tr("Assigned To"),_userColumn, Qt::AlignLeft,   true, "assigned");
  list()->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  list()->addColumn(tr("Notes"),        -1,  Qt::AlignLeft,   true, "notes");
  list()->addColumn(tr("Stage"),   _statusColumn,  Qt::AlignLeft,   true, "stage");
  list()->addColumn(tr("Start Date"),_dateColumn,  Qt::AlignLeft,   false, "start");
  list()->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignLeft,   true, "due");
  list()->addColumn(tr("Account#"), _orderColumn,  Qt::AlignLeft,   false, "crmacct_number");
  list()->addColumn(tr("Account Name"),      100,  Qt::AlignLeft,   true, "crmacct_name");
  list()->addColumn(tr("Parent"),            100,  Qt::AlignLeft,   false, "parent");
  list()->addColumn(tr("Customer"),    _ynColumn,  Qt::AlignLeft,   false, "cust");

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  QToolButton * newBtn = (QToolButton*)toolBar()->widgetForAction(newAction());
  newBtn->setPopupMode(QToolButton::MenuButtonPopup);
  QAction *menuItem;
  QMenu * todoMenu = new QMenu;
  menuItem = todoMenu->addAction(tr("To-Do Item"),   this, SLOT(sNew()));
  if(todoItem::userHasPriv(cNew))
    menuItem->setShortcut(QKeySequence::New);
  menuItem->setEnabled(todoItem::userHasPriv(cNew));
  menuItem = todoMenu->addAction(tr("Opportunity"), this, SLOT(sNewOpportunity()));
  menuItem->setEnabled(opportunity::userHasPriv(cNew));
  menuItem = todoMenu->addAction(tr("Incident"), this, SLOT(sNewIncdt()));
  menuItem->setEnabled(incident::userHasPriv(cNew));
  menuItem = todoMenu->addAction(tr("Project"), this, SLOT(sNewProject()));
  menuItem->setEnabled(project::userHasPriv(cNew));
  newBtn->setMenu(todoMenu);
}

void todoList::showEvent(QShowEvent * event)
{
  display::showEvent(event);

  if(!_shown)
  {
    _shown = true;
    if(_run)
      sFillList();
  }
}

void todoList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  bool edit = false;
  bool view = false;
  bool del = false;
  bool foundDeletable = false;
  bool foundParent = false;
  bool editParent = false;
  bool viewParent = false;
  bool foundCustomer = false;

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    edit = edit || getPriv(cEdit, item->altId(), item);
    view = view || getPriv(cView, item->altId(), item);

    if (item->altId() == TODO || item->altId() == TASK || item->altId() == PROJECT)
    {
      foundDeletable = true;
      del = del || getPriv(cEdit, item->altId(), item);
    }

    if (getParentType(item))
    {
      foundParent = true;

      editParent = editParent || getPriv(cEdit, getParentType(item), item);
      viewParent = viewParent || getPriv(cView, getParentType(item), item);
    }

    if (item->rawValue("cust").toInt() > 0)
      foundCustomer = true;
  }

  QAction *menuItem;

  if (list()->selectedItems().size() > 0)
  {
    menuItem = pMenu->addAction(tr("Edit"), this, SLOT(sEdit()));
    menuItem->setEnabled(edit);

    menuItem = pMenu->addAction(tr("View"), this, SLOT(sView()));
    menuItem->setEnabled(view);
  }

  if (foundDeletable)
  {
    menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
    menuItem->setEnabled(del);
  }

  if (foundParent)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Edit Parent"), this, SLOT(sEditParent()));
    menuItem->setEnabled(editParent);

    menuItem = pMenu->addAction(tr("View Parent"), this, SLOT(sViewParent()));
    menuItem->setEnabled(viewParent);
  }

  if (foundCustomer)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Edit Customer"), this, SLOT(sEditCustomer()));
    menuItem->setEnabled(getPriv(cEdit));

    menuItem = pMenu->addAction(tr("View Customer"), this, SLOT(sViewCustomer()));
    menuItem->setEnabled(getPriv(cView));
  }
}

enum SetResponse todoList::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;

  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void todoList::sNew()
{
  //Need an extra priv check because of display trigger
  if (!todoItem::userHasPriv(cNew))
    return;

  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");

  todoItem newdlg(0, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sNewIncdt()
{
  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");

  incident newdlg(0, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEdit()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    bool edit = getPriv(cEdit, item->altId(), item);
    bool view = getPriv(cView, item->altId(), item);

    if (!edit && !view)
      continue;

    if (item->altId() == TODO)
      if (edit)
        sEditTodo(item->id());
      else
        sViewTodo(item->id());
    if (item->altId() == INCIDENT)
      if (edit)
        sEditIncident(item->id());
      else
        sViewIncident(item->id());
    if (item->altId() == TASK)
      if (edit)
        sEditTask(item->id());
      else
        sViewTask(item->id());
    if (item->altId() == PROJECT)
      if (edit)
        sEditProject(item->id());
      else
        sViewProject(item->id());
    if (item->altId() == OPPORTUNITY)
      if (edit)
        sEditOpportunity(item->id());
      else
        sViewOpportunity(item->id());
  }
}

void todoList::sView()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if(!getPriv(cView, item->altId(), item))
      continue;

    if (item->altId() == TODO)
      sViewTodo(item->id());
    if (item->altId() == INCIDENT)
      sViewIncident(item->id());
    if (item->altId() == TASK)
      sViewTask(item->id());
    if (item->altId() == PROJECT)
      sViewProject(item->id());
    if (item->altId() == OPPORTUNITY)
      sViewOpportunity(item->id());
  }
}

void todoList::sEditParent()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    bool edit = getPriv(cEdit, getParentType(item), item);
    bool view = getPriv(cView, getParentType(item), item);

    if (!edit && !view)
      continue;

    if (getParentType(item) == INCIDENT)
      if (edit)
        sEditIncident(item->id("parent"));
      else
        sViewIncident(item->id("parent"));
    if (getParentType(item) == PROJECT)
      if (edit)
        sEditProject(item->id("parent"));
      else
        sViewProject(item->id("parent"));
    if (getParentType(item) == OPPORTUNITY)
      if (edit)
        sEditOpportunity(item->id("parent"));
      else
        sViewOpportunity(item->id("parent"));
  }
}

void todoList::sViewParent()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if(!getPriv(cView, getParentType(item), item))
      continue;

    if (getParentType(item) == INCIDENT)
      sViewIncident(item->id("parent"));
    if (getParentType(item) == PROJECT)
      sViewProject(item->id("parent"));
    if (getParentType(item) == OPPORTUNITY)
      sViewOpportunity(item->id("parent"));
  }
}

void todoList::sEditTodo(int id)
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", id);

  todoItem* newdlg = new todoItem(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sViewTodo(int id)
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", id);

  todoItem* newdlg = new todoItem(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sDelete()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if(!getPriv(cEdit, item->altId(), item))
      continue;

    XSqlQuery todoDelete;
    QString recurstr;
    QString recurtype;
    if (item->altId() == TODO)
    {
      recurstr = "SELECT MAX(todoitem_due_date) AS max"
                 "  FROM todoitem"
                 " WHERE todoitem_recurring_todoitem_id=:id"
                 "   AND todoitem_id!=:id;" ;
      recurtype = "TODO";
    }

    bool deleteAll  = false;
    bool deleteOne  = false;
    if (! recurstr.isEmpty())
    {
      XSqlQuery recurq;
      recurq.prepare(recurstr);
      recurq.bindValue(":id", item->id());
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
        // user said delete one but the only one that exists is the parent ToDo
        else if (ret == QMessageBox::Yes)
          deleteOne = true;
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving To Do Item Information"),
                                    recurq, __FILE__, __LINE__))
      {
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

    int procresult = 0;
    if (deleteAll)  // Delete all todos in the recurring series
    {
      todoDelete.prepare("SELECT deleteOpenRecurringItems(:id, :type, NULL, true)"
                "       AS result;");
      todoDelete.bindValue(":id",   item->id());
      todoDelete.bindValue(":type", recurtype);
      todoDelete.exec();
      if (todoDelete.first())
        procresult = todoDelete.value("result").toInt();

      if (procresult < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Recurring To Do Item Information"),
                             storedProcErrorLookup("deleteOpenRecurringItems", procresult),
                             __FILE__, __LINE__);
        return;
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Recurring To Do Item Information"),
                                    todoDelete, __FILE__, __LINE__))
      {
        return;
      }
    }

    if (deleteOne) // The base todo in a recurring series has been seleted.  Have to move
                   // recurrence to the next item else we hit foreign key errors.
                   // Make the next item on the list the parent in the series
    {
      todoDelete.prepare("UPDATE todoitem SET todoitem_recurring_todoitem_id =("
                          "               SELECT MIN(todoitem_id) FROM todoitem"
                          "                 WHERE todoitem_recurring_todoitem_id=:id"
                          "                   AND todoitem_id!=:id)"
                          "  WHERE todoitem_recurring_todoitem_id=:id"
                          "  AND todoitem_id!=:id;");
      todoDelete.bindValue(":id",   item->id());
      todoDelete.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Recurring To Do Item Information"),
                               todoDelete, __FILE__, __LINE__))
      {
        return;
      }
    }

    if (item->altId() == TODO)
      todoDelete.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
    else if (item->altId() == TASK)
      todoDelete.prepare("DELETE FROM prjtask"
                " WHERE (prjtask_id=:todoitem_id); ");
    else if (item->altId() == PROJECT)
      todoDelete.prepare("SELECT deleteProject(:todoitem_id) AS result");
    else
      return;
    todoDelete.bindValue(":todoitem_id", item->id());
    todoDelete.exec();
    if (todoDelete.first())
    {
      int result = todoDelete.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving To Do Item Information"),
                             storedProcErrorLookup("deleteTodoItem", result),
                             __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving To Do Item Information"),
                                  todoDelete, __FILE__, __LINE__))
    {
       return;
    }
  }

  sFillList();
}

bool todoList::setParams(ParameterList &params)
{
  if (!_todolist->isChecked() &&
      !_opportunities->isChecked() &&
      !_incidents->isChecked() &&
      !_projects->isChecked())
  {
    list()->clear();
    return false;
  }

  if (_todolist->isChecked())
    params.append("todoList");
  if (_opportunities->isChecked())
    params.append("opportunities");
  if (_incidents->isChecked())
    params.append("incidents");
  if (_projects->isChecked())
    params.append("projects");

  params.append("todo", tr("To-do"));
  params.append("incident", tr("Incident"));
  params.append("task", tr("Task"));
  params.append("project", tr("Project"));
  params.append("opportunity", tr("Opportunity"));
  params.append("complete", tr("Completed"));
  params.append("deferred", tr("Deferred"));
  params.append("pending", tr("Pending"));
  params.append("inprocess", tr("InProcess"));
  params.append("feedback", tr("Feedback"));
  params.append("confirmed", tr("Confirmed"));
  params.append("assigned", tr("Assigned"));
  params.append("resolved", tr("Resolved"));
  params.append("closed", tr("Closed"));
  params.append("concept", tr("Concept"));
  params.append("new", tr("New"));

  if (!display::setParams(params))
    return false;

  return true;
}

bool todoList::getPriv(const int pMode, const int pType, XTreeWidgetItem* item)
{
  if (!item)
    item = list()->currentItem();

  int id;
  if (item->altId() == pType)
    id = item->id();
  else
    id = item->id("parent");

  if (pType == TODO)
    return todoItem::userHasPriv(pMode, id);
  else if (pType == INCIDENT)
    return incident::userHasPriv(pMode, id);
  else if (pType == TASK)
    return task::userHasPriv(pMode, id);
  else if (pType == PROJECT)
    return project::userHasPriv(pMode, id);
  else if (pType == OPPORTUNITY)
    return opportunity::userHasPriv(pMode, id);
  else
    return customer::userHasPriv(pMode);
}

int todoList::getParentType(XTreeWidgetItem* item)
{
  if (item->altId() == TODO && item->rawValue("parent") == "INCDT")
    return INCIDENT;
  if (item->altId() == TODO && item->rawValue("parent") == "OPP")
    return OPPORTUNITY;
  if (item->altId() == TASK)
    return PROJECT;

  return 0;
}

void todoList::sEditIncident(int id)
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", id);

  incident* newdlg = new incident(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sViewIncident(int id)
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", id);

  incident* newdlg = new incident(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sNewProject()
{
  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");

  project newdlg(0, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEditProject(int id)
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prj_id", id);

  project* newdlg = new project(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sViewProject(int id)
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prj_id", id);

  project* newdlg = new project(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sEditTask(int id)
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtask_id", id);

  task* newdlg = new task(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sViewTask(int id)
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", id);

  task* newdlg = new task(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sEditCustomer()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (item->rawValue("cust").toInt() > 0)
    {
      ParameterList params;
      params.append("cust_id", item->rawValue("cust").toInt());
      params.append("mode","edit");

      customer *newdlg = new customer(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void todoList::sViewCustomer()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (item->rawValue("cust").toInt() > 0)
    {
      ParameterList params;
      params.append("cust_id", item->rawValue("cust").toInt());
      params.append("mode","view");

      customer *newdlg = new customer(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void todoList::sNewOpportunity()
{
  ParameterList params;
  parameterWidget()->appendValue(params);
  params.append("mode", "new");

  opportunity newdlg(0, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void todoList::sEditOpportunity(int id)
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ophead_id", id);

  opportunity* newdlg = new opportunity(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sViewOpportunity(int id)
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ophead_id", id);

  opportunity* newdlg = new opportunity(0, "", false);
  newdlg->set(params);
  newdlg->setAttribute(Qt::WA_DeleteOnClose);
  newdlg->show();
}

void todoList::sOpen()
{
  bool editPriv = false;
  bool viewPriv = false;

    switch (list()->altId())
    {
    case 1:
      editPriv = todoItem::userHasPriv(cEdit, list()->currentItem()->id());
      viewPriv = todoItem::userHasPriv(cView, list()->currentItem()->id());
      break;
    case 2:
      editPriv = incident::userHasPriv(cEdit, list()->currentItem()->id());
      viewPriv = incident::userHasPriv(cView, list()->currentItem()->id());
      break;
    case 3:
      editPriv = task::userHasPriv(cEdit, list()->currentItem()->id());
      viewPriv = task::userHasPriv(cView, list()->currentItem()->id());
      break;
    case 4:
      editPriv = project::userHasPriv(cEdit, list()->currentItem()->id());
      viewPriv = project::userHasPriv(cView, list()->currentItem()->id());
      break;
    case 5:
      editPriv = opportunity::userHasPriv(cEdit, list()->currentItem()->id());
      viewPriv = opportunity::userHasPriv(cView, list()->currentItem()->id());
      break;
    default:
      break;
    }

  if(editPriv)
    sEdit();
  else if(viewPriv)
    sView();
  else
    QMessageBox::information(this, tr("Restricted Access"), tr("You have not been granted privileges to open this item."));
}

void todoList::sFillList()
{
  if(_shown)
    display::sFillList();
  else
    _run = true;
}



