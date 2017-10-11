/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoSchedule.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "bom.h"
#include "changeWoQty.h"
#include "closeWo.h"
#include "correctProductionPosting.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspRunningAvailability.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "issueWoMaterialItem.h"
#include "returnWoMaterialItem.h"
#include "postProduction.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "salesOrderInformation.h"
#include "storedProcErrorLookup.h"
#include "workOrder.h"
#include "parameterwidget.h"
#include "errorReporter.h"

dspWoSchedule::dspWoSchedule(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspWoSchedule", fl)
{
  setNewVisible(true);
  setSearchVisible(true);
  setWindowTitle(tr("Work Order Schedule"));
  setReportName("WOSchedule");
  setMetaSQLOptions("workOrderSchedule", "detail");
  setUseAltId(true);
  setAutoUpdateEnabled(true);
  setParameterWidgetVisible(true);
  setQueryOnStartEnabled(true);

  QString qryStatus = QString( "SELECT  'O', '%1' UNION "
                               "SELECT  'E', '%2' UNION "
                               "SELECT  'R', '%3' UNION "
                               "SELECT  'I', '%4'")
      .arg(tr("Open"))
      .arg(tr("Exploded"))
      .arg(tr("Released"))
      .arg(tr("In-Process"));

  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date);
  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->append(tr("Item Group Pattern"), "itemgrp_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Planner Code"), "plancode_id", XComboBox::PlannerCodes);
  parameterWidget()->append(tr("Planner Code Pattern"), "plancode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Show Only Top Level"), "showOnlyTopLevel", ParameterWidget::Exists);
  parameterWidget()->append(tr("Status"), "status_list",
                           ParameterWidget::Multiselect, QVariant(), false,
                           qryStatus);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Has Parent Sales Order"), "woSoStatus", ParameterWidget::Exists);
  parameterWidget()->append(tr("Has Closed Parent Sales Order"), "woSoStatusMismatch", ParameterWidget::Exists);
  parameterWidget()->append(tr("Project"), "prj_id", ParameterWidget::Project);

  list()->addColumn(tr("Work Order #"),   _orderColumn,  Qt::AlignLeft,   true,  "wonumber"   );
  list()->addColumn(tr("Parent Type"),    _orderColumn,  Qt::AlignCenter, true,  "wo_ordtype" );
  list()->addColumn(tr("Parent Order #"), _orderColumn,  Qt::AlignLeft,   true,  "parentorder" );
  list()->addColumn(tr("Project #"),      _orderColumn,  Qt::AlignLeft,   true,  "prj_number" );
  list()->addColumn(tr("Status"),         _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  list()->addColumn(tr("Pri."),           _statusColumn, Qt::AlignCenter, false, "wo_priority" );
  list()->addColumn(tr("Site"),           _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),    _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),    -1,            Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("UOM"),            _uomColumn,    Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Ordered"),        _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  list()->addColumn(tr("Received"),       _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  list()->addColumn(tr("Start Date"),     _dateColumn,   Qt::AlignRight,  true,  "wo_startdate"  );
  list()->addColumn(tr("Due Date"),       _dateColumn,   Qt::AlignRight,  true,  "wo_duedate"  );
  list()->addColumn(tr("Condition"),      _dateColumn,   Qt::AlignLeft,   true,  "condition"   );

  list()->addColumn(tr("Wip Value"),      _bigMoneyColumn,   Qt::AlignLeft,   true,  "wo_wipvalue"   );
  list()->addColumn(tr("Cost Est."),      _bigMoneyColumn,   Qt::AlignLeft,   true,  "plancost"   );
  list()->addColumn(tr("Variance Est."),  _bigMoneyColumn,   Qt::AlignLeft,   true,  "wopvar"   );
  list()->addColumn(tr("Cost Method"),    _orderColumn,   Qt::AlignLeft,   true,  "costmethod"   );

  if (_privileges->check("MaintainWorkOrders"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

enum SetResponse dspWoSchedule::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  parameterWidget()->setSavedFilters();

  param = pParams.value("plancode_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Planner Code"), param);

  param = pParams.value("wrkcnt", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Work Center"), param);

  param = pParams.value("warehous_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Site"), param);

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param);

  param = pParams.value("startDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Start Date"), param);
  
  param = pParams.value("endDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("End Date"), param);

  param = pParams.value("status", &valid);
  if (valid)
  {
    QVariantList list;
    list.append(param.toString());
    parameterWidget()->setDefault(tr("Status"), list);
  }

  if (pParams.inList("run"))
  {
    parameterWidget()->applyDefaultFilterSet();
    sFillList();
    return NoError_Run;
  }   

  return NoError;
}

