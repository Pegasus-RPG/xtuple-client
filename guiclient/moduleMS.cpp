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

//  moduleMS.cpp
//  Create 08/22/2000 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

#include "plannedSchedules.h"
#include "plannedSchedule.h"

#include "plannedOrder.h"
#include "createPlannedOrdersByItem.h"
#include "createPlannedOrdersByPlannerCode.h"
#include "runMPSByPlannerCode.h"
#include "deletePlannedOrder.h"
#include "deletePlannedOrdersByPlannerCode.h"
#include "firmPlannedOrdersByPlannerCode.h"
#include "releasePlannedOrdersByPlannerCode.h"

#include "createBufferStatusByItem.h"
#include "createBufferStatusByPlannerCode.h"
#include "dspInventoryBufferStatusByParameterList.h"
#include "dspCapacityBufferStatusByWorkCenter.h"
#include "dspWoBufferStatusByParameterList.h"
#include "dspWoOperationBufrStsByWorkCenter.h"
#include "dspPoItemsByBufferStatus.h"

#include "dspPlannedOrdersByItem.h"
#include "dspPlannedOrdersByPlannerCode.h"
#include "dspMRPDetail.h"
#include "dspMPSDetail.h"
#include "dspRoughCutByWorkCenter.h"
#include "dspTimePhasedRoughCutByWorkCenter.h"
#include "dspPlannedRevenueExpensesByPlannerCode.h"
#include "dspTimePhasedPlannedREByPlannerCode.h"
#include "dspRunningAvailability.h"
#include "dspTimePhasedAvailability.h"
#include "dspExpediteExceptionsByPlannerCode.h"
#include "dspReorderExceptionsByPlannerCode.h"

#include "rptPlannedOrdersByItem.h"
#include "rptPlannedOrdersByPlannerCode.h"
#include "rptMRPDetail.h"
#include "rptMPSDetail.h"
#include "rptRoughCutByWorkCenter.h"
#include "rptTimePhasedRoughCutByWorkCenter.h"
#include "rptPlannedRevenueExpensesByPlannerCode.h"
#include "rptTimePhasedPlannedREByPlannerCode.h"
#include "rptRunningAvailability.h"
#include "rptExpediteExceptionsByPlannerCode.h"
#include "rptReorderExceptionsByPlannerCode.h"

#include "plannerCodes.h"
#include "whseWeek.h"
#include "whseCalendars.h"

#include "moduleMS.h"

moduleMS::moduleMS(OpenMFGGUIClient *Pparent) :
 QObject(Pparent, "msModule")
{
  parent = Pparent;
  
  toolBar = new QToolBar(tr("M/S Tools"));
  toolBar->setObjectName("M/S Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowMSToolbar"))
    parent->addToolBar(toolBar);


//  Scheduling
  planningMenu = new QMenu();

  parent->actions.append( new Action( parent, "ms.listProductionPlans", tr("List Production Plans..."),
                                      this, SLOT(sListProductionPlans()),
                                      planningMenu, (_privleges->check("MaintainPlannedSchedules") || _privleges->check("ViewPlannedSchedules")) ) );

  parent->actions.append( new Action( parent, "ms.newProductionPlan", tr("New Production Plan..."),
                                      this, SLOT(sNewProductionPlan()),
                                      planningMenu, _privleges->check("MaintainPlannedSchedules") ) );

