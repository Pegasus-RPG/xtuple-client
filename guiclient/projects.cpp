/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
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

#define DEBUG false

projects::projects(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "projects", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Projects"));
  setMetaSQLOptions("projects", "detail");
  setReportName("ListProjectsDetail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllProjects") && !_privileges->check("MaintainPersonalProjects"))
    newAction()->setEnabled(FALSE);

  list()->addColumn(tr("Number"),       _orderColumn,  Qt::AlignLeft,   true,  "prj_number");
  list()->addColumn(tr("Name"),                   -1,  Qt::AlignLeft,   true,  "prj_name");
  list()->addColumn(tr("Description"),            -1,  Qt::AlignLeft,   true,  "prj_descrip");
  list()->addColumn(tr("Status"),        _itemColumn,  Qt::AlignCenter, true,  "prj_status" );
  list()->addColumn(tr("Project Type"),        _itemColumn,  Qt::AlignCenter, true,  "prjtype_code" );
  list()->addColumn(tr("Owner"),         _userColumn,  Qt::AlignLeft,   false, "prj_owner_username");
  list()->addColumn(tr("Assigned To"),   _userColumn,  Qt::AlignLeft,   true,  "prj_username");
  list()->addColumn(tr("CRM Account/Customer"),   _userColumn,  Qt::AlignLeft,   true,  "crmacct_number");
  list()->addColumn(tr("Contact"),       _userColumn,  Qt::AlignLeft,   false,  "contact_name");
  list()->addColumn(tr("City"),       -1,  Qt::AlignLeft,   false,  "contact_city");
  list()->addColumn(tr("State"),       -1,  Qt::AlignLeft,   false,  "contact_state");
  list()->addColumn(tr("Due"),           _dateColumn,  Qt::AlignCenter, true,  "prj_due_date");
  list()->addColumn(tr("Assigned"),      _dateColumn,  Qt::AlignCenter, true,  "prj_assigned_date");
  list()->addColumn(tr("Started"),       _dateColumn,  Qt::AlignCenter, true,  "prj_start_date");
  list()->addColumn(tr("Completed"),     _dateColumn,  Qt::AlignCenter, true,  "prj_completed_date");
  list()->addColumn(tr("Budget Hrs."),   _costColumn,  Qt::AlignRight,  true,  "budget_hrs");
  list()->addColumn(tr("Actual Hrs."),   _costColumn,  Qt::AlignRight,  true,  "actual_hrs");
  list()->addColumn(tr("Balance Hrs."),  _costColumn,  Qt::AlignRight,  true,  "balance_hrs");
  list()->addColumn(tr("Budget Exp."),   _priceColumn,  Qt::AlignRight,  false,  "budget_exp");
  list()->addColumn(tr("Actual Exp."),   _priceColumn,  Qt::AlignRight,  false,  "actual_exp");
  list()->addColumn(tr("Balance Exp."),  _priceColumn,  Qt::AlignRight,  false,  "balance_exp");

  connect(omfgThis, SIGNAL(projectsUpdated(int)), this, SLOT(sFillList()));
  connect(_showComplete, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_salesOrders, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_workOrders, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_purchaseOrders, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  QString qryType = QString( "SELECT prjtype_id, prjtype_descr FROM prjtype " );
  QString qryTask = QString( "SELECT prjtask_id, prjtask_number	 FROM prjtask " );

  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("AssignedTo"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("CRM Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Contact"), "cntct_id", ParameterWidget::Contact);
  parameterWidget()->appendComboBox(tr("Project Type"), "project_type", qryType);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);
  parameterWidget()->appendComboBox(tr("Project Task"), "project_task", qryTask);
  parameterWidget()->append(tr("Sales Order"), "cohead_id", ParameterWidget::SalesOrder);
  parameterWidget()->append(tr("Work Order"), "wo_id", ParameterWidget::WorkOrder);
  parameterWidget()->append(tr("Purchase Order"), "pohead_id", ParameterWidget::PurchaseOrder);
  parameterWidget()->append(tr("Start Start Date"), "startStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Start End Date"), "startEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Due Start Date"), "dueStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Due End Date"), "dueEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Assigned Start Date"), "assignedStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Assigned End Date"), "assignedEndDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Completed Start Date"), "completedStartDate", ParameterWidget::Date, QDate::currentDate());
  parameterWidget()->append(tr("Completed End Date"), "completedEndDate", ParameterWidget::Date, QDate::currentDate());

  _salesOrders->setChecked(true);
  _workOrders->setChecked(true);
  _purchaseOrders->setChecked(true);

  sFillList();
}

void projects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sEdit()
{
  ParameterList params;
  params.append("prj_id", list()->id());
  params.append("mode", "edit");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sView()
{
  ParameterList params;
  params.append("prj_id", list()->id());
  params.append("mode", "view");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void projects::sDelete()
{
  XSqlQuery projectsDelete;
  projectsDelete.prepare("SELECT deleteProject(:prj_id) AS result");
  projectsDelete.bindValue(":prj_id", list()->id());
  projectsDelete.exec();
  if(projectsDelete.first())
  {
    int result = projectsDelete.value("result").toInt();
    if(result < 0)
    {
      QString errmsg;
      switch(result)
      {
        case -1:
          errmsg = tr("One or more Quote's refer to this project.");
          break;
        case -2:
          errmsg = tr("One or more Sales Orders refer to this project.");
          break;
        case -3:
          errmsg = tr("One or more Work Orders refer to this project.");
          break;
        case -4:
          errmsg = tr("One or more Purchase Requests refer to this project.");
          break;
        case -5:
          errmsg = tr("One or more Purchase order Items refer to this project.");
          break;
        case -6:
          errmsg = tr("One or more Invoices refer to this project.");
          break;
        default:
          errmsg = tr("Error #%1 encountered while trying to delete project.").arg(result);
      }
      QMessageBox::critical( this, tr("Cannot Delete Project"),
        tr("Could not delete the project for one or more reasons.\n") + errmsg);
      return;
    }
    else if (projectsDelete.lastError().type() != QSqlError::NoError)
    {
      systemError(this, projectsDelete.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  sFillList();
}

void projects::sCopy()
{
  if (DEBUG)
    qDebug("Project sCopy() Project ID: %d)", list()->id());  

  if (list()->id() == -1)
  {
    QMessageBox::information(this, tr("Project Copy"), tr("Please select a project to copy first"));
    return;
  }
  
  ParameterList params;
  params.append("prj_id", list()->id());
  
  projectCopy newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  sFillList();
}

void projects::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("MaintainPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("MaintainPersonalProjects")) ||
      (_privileges->check("MaintainAllProjects"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("prj_owner_username") && _privileges->check("ViewPersonalProjects")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("prj_username") && _privileges->check("ViewPersonalProjects")) ||
      (_privileges->check("ViewAllProjects"));

  menuItem = pMenu->addAction("View...", this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction("Delete...", this, SLOT(sDelete()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction("Copy...", this, SLOT(sCopy()));
  menuItem->setEnabled(editPriv);
}

bool projects::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (_showComplete->isChecked())
    params.append("showComplete",true);

  if (_salesOrders->isChecked())
    params.append("showSo", true);

  if (_workOrders->isChecked())
    params.append("showWo", true);

  if (_purchaseOrders->isChecked())
    params.append("showPo", true);

  params.append("planning", tr("Concept"));
  params.append("open", tr("In-Process"));
  params.append("complete", tr("Complete"));
  params.append("undefined", tr("Undefined"));

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