bool dspWoSchedule::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;
  params.append("open", tr("Open"));
  params.append("exploded", tr("Exploded"));
  params.append("released", tr("Released"));
  params.append("inprocess", tr("In-Process"));
  params.append("closed", tr("Closed"));

  params.append("wo", tr("W/O"));
  params.append("planord", tr("Planned Order"));
  params.append("mps", tr("MPS"));
  params.append("so", tr("S/O"));
  params.append("quote", tr("Quote"));

  params.append("overdue", tr("Overdue"));
  params.append("ontime", tr("On Time"));
  
  return true;
}

void dspWoSchedule::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSchedule::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSchedule::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  postProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  correctProductionPosting newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sReleaseWO()
{
  XSqlQuery dspReleaseWO;
  dspReleaseWO.prepare("SELECT releaseWo(:wo_id, false);");
  dspReleaseWO.bindValue(":wo_id", list()->id());
  dspReleaseWO.exec();

  omfgThis->sWorkOrdersUpdated(list()->id(), true);
}

void dspWoSchedule::sRecallWO()
{
  XSqlQuery dspRecallWO;
  dspRecallWO.prepare("SELECT recallWo(:wo_id, false);");
  dspRecallWO.bindValue(":wo_id", list()->id());
  dspRecallWO.exec();

  omfgThis->sWorkOrdersUpdated(list()->id(), true);
}

void dspWoSchedule::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  explodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  implodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sDeleteWO()
{
  XSqlQuery dspDeleteWO;
  dspDeleteWO.prepare( "SELECT wo_ordtype, itemsite_costmethod "
             "FROM wo JOIN itemsite ON (itemsite_id=wo_itemsite_id) "
             "WHERE (wo_id=:wo_id);" );
  dspDeleteWO.bindValue(":wo_id", list()->id());
  dspDeleteWO.exec();
  if (dspDeleteWO.first())
  {
    QString question;
    if (dspDeleteWO.value("wo_ordtype") == "W")
      question = tr("<p>The Work Order that you selected to delete is a child "
		    "of another Work Order.  If you delete the selected Work "
		    "Order then the Work Order Materials Requirements for the "
		    "Component Item will remain but the Work Order to relieve "
		    "that demand will not. Are you sure that you want to "
		    "delete the selected Work Order?" );
    else if (dspDeleteWO.value("wo_ordtype") == "S" && dspDeleteWO.value("itemsite_costmethod") == "J")
    {
      QMessageBox::critical(this, tr("Cannot Delete Work Order"),
                            tr("This Work Order is linked to a Sales Order and is a Job Costed Item.  The Work Order may not be deleted."));
      return;
    }
    else if (dspDeleteWO.value("wo_ordtype") == "S")
      question = tr("<p>The Work Order that you selected to delete was created "
		    "to satisfy Sales Order demand. If you delete the selected "
		    "Work Order then the Sales Order demand will remain but "
		    "the Work Order to relieve that demand will not. Are you "
		    "sure that you want to delete the selected Work Order?" );
    else
      question = tr("<p>Are you sure that you want to delete the selected "
		    "Work Order?");
    if (QMessageBox::question(this, tr("Delete Work Order?"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      return;
    }

    dspDeleteWO.prepare("SELECT deleteWo(:wo_id, true) AS returnVal;");
    dspDeleteWO.bindValue(":wo_id", list()->id());
    dspDeleteWO.exec();

    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order Information"),
                         dspDeleteWO, __FILE__, __LINE__))
    {
      return;
    }
    else
      omfgThis->sWorkOrdersUpdated(-1, true);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Work Order Information"),
                                dspDeleteWO, __FILE__, __LINE__))
  {
    return;
  }
}