//  Planned Ordered
  plannedOrdersMenu = new QMenu();

  parent->actions.append( new Action( parent, "ms.createPlannedOrder", tr("Create Planned Order..."),
                                      this, SLOT(sCreatePlannedOrder()),
                                      plannedOrdersMenu, _privleges->check("CreatePlannedOrders") ) );

  plannedOrdersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.runMPSByPlannerCode", tr("Run MPS by Planner Code..."),
                                      this, SLOT(sRunMPSByPlannerCode()),
                                      plannedOrdersMenu, _privleges->check("CreatePlannedOrders")));

  parent->actions.append( new Action( parent, "ms.runMRPByPlannerCode", tr("Run MRP by Planner Code..."),
                                      this, SLOT(sCreatePlannedReplenOrdersByPlannerCode()),
                                      plannedOrdersMenu, _privleges->check("CreatePlannedOrders"),
									  QPixmap(":/images/runMrpByPlannerCode.png"), toolBar ) );

  parent->actions.append( new Action( parent, "ms.runMRPByItem", tr("Run MRP by Item..."),
                                      this, SLOT(sCreatePlannedReplenOrdersByItem()),
                                      plannedOrdersMenu, _privleges->check("CreatePlannedOrders") ) );

  plannedOrdersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.firmPlannedOrdersByPlannerCode", tr("Firm Planned Orders by Planner Code..."),
                                      this, SLOT(sFirmPlannedOrdersByPlannerCode()),
                                      plannedOrdersMenu, _privleges->check("FirmPlannedOrders") ) );

  parent->actions.append( new Action( parent, "ms.releasePlannedOrdersByPlannerCode", tr("Release Planned Orders by Planner Code..."),
                                      this, SLOT(sReleasePlannedOrdersByPlannerCode()),
                                      plannedOrdersMenu, _privleges->check("ReleasePlannedOrders") ) );

  plannedOrdersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.deletePlannedOrdersByPlannerCode", tr("Delete Planned Orders by Planner Code..."),
                                      this, SLOT(sDeletePlannedOrdersByPlannerCode()),
                                      plannedOrdersMenu, _privleges->check("DeletePlannedOrders") ) );

  parent->actions.append( new Action( parent, "ms.deletePlannedOrder", tr("Delete Planned Order..."),
                                      this, SLOT(sDeletePlannedOrder()),
                                      plannedOrdersMenu, _privleges->check("DeletePlannedOrders") ) );

//  Buffer Management
  bufferManagementMenu = new QMenu();

  parent->actions.append( new Action( parent, "ms.runBufferStatusByItem", tr("Run Buffer Status by Item..."),
                                      this, SLOT(sCreateBufferStatusByItem()),
                                      bufferManagementMenu, _privleges->check("CreateBufferStatus") ) );

  parent->actions.append( new Action( parent, "ms.runBufferStatusByPlannerCode", tr("Run Buffer Status by Planner Code..."),
                                      this, SLOT(sCreateBufferStatusByPlannerCode()),
                                      bufferManagementMenu, _privleges->check("CreateBufferStatus") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspInventoryBufferStatusByItemGroup", tr("Inventory Buffer Status by Item Group..."),
                                      this, SLOT(sDspInventoryBufferStatusByItemGroup()),
                                      bufferManagementMenu, _privleges->check("ViewInventoryBufferStatus") ) );

  parent->actions.append( new Action( parent, "ms.dspInventoryBufferStatusByClassCode", tr("Inventory Buffer Status by Class Code..."),
                                      this, SLOT(sDspInventoryBufferStatusByClassCode()),
                                      bufferManagementMenu, _privleges->check("ViewInventoryBufferStatus") ) );

  parent->actions.append( new Action( parent, "ms.dspInventoryBufferStatusByPlannerCode", tr("Inventory Buffer Status by Planner Code..."),
                                      this, SLOT(sDspInventoryBufferStatusByPlannerCode()),
                                      bufferManagementMenu, _privleges->check("ViewInventoryBufferStatus") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspCapacityBufferStatusByWorkCenter", tr("Capacity Buffer Status by Work Center..."),
                                      this, SLOT(sDspCapacityBufferStatusByWorkCenter()),
                                      bufferManagementMenu, _privleges->check("ViewWorkCenterBufferStatus") && _metrics->boolean("Routings") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspWoBufferStatusByItemGroup", tr("Work Order Buffer Status by Item Group..."),
                                      this, SLOT(sDspWoBufferStatusByItemGroup()),
                                      bufferManagementMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "ms.dspWoBufferStatusByClassCode", tr("Work Order Buffer Status by Class Code..."),
                                      this, SLOT(sDspWoBufferStatusByClassCode()),
                                      bufferManagementMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "ms.dspWoBufferStatusByPlannerCode", tr("Work Order Buffer Status by Planner Code..."),
                                      this, SLOT(sDspWoBufferStatusByPlannerCode()),
                                      bufferManagementMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "ms.dspWoOperationBufrStsByWorkCenter", tr("W/O Operation Buffer Status by Work Center..."),
                                      this, SLOT(sDspWoOperationBufrStsByWorkCenter()),
                                      bufferManagementMenu, (_privleges->check("MaintainWoOperations") || _privleges->check("ViewWoOperations")) &&  _metrics->boolean("Routings") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspPoLineItemsByBufferStatus", tr("P/O Items by Buffer Status..."),
                                      this, SLOT(sDspPoItemsByBufferStatus()),
                                      bufferManagementMenu, _privleges->check("ViewPurchaseOrders") ) );

