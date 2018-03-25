/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projects.h"
#include "projectCopy.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QVariant>
#include <QSqlError>

#include <parameter.h>

#include "parameterwidget.h"
#include "project.h"
#include "task.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "invoice.h"
#include "invoiceItem.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "purchaseOrderItem.h"
#include "incident.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

#define DEBUG true

static const int PROJECT           = 1;
static const int TASK              = 5;
static const int QUOTE             = 15;
static const int QUOTEITEM         = 17;
static const int SALESORDER        = 25;
static const int SALESORDERITEM    = 27;
static const int INVOICE           = 35;
static const int INVOICEITEM       = 37;
static const int WORKORDER         = 45;
static const int PURCHASEREQUEST   = 55;
static const int PURCHASEORDER     = 65;
static const int PURCHASEORDERITEM = 67;
static const int INCIDENT          = 105;

projects::projects(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "projects", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Projects"));
  setMetaSQLOptions("projects", "detail_nohierarchy");
  setReportName("ProjectsList");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);
  setUseAltId(true);

  //  Project Details
  list()->addColumn(tr("Project"),                        _orderColumn, Qt::AlignLeft,   true,  "prj_number");
  list()->addColumn(tr("Project Name"),                   -1,           Qt::AlignLeft,   true,  "prj_name");
  list()->addColumn(tr("Project Description"),            -1,           Qt::AlignLeft,   true,  "prj_descrip");
  list()->addColumn(tr("Status"),                         _itemColumn,  Qt::AlignCenter, true,  "project_status" );
  list()->addColumn(tr("Project Type"),                   _itemColumn,  Qt::AlignCenter, true,  "project_type" );
  list()->addColumn(tr("Owner"),                          _userColumn,  Qt::AlignLeft,   true,  "prj_owner_username");
  list()->addColumn(tr("Assigned To"),                    _userColumn,  Qt::AlignLeft,   true,  "prj_username");
  list()->addColumn(tr("Account"),                        _userColumn,  Qt::AlignLeft,   true,  "crmacct_name");
  list()->addColumn(tr("Contact"),                        _userColumn,  Qt::AlignLeft,   true,  "cntct_name");
  list()->addColumn(tr("City"),                           -1,           Qt::AlignLeft,   true,  "addr_city");
  list()->addColumn(tr("State"),                          -1,           Qt::AlignLeft,   true,  "addr_state");
  list()->addColumn(tr("Project Due"),                    _dateColumn,  Qt::AlignCenter, true,  "prj_due_date");
  list()->addColumn(tr("Project Assigned"),               _dateColumn,  Qt::AlignCenter, true,  "prj_assigned_date");
  list()->addColumn(tr("Project Started"),                _dateColumn,  Qt::AlignCenter, true,  "prj_start_date");
  list()->addColumn(tr("Project Completed"),              _dateColumn,  Qt::AlignCenter, true,  "prj_completed_date");

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllProjects") && !_privileges->check("MaintainPersonalProjects"))
    newAction()->setEnabled(false);

  _salesOrders->setChecked(false);
  _workOrders->setChecked(false);
  _purchaseOrders->setChecked(false);
  _incidents->setChecked(false);
  _showHierarchy->setChecked(false);

  connect(omfgThis, SIGNAL(projectsUpdated(int)), this, SLOT(sFillList()));
  connect(_showComplete, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _ordersGroup->hide();

  QString qryType = QString( "SELECT prjtype_id, prjtype_descr FROM prjtype " );

  QString qryStatus = QString( "SELECT  1, '%1' UNION "
                               "SELECT  2, '%2' UNION "
                               "SELECT  3, '%3'")
  .arg(tr("Complete"))
  .arg(tr("Concept"))
  .arg(tr("In-Process"));
  
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("AssignedTo"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Contact"), "cntct_id", ParameterWidget::Contact);
  parameterWidget()->appendComboBox(tr("Project Type"), "prjtype_id", qryType);
  parameterWidget()->appendComboBox(tr("Project Status"), "prjstatus_id", qryStatus);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);
  parameterWidget()->append(tr("Project Task"), "project_task", ParameterWidget::Text);
  parameterWidget()->append(tr("Sales Order"), "cohead_id", ParameterWidget::SalesOrder);
  parameterWidget()->append(tr("Work Order"), "wo_id", ParameterWidget::WorkOrder);
  parameterWidget()->append(tr("Purchase Order"), "pohead_id", ParameterWidget::PurchaseOrder);
  parameterWidget()->append(tr("Start Start Date"), "startStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Start End Date"), "startEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due Start Date"), "dueStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Due End Date"), "dueEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Assigned Start Date"), "assignedStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Assigned End Date"), "assignedEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Completed Start Date"), "completedStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Completed End Date"), "completedEndDate", ParameterWidget::Date);

  setupCharacteristics("PROJ");
          
  _statuses << "None" << "C" << "P" << "O";

}