void dspWoSchedule::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  closeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  printWoTraveler newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSchedule::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSchedule::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  reprioritizeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  rescheduleWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  changeWoQty newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sViewParentSO()
{
  ParameterList params;
  params.append("soitem_id", list()->altId());

  salesOrderInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoSchedule::sViewParentWO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->altId());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSchedule::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;
  XTreeWidgetItem * item = (XTreeWidgetItem*)pSelected;
  QString  status = item->rawValue("wo_status").toString();
  QString  ordtype = item->rawValue("wo_ordtype").toString();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  pMenu->addSeparator();

  if (status == "E")
  {
    menuItem = pMenu->addAction(tr("Release"), this, SLOT(sReleaseWO()));
    menuItem->setEnabled(_privileges->check("ReleaseWorkOrders"));
  }
  else if (status == "R")
  {
    menuItem = pMenu->addAction(tr("Recall"), this, SLOT(sRecallWO()));
    menuItem->setEnabled(_privileges->check("RecallWorkOrders"));
  }

  if (status == "O")
  {
    menuItem = pMenu->addAction(tr("Explode..."), this, SLOT(sExplodeWO()));
    menuItem->setEnabled(_privileges->check("ExplodeWorkOrders"));
  }
  else if (status == "E")
  {
    menuItem = pMenu->addAction(tr("Implode..."), this, SLOT(sImplodeWO()));
    menuItem->setEnabled(_privileges->check("ImplodeWorkOrders"));
  }

  if ((status == "O") || (status == "E"))
  {
    menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteWO()));
    menuItem->setEnabled(_privileges->check("DeleteWorkOrders"));
  }
  else
  {
    menuItem = pMenu->addAction(tr("Close..."), this, SLOT(sCloseWO()));
    menuItem->setEnabled(_privileges->check("CloseWorkOrders"));
  }

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Bill of Materials..."), this, SLOT(sViewBOM()));
  menuItem->setEnabled(_privileges->check("ViewBOMs"));

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("View Material Requirements..."), this, SLOT(sViewWomatl()));
    menuItem->setEnabled(_privileges->check("ViewWoMaterials"));
      
    menuItem = pMenu->addAction(tr("Inventory Availability..."), this, SLOT(sInventoryAvailabilityByWorkOrder()));
    menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

    menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Print Traveler..."), this, SLOT(sPrintTraveler()));
    menuItem->setEnabled(_privileges->check("PrintWorkOrderPaperWork"));

    pMenu->addSeparator();

    if ((_metrics->boolean("IssueToExplodedWO") && status == "E") || status == "R" || status == "I")
    {
      menuItem = pMenu->addAction(tr("Issue Material Item..."), this, SLOT(sIssueWoMaterialItem()));
      menuItem->setEnabled(_privileges->check("IssueWoMaterials"));
    }

    menuItem = pMenu->addAction(tr("Return Material Item..."), this, SLOT(sReturnWoMaterialItem()));
    menuItem->setEnabled(_privileges->check("ReturnWoMaterials"));
    
    menuItem = pMenu->addAction(tr("Post Production..."), this, SLOT(sPostProduction()));
    menuItem->setEnabled(_privileges->check("PostProduction"));

    if (status != "E")
    {
      menuItem = pMenu->addAction(tr("Correct Production Posting..."), this, SLOT(sCorrectProductionPosting()));
      menuItem->setEnabled(_privileges->check("PostProduction"));
    }
  }

  pMenu->addSeparator();

  if ((status == "O") || (status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("Reprioritize..."), this, SLOT(sReprioritizeWo()));
    menuItem->setEnabled(_privileges->check("ReprioritizeWorkOrders"));
  }
  
  if ((status == "O") || (status == "E") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("Reschedule..."), this, SLOT(sRescheduleWO()));
    menuItem->setEnabled(_privileges->check("RescheduleWorkOrders"));

    menuItem = pMenu->addAction(tr("Change Quantity..."), this, SLOT(sChangeWOQty()));
    menuItem->setEnabled(_privileges->check("ChangeWorkOrderQty"));
  }
  
  if (list()->altId() != -1)
  {
    if (ordtype == "S")
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("View Parent Sales Order Information..."), this, SLOT(sViewParentSO()));
    }
    else if (ordtype == "W")
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("View Parent Work Order..."), this, SLOT(sViewParentWO()));
    }
  }
}

void dspWoSchedule::sIssueWoMaterialItem()
{
  issueWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("wo_id", list()->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void dspWoSchedule::sReturnWoMaterialItem()
{
  returnWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("wo_id", list()->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void dspWoSchedule::sDspRunningAvailability()
{
  XSqlQuery dspDspRunningAvailability;
  dspDspRunningAvailability.prepare("SELECT wo_itemsite_id FROM wo WHERE (wo_id=:id);");
  dspDspRunningAvailability.bindValue(":id", list()->id());
  dspDspRunningAvailability.exec();
  if (dspDspRunningAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", dspDspRunningAvailability.value("wo_itemsite_id"));
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                dspDspRunningAvailability, __FILE__, __LINE__))
  {
    return;
  }
}

void dspWoSchedule::sViewBOM()
{
  XSqlQuery dspViewBOM;
  dspViewBOM.prepare("SELECT itemsite_item_id "
	    "FROM wo, itemsite "
	    "WHERE ((wo_itemsite_id=itemsite_id)"
	    "  AND  (wo_id=:id));");
  dspViewBOM.bindValue(":id", list()->id());
  dspViewBOM.exec();
  if (dspViewBOM.first())
  {
    ParameterList params;
    params.append("item_id", dspViewBOM.value("itemsite_item_id"));
    params.append("mode", "view");

    BOM *newdlg = new BOM();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Information"),
                                dspViewBOM, __FILE__, __LINE__))
  {
    return;
  }
}

void dspWoSchedule::sNew()
{
    ParameterList params;
    params.append("mode", "new");
    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
}