//  Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "ms.dspPlannedOrdersByItem", tr("Planned Orders by Item..."),
                                      this, SLOT(sDspPlannedOrdersByItem()),
                                      displaysMenu, _privleges->check("ViewPlannedOrders") ) );

  parent->actions.append( new Action( parent, "ms.dspPlannedOrdersByPlannerCode", tr("Planned Orders by Planner Code..."),
                                      this, SLOT(sDspPlannedOrdersByPlannerCode()),
                                      displaysMenu, _privleges->check("ViewPlannedOrders"),
									  QPixmap(":/images/dspPlannedOrdersByPlannerCode.png"), toolBar ) );

  parent->actions.append( new Action( parent, "ms.dspMPSDetail", tr("MPS Detail..."),
                                      this, SLOT(sDspMPSDetail()),
                                      displaysMenu, _privleges->check("ViewMPS") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspRoughCutByWorkCenter", tr("Rough Cut Capacity Plan by Work Center..."),
                                      this, SLOT(sDspRoughCutByWorkCenter()),
                                      displaysMenu, _privleges->check("ViewRoughCut") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "ms.dspTimePhasedRoughCutByWorkCenter", tr("Time-Phased Rough Cut Capacity Plan by Work Center..."),
                                      this, SLOT(sDspTimePhasedRoughCutByWorkCenter()),
                                      displaysMenu, _privleges->check("ViewRoughCut") && _metrics->boolean("Routings") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspPlannedRevenue/ExpensesByPlannerCode", tr("Planned Revenue/Expenses by Planner Code..."),
                                      this, SLOT(sDspPlannedRevenueExpensesByPlannerCode()),
                                      displaysMenu, (_privleges->check("ViewPlannedOrders") && _privleges->check("ViewCosts") && _privleges->check("ViewListPrices")) ) );

  parent->actions.append( new Action( parent, "ms.dspTimePhasedPlannedRevenue/ExpensesByPlannerCode", tr("Time-Phased Planned Revenue/Expenses by Planner Code..."),
                                      this, SLOT(sDspTimePhasedPlannedREByPlannerCode()),
                                      displaysMenu, (_privleges->check("ViewPlannedOrders") && _privleges->check("ViewCosts") && _privleges->check("ViewListPrices")) ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspTimePhasedAvailabiltiy", tr("Time-Phased Availability..."),
                                      this, SLOT(sDspTimePhasedAvailability()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

  parent->actions.append( new Action( parent, "ms.dspRunningAvailability", tr("Running Availability..."),
                                      this, SLOT(sDspRunningAvailability()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

  parent->actions.append( new Action( parent, "ms.dspMRPDetail", tr("MRP Detail..."),
                                      this, SLOT(sDspMRPDetail()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.dspExpediteExceptionsByPlannerCode", tr("Expedite Exceptions by Planner Code..."),
                                      this, SLOT(sDspExpediteExceptionsByPlannerCode()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

  parent->actions.append( new Action( parent, "ms.dspReorderExceptionsByPlannerCode", tr("Reorder Exceptions by Planner Code..."),
                                      this, SLOT(sDspReorderExceptionsByPlannerCode()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

//  Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "ms.rptPlannedOrdersByItem", tr("Planned Orders by Item..."),
                                      this, SLOT(sRptPlannedOrdersByItem()),
                                      reportsMenu, _privleges->check("ViewPlannedOrders") ) );

  parent->actions.append( new Action( parent, "ms.rptPlannedOrdersByPlannerCode", tr("Planned Orders by Planner Code..."),
                                      this, SLOT(sRptPlannedOrdersByPlannerCode()),
                                      reportsMenu, _privleges->check("ViewPlannedOrders") ) );

  parent->actions.append( new Action( parent, "ms.rptMPSDetail", tr("MPS Detail..."),
                                      this, SLOT(sRptMPSDetail()),
                                      reportsMenu, _privleges->check("ViewMPS") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.rptRoughCutByWorkCenter", tr("Rough Cut Capacity Plan by Work Center..."),
                                      this, SLOT(sRptRoughCutByWorkCenter()),
                                      reportsMenu, _privleges->check("ViewRoughCut") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "ms.rptTimePhasedRoughCutByWorkCenter", tr("Time-Phased Rough Cut Capacity Plan by Work Center..."),
                                      this, SLOT(sRptTimePhasedRoughCutByWorkCenter()),
                                      reportsMenu, _privleges->check("ViewRoughCut") && _metrics->boolean("Routings") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.rptPlannedRevenue/ExpensesByPlannerCode", tr("Planned Revenue/Expenses by Planner Code..."),
                                      this, SLOT(sRptPlannedRevenueExpensesByPlannerCode()),
                                      reportsMenu, _privleges->check("ViewPlannedOrders") ) );

  parent->actions.append( new Action( parent, "ms.rptTimePhasedPlannedRevenue/ExpensesByPlannerCode", tr("Time-Phased Planned Revenue/Expenses by Planner Code..."),
                                      this, SLOT(sRptTimePhasedPlannedREByPlannerCode()),
                                      reportsMenu, _privleges->check("ViewPlannedOrders") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.rptRunningAvailability", tr("Running Availability..."),
                                      this, SLOT(sRptRunningAvailability()),
                                      reportsMenu, _privleges->check("ViewInventoryAvailability") ) );

  parent->actions.append( new Action( parent, "ms.rptMRPDetail", tr("MRP Detail..."),
                                      this, SLOT(sRptMRPDetail()),
                                      reportsMenu, _privleges->check("ViewInventoryAvailability") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ms.rptExpediteExceptionsByPlannerCode", tr("Expedite Exceptions by Planner Code..."),
                                      this, SLOT(sRptExpediteExceptionsByPlannerCode()),
                                      reportsMenu, _privleges->check("ViewInventoryAvailability") ) );

  parent->actions.append( new Action( parent, "ms.rptReorderExceptionsByPlannerCode", tr("Reorder Exceptions by Planner Code..."),
                                      this, SLOT(sRptReorderExceptionsByPlannerCode()),
                                      reportsMenu, _privleges->check("ViewInventoryAvailability") ) );


//  Master Information
  masterInfoMenu = new QMenu();

  parent->actions.append( new Action( parent, "ms.plannerCodes", tr("Planner Codes..."),
                                      this, SLOT(sPlannerCodes()),
                                      masterInfoMenu, (_privleges->check("MaintainPlannerCodes") && _privleges->check("ViewPlannerCodes")) ) );

  parent->actions.append( new Action( parent, "ms.warehouseWeek", tr("Warehouse Week..."),
                                      this, SLOT(sWarehouseWeek()),
                                      masterInfoMenu, _privleges->check("MaintainWarehouseWorkWeek") ) );

  parent->actions.append( new Action( parent, "ms.warehouseCalendarExceptions", tr("Warehouse Calendar Exceptions..."),
                                      this, SLOT(sWarehouseCalendarExceptions()),
                                      masterInfoMenu, (_privleges->check("MaintainWarehouseCalendarExceptions") || _privleges->check("ViewWarehouseCalendarExceptions")) ) );

  mainMenu = new QMenu();
  mainMenu->insertItem(tr("Planning"), planningMenu);
  mainMenu->insertItem(tr("Scheduling"), plannedOrdersMenu);
  if (_metrics->boolean("BufferMgt"))
    mainMenu->insertItem(tr("Buffer Management"), bufferManagementMenu);
  mainMenu->insertItem(tr("Displays"), displaysMenu);
  mainMenu->insertItem(tr("Reports"), reportsMenu);
  mainMenu->insertItem(tr("Master Information"), masterInfoMenu);
  parent->populateCustomMenu(mainMenu, "M/S");
  parent->menuBar()->insertItem(tr("&M/S"), mainMenu);
}

void moduleMS::sCreatePlannedOrder()
{
  ParameterList params;
  params.append("mode", "new");

  plannedOrder newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleMS::sCreatePlannedReplenOrdersByItem()
{
  createPlannedOrdersByItem(parent, "", TRUE).exec();
}

void moduleMS::sCreatePlannedReplenOrdersByPlannerCode()
{
  createPlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sRunMPSByPlannerCode()
{
  runMPSByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sDeletePlannedOrder()
{
  deletePlannedOrder(parent, "", TRUE).exec();
}

void moduleMS::sDeletePlannedOrdersByPlannerCode()
{
  deletePlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sFirmPlannedOrdersByPlannerCode()
{
  firmPlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sReleasePlannedOrdersByPlannerCode()
{
  releasePlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sCreateBufferStatusByItem()
{
  createBufferStatusByItem(parent, "", TRUE).exec();
}

void moduleMS::sCreateBufferStatusByPlannerCode()
{
  createBufferStatusByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sDspInventoryBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleMS::sDspInventoryBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleMS::sDspInventoryBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleMS::sDspCapacityBufferStatusByWorkCenter()
{
  omfgThis->handleNewWindow(new dspCapacityBufferStatusByWorkCenter());
}

void moduleMS::sDspWoBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleMS::sDspWoBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleMS::sDspWoBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleMS::sDspWoOperationBufrStsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspWoOperationBufrStsByWorkCenter());
}

void moduleMS::sDspPoItemsByBufferStatus()
{
  omfgThis->handleNewWindow(new dspPoItemsByBufferStatus());
}

void moduleMS::sDspPlannedOrdersByItem()
{
  omfgThis->handleNewWindow(new dspPlannedOrdersByItem());
}

void moduleMS::sDspPlannedOrdersByPlannerCode()
{
  omfgThis->handleNewWindow(new dspPlannedOrdersByPlannerCode());
}

void moduleMS::sDspMPSDetail()
{
  omfgThis->handleNewWindow(new dspMPSDetail());
}

void moduleMS::sDspRoughCutByWorkCenter()
{
  omfgThis->handleNewWindow(new dspRoughCutByWorkCenter());
}

void moduleMS::sDspTimePhasedRoughCutByWorkCenter()
{
  omfgThis->handleNewWindow(new dspTimePhasedRoughCutByWorkCenter());
}

void moduleMS::sDspPlannedRevenueExpensesByPlannerCode()
{
  omfgThis->handleNewWindow(new dspPlannedRevenueExpensesByPlannerCode());
}

void moduleMS::sDspTimePhasedPlannedREByPlannerCode()
{
  omfgThis->handleNewWindow(new dspTimePhasedPlannedREByPlannerCode());
}

void moduleMS::sDspTimePhasedAvailability()
{
  omfgThis->handleNewWindow(new dspTimePhasedAvailability());
}

void moduleMS::sDspRunningAvailability()
{
  omfgThis->handleNewWindow(new dspRunningAvailability());
}

void moduleMS::sDspMRPDetail()
{
  omfgThis->handleNewWindow(new dspMRPDetail());
}

void moduleMS::sDspExpediteExceptionsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspExpediteExceptionsByPlannerCode());
}

void moduleMS::sDspReorderExceptionsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspReorderExceptionsByPlannerCode());
}

void moduleMS::sRptPlannedOrdersByItem()
{
  rptPlannedOrdersByItem(parent, "", TRUE).exec();
}

void moduleMS::sRptPlannedOrdersByPlannerCode()
{
  rptPlannedOrdersByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sRptMPSDetail()
{
  rptMPSDetail(parent, "", TRUE).exec();
}

void moduleMS::sRptRoughCutByWorkCenter()
{
  rptRoughCutByWorkCenter(parent, "", TRUE).exec();
}

void moduleMS::sRptTimePhasedRoughCutByWorkCenter()
{
  rptTimePhasedRoughCutByWorkCenter(parent, "", TRUE).exec();
}

void moduleMS::sRptPlannedRevenueExpensesByPlannerCode()
{
  rptPlannedRevenueExpensesByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sRptTimePhasedPlannedREByPlannerCode()
{
  rptTimePhasedPlannedREByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sRptRunningAvailability()
{
  rptRunningAvailability(parent, "", TRUE).exec();
}

void moduleMS::sRptMRPDetail()
{
  rptMRPDetail(parent, "", TRUE).exec();
}

void moduleMS::sRptExpediteExceptionsByPlannerCode()
{
  rptExpediteExceptionsByPlannerCode(parent, "", TRUE).exec();
}

void moduleMS::sRptReorderExceptionsByPlannerCode()
{
  rptReorderExceptionsByPlannerCode(parent, "", TRUE).exec();
}


void moduleMS::sPlannerCodes()
{
  omfgThis->handleNewWindow(new plannerCodes());
}

void moduleMS::sListProductionPlans()
{
  omfgThis->handleNewWindow(new plannedSchedules());
}

void moduleMS::sNewProductionPlan()
{
  ParameterList params;
  params.append("mode", "new");

  plannedSchedule newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleMS::sWarehouseWeek()
{
  omfgThis->handleNewWindow(new whseWeek());
}

void moduleMS::sWarehouseCalendarExceptions()
{
  omfgThis->handleNewWindow(new whseCalendars());
}