void projects::sBuildList()
{

  list()->setColumnCount(0);

  if (!_showHierarchy->isChecked())
  {
//  List Hierarchy Display
    setMetaSQLOptions("projects", "detail");
    list()->addColumn(tr("Number"),       _orderColumn,  Qt::AlignLeft,   true,  "name");
    list()->addColumn(tr("Name"),                   -1,  Qt::AlignLeft,   true,  "item");
    list()->addColumn(tr("Description"),            -1,  Qt::AlignLeft,   true,  "descrip");
    list()->addColumn(tr("Status"),        _itemColumn,  Qt::AlignCenter, true,  "status" );
    list()->addColumn(tr("Project Type"),        _itemColumn,  Qt::AlignCenter, true,  "project_type" );
    list()->addColumn(tr("Owner"),         _userColumn,  Qt::AlignLeft,   false, "prj_owner_username");
    list()->addColumn(tr("Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prj_username");
    list()->addColumn(tr("Account/Customer"),   _userColumn,  Qt::AlignLeft,   true,  "customer");
    list()->addColumn(tr("Contact"),       _userColumn,  Qt::AlignLeft,   false,  "contact");
    list()->addColumn(tr("City"),       -1,  Qt::AlignLeft,   false,  "city");
    list()->addColumn(tr("State"),       -1,  Qt::AlignLeft,   false,  "state");
    list()->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
    list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true,  "uom"  );
    list()->addColumn(tr("Value"),      _qtyColumn,   Qt::AlignRight,  true,  "value"  );
    list()->addColumn(tr("Due"),           _dateColumn,  Qt::AlignCenter, true,  "due");
    list()->addColumn(tr("Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "assigned");
    list()->addColumn(tr("Started"),       _dateColumn,  Qt::AlignCenter, true,  "started");
    list()->addColumn(tr("Completed"),     _dateColumn,  Qt::AlignCenter, true,  "completed");
    list()->addColumn(tr("Budget Hrs."),   _costColumn,  Qt::AlignRight,  true,  "hrs_budget");
    list()->addColumn(tr("Actual Hrs."),   _costColumn,  Qt::AlignRight,  true,  "hrs_actual");
    list()->addColumn(tr("Balance Hrs."),  _costColumn,  Qt::AlignRight,  true,  "hrs_balance");
    list()->addColumn(tr("Budget Exp."),   _priceColumn,  Qt::AlignRight,  false,  "exp_budget");
    list()->addColumn(tr("Actual Exp."),   _priceColumn,  Qt::AlignRight,  false,  "exp_actual");
    list()->addColumn(tr("Balance Exp."),  _priceColumn,  Qt::AlignRight,  false,  "exp_balance");
  } else {
//  List flat display (for CSV export purposes)
    setMetaSQLOptions("projects", "detail_nohierarchy");
//  Project Details
    list()->addColumn(tr("Project"),       _orderColumn,  Qt::AlignLeft,   true,  "prj_number");
    list()->addColumn(tr("Project Name"),                   -1,  Qt::AlignLeft,   true,  "prj_name");
    list()->addColumn(tr("Project Description"),            -1,  Qt::AlignLeft,   true,  "prj_descrip");
    list()->addColumn(tr("Status"),        _itemColumn,  Qt::AlignCenter, true,  "project_status" );
    list()->addColumn(tr("Project Type"),        _itemColumn,  Qt::AlignCenter, true,  "project_type" );
    list()->addColumn(tr("Owner"),         _userColumn,  Qt::AlignLeft,   true, "prj_owner_username");
    list()->addColumn(tr("Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prj_username");
    list()->addColumn(tr("Account"),   _userColumn,  Qt::AlignLeft,   true,  "crmacct_name");
    list()->addColumn(tr("Contact"),       _userColumn,  Qt::AlignLeft,   true,  "cntct_name");
    list()->addColumn(tr("City"),       -1,  Qt::AlignLeft,   true,  "addr_city");
    list()->addColumn(tr("State"),       -1,  Qt::AlignLeft,   true,  "addr_state");
    list()->addColumn(tr("Project Due"),           _dateColumn,  Qt::AlignCenter, true,  "prj_due_date");
    list()->addColumn(tr("Project Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "prj_assigned_date");
    list()->addColumn(tr("Project Started"),       _dateColumn,  Qt::AlignCenter, true,  "prj_start_date");
    list()->addColumn(tr("Project Completed"),     _dateColumn,  Qt::AlignCenter, true,  "prj_completed_date");
//  Task Details
    list()->addColumn(tr("Task"),       _orderColumn,  Qt::AlignLeft,   true,  "prjtask_number");
    list()->addColumn(tr("Task Name"),                   -1,  Qt::AlignLeft,   true,  "prjtask_name");
    list()->addColumn(tr("Task Description"),            -1,  Qt::AlignLeft,   true,  "prjtask_descrip");
    list()->addColumn(tr("Task Status"),        _itemColumn,  Qt::AlignCenter, true,  "task_status" );
    list()->addColumn(tr("Task Owner"),         _userColumn,  Qt::AlignLeft,   true, "prjtask_owner_username");
    list()->addColumn(tr("Task Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prjtask_username");
    list()->addColumn(tr("Customer"),   _userColumn,  Qt::AlignLeft,   true,  "cust_name");
    list()->addColumn(tr("Task Due"),           _dateColumn,  Qt::AlignCenter, true,  "prjtask_due_date");
    list()->addColumn(tr("Task Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "prjtask_assigned_date");
    list()->addColumn(tr("Task Started"),       _dateColumn,  Qt::AlignCenter, true,  "prjtask_start_date");
    list()->addColumn(tr("Task Completed"),     _dateColumn,  Qt::AlignCenter, true,  "prjtask_completed_date");
    list()->addColumn(tr("Budget Hrs."),   _costColumn,  Qt::AlignRight,  true,  "prjtask_hours_budget");
    list()->addColumn(tr("Actual Hrs."),   _costColumn,  Qt::AlignRight,  true,  "prjtask_hours_actual");
    list()->addColumn(tr("Balance Hrs."),  _costColumn,  Qt::AlignRight,  true,  "prjtask_hours_balance");
    list()->addColumn(tr("Budget Exp."),   _priceColumn,  Qt::AlignRight,  true,  "prjtask_exp_budget");
    list()->addColumn(tr("Actual Exp."),   _priceColumn,  Qt::AlignRight,  true,  "prjtask_exp_actual");
    list()->addColumn(tr("Balance Exp."),  _priceColumn,  Qt::AlignRight,  true,  "prjtask_exp_balance");
    setupCharacteristics("PROJ");
    _showHierarchy->setEnabled(false);
  }

  sFillList();
}

void projects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(0, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  bool edit = false;
  bool view = false;
  bool del = false;
  bool foundEditable = false;
  bool foundDeletable = false;

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if(item->altId() != PURCHASEREQUEST)
    {
      foundEditable = true;
      edit = edit || getPriv(cEdit, item);
    }

    view = view || getPriv(cView, item);

    if(item->altId() == PROJECT)
    {
      foundDeletable = true;
      del = del || getPriv(cEdit, item);
    }
  }

  if (foundEditable)
  {
    menuItem = pMenu->addAction(tr("Edit"), this, SLOT(sEdit()));
    menuItem->setEnabled(edit);
  }

  menuItem = pMenu->addAction(tr("View"), this, SLOT(sView()));
  menuItem->setEnabled(view);
 
  if (foundDeletable)
  {
    menuItem = pMenu->addAction("Delete", this, SLOT(sDelete()));
    menuItem->setEnabled(del);

    menuItem = pMenu->addAction("Copy", this, SLOT(sCopy()));
    menuItem->setEnabled(del);
  }
}

void projects::sEdit()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    bool edit = getPriv(cEdit, item);
    bool view = getPriv(cView, item);

    if (!edit && !view)
      continue;

    if (edit)
      open(item, "edit");
    else
      open(item, "view");
  }
}

void projects::sView()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (!getPriv(cView, item))
      return;

    open(item, "view");
  }
}

void projects::open(XTreeWidgetItem* item, QString mode)
{
  ParameterList params;

  if(item->altId() == PROJECT)
  {
    params.append("prj_id", item->id());
    params.append("mode", mode);
   
    project* newdlg = new project(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == TASK)
  {
    params.append("mode", mode);
    params.append("prjtask_id", item->id());

    task* newdlg = new task(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == QUOTE)
  {
    params.append("mode", mode + "Quote");
    params.append("quhead_id", item->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->altId() == QUOTEITEM)
  {
    params.append("mode", mode + "Quote");
    params.append("soitem_id", item->id());

    salesOrderItem* newdlg = new salesOrderItem();
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == SALESORDER)
  {
    params.append("mode", mode);
    params.append("sohead_id", item->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->altId() == SALESORDERITEM)
  {
    params.append("mode", mode);
    params.append("soitem_id", item->id());

    salesOrderItem* newdlg = new salesOrderItem();
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == INVOICE)
  {
    if (mode=="edit")
      invoice::editInvoice(item->id(), this);
    else
      invoice::viewInvoice(item->id(), this);
  }
  else if(item->altId() == INVOICEITEM)
  {
    params.append("mode", mode);
    params.append("invcitem_id", item->id());

    invoiceItem* newdlg = new invoiceItem(this);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == WORKORDER)
  {
    params.append("mode", mode);
    params.append("wo_id", item->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == PURCHASEREQUEST)
  {
    params.append("mode", "view");
    params.append("pr_id", item->id());

    purchaseRequest* newdlg = new purchaseRequest(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == PURCHASEORDER)
  {
    params.append("mode", mode);
    params.append("pohead_id", item->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->altId() == PURCHASEORDERITEM)
  {
    params.append("mode", mode);
    params.append("poitem_id", item->id());

    purchaseOrderItem* newdlg = new purchaseOrderItem(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
  else if(item->altId() == INCIDENT)
  {
    params.append("mode", mode);
    params.append("incdt_id", item->id());

    incident* newdlg = new incident(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
}

void projects::sDelete()
{
  XSqlQuery projectsDelete;
  projectsDelete.prepare("SELECT deleteProject(:prj_id) AS result");

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (item->altId() != 1 || !getPriv(cEdit, item))
      continue;

    projectsDelete.bindValue(":prj_id", item->id());
    projectsDelete.exec();
    if(projectsDelete.first())
    {
      int result = projectsDelete.value("result").toInt();
      if(result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Project"),
                             storedProcErrorLookup("deleteProject", result),
                             __FILE__, __LINE__);
        return;
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Project"),
                                    projectsDelete, __FILE__, __LINE__))
      {
        return;
      }
    }
  }

  sFillList();
}

void projects::sCopy()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (item->altId() != 1 || !getPriv(cEdit, item))
      continue;

    if (DEBUG)
      qDebug("Project sCopy() Project ID: %d)", item->id());  

    if (item->id() == -1)
    {
      QMessageBox::information(this, tr("Project Copy"), tr("Please select a project to copy first"));
      return;
    }
  
    ParameterList params;
    params.append("prj_id", item->id());
  
    projectCopy newdlg(parentWidget(), "", true);
    newdlg.set(params);
    newdlg.exec();
  }

  sFillList();
}

bool projects::getPriv(int mode, XTreeWidgetItem* item)
{
  if(item->altId() == PROJECT || item->altId() == TASK)
    if (mode==cEdit)
      return (omfgThis->username() == item->rawValue("prj_owner_username") &&
              _privileges->check("MaintainPersonalProjects")) ||
             (omfgThis->username() == item->rawValue("prj_username") &&
              _privileges->check("MaintainPersonalProjects")) ||
             _privileges->check("MaintainAllProjects");
    else
      return (omfgThis->username() == item->rawValue("prj_owner_username") &&
              _privileges->check("ViewPersonalProjects")) ||
             (omfgThis->username() == item->rawValue("prj_username") &&
              _privileges->check("ViewPersonalProjects")) ||
             _privileges->check("ViewAllProjects");


  if(item->altId() == QUOTE || item->altId() == QUOTEITEM)
    if (mode==cEdit)
      return _privileges->check("MaintainQuotes");
    else
      return _privileges->check("MaintainQuotes ViewQuotes");

  if(item->altId() == SALESORDER || item->altId() == SALESORDERITEM)
    if (mode==cEdit)
      return _privileges->check("MaintainSalesOrders");
    else
      return _privileges->check("MaintainSalesOrders ViewSalesOrders");

  if(item->altId() == INVOICE || item->altId() == INVOICEITEM)
    if (mode==cEdit)
      return _privileges->check("MaintainMiscInvoices");
    else
      return _privileges->check("MaintainMiscInvoices ViewMiscInvoices");

  if(item->altId() == WORKORDER)
    if (mode==cEdit)
      return _privileges->check("MaintainWorkOrders");
    else
      return _privileges->check("MaintainWorkOrders ViewWorkOrders");

  if(item->altId() == PURCHASEREQUEST)
    if (mode==cEdit)
      return false;
    else
      return _privileges->check("MaintainPurchaseRequests ViewPurchaseRequests");

  if(item->altId() == PURCHASEORDER || item->altId() == PURCHASEORDERITEM)
    if (mode==cEdit)
      return _privileges->check("MaintainPurchaseOrders");
    else
      return _privileges->check("MaintainPurchaseOrders ViewPurchaseOrders");

  if(item->altId() == INCIDENT)
    if (mode==cEdit)
      return _privileges->check("MaintainPersonalIncidents MaintainAllIncidents");
    else
      return _privileges->check("ViewPersonalIncidents ViewAllIncidents MaintainPersonalIncidents "
                                "MaintainAllIncidents");

  return false;
}

bool projects::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  bool valid;
  QVariant param;
  
  param = params.value("prjstatus_id", &valid);
  if (valid)
    params.append("prjstatus", _statuses.at(param.toInt()));
  
  if (_showComplete->isChecked())
    params.append("showComplete",true);

  if (_salesOrders->isChecked())
    params.append("showSo", true);

  if (_workOrders->isChecked())
    params.append("showWo", true);

  if (_purchaseOrders->isChecked())
    params.append("showPo", true);

  if (_incidents->isChecked())
    params.append("showIn", true);

  params.append("planning", tr("Concept"));
  params.append("open", tr("In-Process"));
  params.append("complete", tr("Complete"));
  params.append("undefined", tr("Undefined"));
  params.append("so", tr("Sales Order"));
  params.append("wo", tr("Work Order"));
  params.append("po", tr("Purchase Order"));
  params.append("pr", tr("Purchase Request"));
  params.append("sos", tr("Sales Orders"));
  params.append("wos", tr("Work Orders"));
  params.append("pos", tr("Purchase Orders"));
  params.append("prs", tr("Purchase Requests"));
  params.append("quote", tr("Quote"));
  params.append("quotes", tr("Quotes"));
  params.append("invoice", tr("Invoice"));
  params.append("invoices", tr("Invoices"));
  params.append("task", tr("Task"));
  params.append("tasks", tr("Tasks"));

  params.append("open", tr("Open"));
  params.append("closed", tr("Closed"));
  params.append("converted", tr("Converted"));
  params.append("canceled", tr("Canceled"));
  params.append("expired", tr("Expired"));
  params.append("unposted", tr("Unposted"));
  params.append("posted", tr("Posted"));
  params.append("exploded", tr("Exploded"));
  params.append("released", tr("Released"));
  params.append("planning", tr("Concept"));
  params.append("inprocess", tr("In Process"));
  params.append("complete", tr("Complete"));
  params.append("unreleased", tr("Unreleased"));
  params.append("total", tr("Total"));

  return true;
}

void projects::sOpen()
{
  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("MaintainPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("MaintainPersonalProjects")) ||
      (_privileges->check("MaintainAllProjects"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("ViewPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("ViewPersonalProjects")) ||
      (_privileges->check("ViewAllProjects"));

  if (editPriv)
    sEdit();
  else if (viewPriv)
    sView();

}

