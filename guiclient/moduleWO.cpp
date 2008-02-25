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

//  moduleWO.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include "workOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "woTimeClock.h"
#include "closeWo.h"

#include "printWoTraveler.h"
#include "printWoPickList.h"
#include "printWoRouting.h"

#include "releaseWorkOrdersByPlannerCode.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "purgeClosedWorkOrders.h"

#include "woMaterialItem.h"
#include "deleteWoMaterialItem.h"
#include "workOrderMaterials.h"

#include "issueWoMaterialBatch.h"
#include "issueWoMaterialItem.h"
#include "returnWoMaterialBatch.h"
#include "returnWoMaterialItem.h"
#include "scrapWoMaterialFromWIP.h"

#include "postProduction.h"
#include "correctProductionPosting.h"
#include "postMiscProduction.h"

#include "woOperation.h"
#include "workOrderOperations.h"
#include "postOperations.h"
#include "correctOperationsPosting.h"

#include "dspWoScheduleByItem.h"
#include "dspWoScheduleByParameterList.h"
#include "dspWoBufferStatusByParameterList.h"
#include "dspWoHistoryByItem.h"
#include "dspWoHistoryByNumber.h"
#include "dspWoHistoryByClassCode.h"
#include "dspWoMaterialsByItem.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspPendingAvailability.h"
#include "dspWoOperationsByWorkOrder.h"
#include "dspWoOperationsByWorkCenter.h"
#include "dspWoOperationBufrStsByWorkCenter.h"
#include "dspMaterialUsageVarianceByBOMItem.h"
#include "dspMaterialUsageVarianceByItem.h"
#include "dspMaterialUsageVarianceByComponentItem.h"
#include "dspMaterialUsageVarianceByWorkOrder.h"
#include "dspMaterialUsageVarianceByWarehouse.h"
#include "dspLaborVarianceByBOOItem.h"
#include "dspLaborVarianceByItem.h"
#include "dspLaborVarianceByWorkCenter.h"
#include "dspLaborVarianceByWorkOrder.h"
#include "dspBreederDistributionVarianceByItem.h"
#include "dspBreederDistributionVarianceByWarehouse.h"
#include "dspWoSoStatus.h"
#include "dspWoSoStatusMismatch.h"
#include "dspWoEffortByUser.h"
#include "dspWoEffortByWorkOrder.h"

#include "printWoForm.h"
#include "printProductionEntrySheet.h"

#include "moduleWO.h"

moduleWO::moduleWO(GUIClient *Pparent) :
  QObject(Pparent, "poModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("W/O Tools"));
  toolBar->setObjectName("W/O Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowWOToolbar"))
    parent->addToolBar(toolBar);

//  Create the W/O menuitems
  controlMenu = new QMenu();

  if (_metrics->boolean("Routings"))
  {
  parent->actions.append( new Action( parent, "wo.woTimeClock", tr("Shop Floor Workbench..."),
                                      this, SLOT(sWoTimeClock()),
                                      controlMenu, _privleges->check("WoTimeClock"),
									  QPixmap(":/images/shopFloorWorkbench.png"), toolBar ) );

  controlMenu->insertSeparator();
  }

  parent->actions.append( new Action( parent, "wo.newWorkOrder", tr("Create New Work Order..."),
                                      this, SLOT(sNewWorkOrder()),
                                      controlMenu, _privleges->check("MaintainWorkOrders") ) );

  controlMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.explodeWorkOrder", tr("Explode Work Order..."),
                                      this, SLOT(sExplodeWorkOrder()),
                                      controlMenu, _privleges->check("ExplodeWorkOrders") ) );

  parent->actions.append( new Action( parent, "wo.implodeWorkOrder", tr("Implode Work Order..."),
                                      this, SLOT(sImplodeWorkOrder()),
                                      controlMenu, _privleges->check("ImplodeWorkOrders") ) );

  parent->actions.append( new Action( parent, "wo.closeWorkOrder", tr("Close Work Order..."),
                                      this, SLOT(sCloseWorkOrder()),
                                      controlMenu, _privleges->check("CloseWorkOrders") ) );

  controlMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.printTraveler", tr("Print Traveler..."),
                                      this, SLOT(sPrintTraveler()),
                                      controlMenu, _privleges->check("PrintWorkOrderPaperWork") ) );

  parent->actions.append( new Action( parent, "wo.printPickList", tr("Print Pick List..."),
                                      this, SLOT(sPrintPickList()),
                                      controlMenu, _privleges->check("PrintWorkOrderPaperWork") ) );
 
  if ( _metrics->boolean("Routings") )
  parent->actions.append( new Action( parent, "wo.printRouting", tr("Print Routing..."),
                                      this, SLOT(sPrintRouting()),
                                      controlMenu, _privleges->check("PrintWorkOrderPaperWork") ) );

  controlMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.releaseWorkOrdersByPlannerCode", tr("Release Work Orders by Planner Code..."),
                                      this, SLOT(sReleaseWorkOrdersByPlannerCode()),
                                      controlMenu, _privleges->check("ReleaseWorkOrders") ) );

  parent->actions.append( new Action( parent, "wo.reprioritizeWorkOrder", tr("Reprioritize Work Order..."),
                                      this, SLOT(sReprioritizeWorkOrder()),
                                      controlMenu, _privleges->check("ReprioritizeWorkOrders") ) );

  parent->actions.append( new Action( parent, "wo.rescheduleWorkOrder", tr("Reschedule Work Order..."),
                                      this, SLOT(sRescheduleWorkOrder()),
                                      controlMenu, _privleges->check("RescheduleWorkOrders") ) );

  parent->actions.append( new Action( parent, "wo.changeWorkOrderQuantity", tr("Change Work Order Quantity..."),
                                      this, SLOT(sChangeWorkOrderQty()),
                                      controlMenu, _privleges->check("ChangeWorkOrderQty") ) );

  controlMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.purgeClosedWorkOrder", tr("Purge Closed Work Orders..."),
                                      this, SLOT(sPurgeClosedWorkOrders()),
                                      controlMenu, _privleges->check("PurgeWorkOrders") ) );

//  W/O | W/O Materials
  materialsMenu = new QMenu();

  parent->actions.append( new Action( parent, "wo.createWoMaterialRequirement", tr("Create W/O Material Requirement..."),
                                      this, SLOT(sCreateWoMaterialRequirement()),
                                      materialsMenu, _privleges->check("MaintainWoMaterials") ) );

  parent->actions.append( new Action( parent, "wo.deleteWoMaterialRequirement", tr("Delete W/O Material Requirement..."),
                                      this, SLOT(sDeleteWoMaterialRequirement()),
                                      materialsMenu, _privleges->check("MaintainWoMaterials") ) );

  parent->actions.append( new Action( parent, "wo.maintainWoMaterialRequirements", tr("Maintain W/O Material Requirements..."),
                                      this, SLOT(sMaintainWoMaterials()),
                                      materialsMenu, _privleges->check("MaintainWoMaterials") ) );

  materialsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.issueWoMaterialBatch", tr("Issue W/O Material Batch..."),
                                      this, SLOT(sIssueWoMaterialBatch()),
                                      materialsMenu, _privleges->check("IssueWoMaterials") ) );

  parent->actions.append( new Action( parent, "wo.issueWoMaterialItem", tr("Issue W/O Material Item..."),
                                      this, SLOT(sIssueWoMaterialItem()),
                                      materialsMenu, _privleges->check("IssueWoMaterials") ) );

  parent->actions.append( new Action( parent, "wo.returnWoMaterialBatch", tr("Return W/O Material Batch..."),
                                      this, SLOT(sReturnWoMaterialBatch()),
                                      materialsMenu, _privleges->check("ReturnWoMaterials") ) );

  parent->actions.append( new Action( parent, "wo.returnWoMaterialItem", tr("Return W/O Material Item..."),
                                      this, SLOT(sReturnWoMaterialItem()),
                                      materialsMenu, _privleges->check("ReturnWoMaterials") ) );

  parent->actions.append( new Action( parent, "wo.scrapWoMaterialFromWo", tr("Scrap W/O Material from W/O..."),
                                      this, SLOT(sScrapWoMaterialFromWo()),
                                      materialsMenu, _privleges->check("ScrapWoMaterials") ) );

  materialsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.postProduction", tr("Post Production..."),
                                      this, SLOT(sPostProduction()),
                                      materialsMenu, _privleges->check("PostProduction") ) );

  parent->actions.append( new Action( parent, "wo.correctProductionPosting", tr("Correct Production Posting..."),
                                      this, SLOT(sCorrectProductionPosting()),
                                      materialsMenu, _privleges->check("PostProduction") ) );

  parent->actions.append( new Action( parent, "wo.postMiscProduction", tr("Post Miscellaneous Production..."),
                                      this, SLOT(sPostMiscProduction()),
                                      materialsMenu, _privleges->check("PostMiscProduction") ) );

//  W/O | Operations
  operationsMenu = new QMenu();

  parent->actions.append( new Action( parent, "wo.createWoOperation", tr("Create W/O Operation..."),
                                      this, SLOT(sCreateWoOperation()),
                                      operationsMenu, _privleges->check("MaintainWoOperations") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "wo.maintainWoOperation", tr("Maintain W/O Operations..."),
                                      this, SLOT(sMaintainWoOperations()),
                                      operationsMenu, _privleges->check("MaintainWoOperations") && _metrics->boolean("Routings") ) );

  operationsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.postOperations", tr("Post Operations..."),
                                      this, SLOT(sPostOperations()),
                                      operationsMenu, _privleges->check("PostWoOperations") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "wo.correctOperationsPosting", tr("Correct Operations Posting..."),
                                      this, SLOT(sCorrectOperationsPosting()),
                                      operationsMenu, _privleges->check("PostWoOperations") && _metrics->boolean("Routings") ) );

//  W/O | Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "wo.dspWoScheduleByItem", tr("Work Order Schedule by Item..."),
                                      this, SLOT(sDspWoScheduleByItem()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders"))));
//                                    QPixmap(":/images/dspWoScheduleByItem.xpm"), toolBar ) );

  parent->actions.append( new Action( parent, "wo.dspWoScheduleByItemGroup", tr("Work Order Schedule by Item Group..."),
                                      this, SLOT(sDspWoScheduleByItemGroup()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspWoScheduleByClassCode", tr("Work Order Schedule by Class Code..."),
                                      this, SLOT(sDspWoScheduleByClassCode()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspWoScheduleByPlannerCode", tr("Work Order Schedule by Planner Code..."),
                                      this, SLOT(sDspWoScheduleByPlannerCode()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")),
									  QPixmap(":/images/dspWoScheduleByPlannerCode.png"), toolBar ) );

  if (_metrics->boolean("Routings"))
      parent->actions.append( new Action( parent, "wo.dspWoScheduleByWorkCenter", tr("Work Order Schedule by Work Center..."),
                                          this, SLOT(sDspWoScheduleByWorkCenter()),
                                          displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) 
                                          && _metrics->boolean("Routings") ) );

  displaysMenu->insertSeparator();

 if (_metrics->boolean("BufferMgt") )
 {
    //  parent->actions.append( new Action( parent, "wo.dspWoBufferStatusByItem", tr("Work Order Buffer Status by Item..."),
    //                                      this, SLOT(sDspWoBufferStatusByItem()),
    //                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders"))  
    //                                      &&  (_metrics->value("Applicaion") == "OpenMFG") ) );

      parent->actions.append( new Action( parent, "wo.dspWoBufferStatusByItemGroup", tr("Work Order Buffer Status by Item Group..."),
                                          this, SLOT(sDspWoBufferStatusByItemGroup()),
                                          displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) 
                                          &&  _metrics->boolean("BufferMgt") ) );

      parent->actions.append( new Action( parent, "wo.dspWoBufferStatusByClassCode", tr("Work Order Buffer Status by Class Code..."),
                                          this, SLOT(sDspWoBufferStatusByClassCode()),
                                          displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders"))  
                                          &&  _metrics->boolean("BufferMgt") ) );

      parent->actions.append( new Action( parent, "wo.dspWoBufferStatusByPlannerCode", tr("Work Order Buffer Status by Planner Code..."),
                                          this, SLOT(sDspWoBufferStatusByPlannerCode()),
                                          displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders"))  
                                          &&  _metrics->boolean("BufferMgt") ) );
                                          
      displaysMenu->insertSeparator();
  }

  parent->actions.append( new Action( parent, "wo.dspWoHistoryByItem", tr("Work Order History by Item..."),
                                      this, SLOT(sDspWoHistoryByItem()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspWoHistoryByNumber", tr("Work Order History by W/O Number..."),
                                      this, SLOT(sDspWoHistoryByNumber()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspWoHistoryByClassCode", tr("Work Order History by Class Code..."),
                                      this, SLOT(sDspWoHistoryByClassCode()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.dspWoMaterialRequirementsByComponentItem", tr("W/O Material Requirements by Component Item..."),
                                      this, SLOT(sDspWoMaterialsByComponentItem()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspWoMaterialRequirementsByWorkOrder", tr("W/O Material Requirements by Work Order..."),
                                      this, SLOT(sDspWoMaterialsByWo()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspInventoryAvailabilityByWorkOrder", tr("Inventory Availability by Work Order..."),
                                      this, SLOT(sDspInventoryAvailabilityByWorkOrder()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

  parent->actions.append( new Action( parent, "wo.dspPendingWoMaterialAvailability", tr("Pending W/O Material Availability..."),
                                      this, SLOT(sDspPendingAvailability()),
                                      displaysMenu, _privleges->check("ViewInventoryAvailability") ) );

 if (_metrics->boolean("BufferMgt") )
 {
      displaysMenu->insertSeparator();

      parent->actions.append( new Action( parent, "wo.dspWoOperationsByWorkCenter", tr("W/O Operations by Work Center..."),
                                          this, SLOT(sDspWoOperationsByWorkCenter()),
                                          displaysMenu, (_privleges->check("MaintainWoOperations") || _privleges->check("ViewWoOperations")) && _metrics->boolean("Routings") ) );

      parent->actions.append( new Action( parent, "wo.dspWoOperationsByWorkOrder", tr("W/O Operations by Work Order..."),
                                          this, SLOT(sDspWoOperationsByWo()),
                                          displaysMenu, (_privleges->check("MaintainWoOperations") || _privleges->check("ViewWoOperations")) && _metrics->boolean("Routings") ) );

      parent->actions.append( new Action( parent, "wo.dspWoOperationBufrStsByWorkCenter", tr("W/O Operation Buffer Status by Work Center..."),
                                          this, SLOT(sDspWoOperationBufrStsByWorkCenter()),
                                          displaysMenu, (_privleges->check("MaintainWoOperations") || _privleges->check("ViewWoOperations")) 
                                          && _metrics->boolean("Routings") && _metrics->boolean("BufferMgt") ) );
  }

  if (_metrics->boolean("Routings"))
  {
      displaysMenu->insertSeparator();

      parent->actions.append( new Action( parent, "wo.dspWoEffortByWorkOrder", tr("Production Time Clock by Work Order..."),
                                          this, SLOT(sDspWoEffortByWorkOrder()),
                                          displaysMenu, (_privleges->check("MaintainWoTimeClock") || _privleges->check("ViewWoTimeClock")) && _metrics->boolean("Routings") ) );

      parent->actions.append( new Action( parent, "wo.dspWoEffortByUser", tr("Production Time Clock by User..."),
                                          this, SLOT(sDspWoEffortByUser()),
                                          displaysMenu, (_privleges->check("MaintainWoTimeClock") || _privleges->check("ViewWoTimeClock")) && _metrics->boolean("Routings") ) );
  }
  
  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.dspMaterialUsageVarianceByBOMItem", tr("Material Usage Variance by BOM Item..."),
                                      this, SLOT(sDspMaterialUsageVarianceByBOMItem()),
                                      displaysMenu, _privleges->check("ViewMaterialVariances") ) );

  parent->actions.append( new Action( parent, "wo.dspMaterialUsageVarianceByItem", tr("Material Usage Variance by Item..."),
                                      this, SLOT(sDspMaterialUsageVarianceByItem()),
                                      displaysMenu, _privleges->check("ViewMaterialVariances") ) );

  parent->actions.append( new Action( parent, "wo.dspMaterialUsageVarianceByComponentItem", tr("Material Usage Variance by Component Item..."),
                                      this, SLOT(sDspMaterialUsageVarianceByComponentItem()),
                                      displaysMenu, _privleges->check("ViewMaterialVariances") ) );

  parent->actions.append( new Action( parent, "wo.dspMaterialUsageVarianceByWorkOrder", tr("Material Usage Variance by Work Order..."),
                                      this, SLOT(sDspMaterialUsageVarianceByWorkOrder()),
                                      displaysMenu, _privleges->check("ViewMaterialVariances") ) );

  parent->actions.append( new Action( parent, "wo.dspMaterialUsageVarianceByWarehouse", tr("Material Usage Variance by Warehouse..."),
                                      this, SLOT(sDspMaterialUsageVarianceByWarehouse()),
                                      displaysMenu, _privleges->check("ViewMaterialVariances") ) );

  if (_metrics->boolean("Routings"))
  { 
      displaysMenu->insertSeparator();
      
      parent->actions.append( new Action( parent, "wo.dspLaborVarianceByBOOItem", tr("Labor Variance by BOO Item..."),
                                          this, SLOT(sDspLaborVarianceByBOOItem()),
                                          displaysMenu, _privleges->check("ViewLaborVariances") && _metrics->boolean("Routings") ) );

      parent->actions.append( new Action( parent, "wo.dspLaborVarianceByItem", tr("Labor Variance by Item..."),
                                          this, SLOT(sDspLaborVarianceByItem()),
                                          displaysMenu, _privleges->check("ViewLaborVariances") && _metrics->boolean("Routings") ) );

      parent->actions.append( new Action( parent, "wo.dspLaborVarianceByWorkCenter", tr("Labor Variance by Work Center..."),
                                          this, SLOT(sDspLaborVarianceByWorkCenter()),
                                          displaysMenu, _privleges->check("ViewLaborVariances") && _metrics->boolean("Routings") ) );

      parent->actions.append( new Action( parent, "wo.dspLaborVarianceByWorkOrder", tr("Labor Variance by Work Order..."),
                                          this, SLOT(sDspLaborVarianceByWorkOrder()),
                                          displaysMenu, _privleges->check("ViewLaborVariances") && _metrics->boolean("Routings") ) );

      displaysMenu->insertSeparator();
  }

  if (_metrics->boolean("BBOM"))
  { 
      parent->actions.append( new Action( parent, "wo.dspBreederDistributionVarianceByItem", tr("Breeder Distribution Variance by Item..."),
                                          this, SLOT(sDspBreederDistributionVarianceByItem()),
                                          displaysMenu, _privleges->check("ViewBreederVariances") && _metrics->boolean("BBOM") ) );

      parent->actions.append( new Action( parent, "wo.dspBreederDistributionVarianceByWarehouse", tr("Breeder Distribution Variance by Warehouse..."),
                                          this, SLOT(sDspBreederDistributionVarianceByWarehouse()),
                                          displaysMenu, _privleges->check("ViewBreederVariances") && _metrics->boolean("BBOM") ) );
  }
  
  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "wo.dspOpenWorkOrdersWithClosedParentSalesOrders", tr("Open Work Orders with Closed Parent Sales Orders..."),
                                      this, SLOT(sDspWoSoStatusMismatch()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "wo.dspOpenWorkOrdersWithParentSalesOrders", tr("Open Work Orders with Parent Sales Orders..."),
                                      this, SLOT(sDspWoSoStatus()),
                                      displaysMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

//  W/O | Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "wo.rptPrintWorkOrderForm", tr("Print Work Order Form..."),
                                      this, SLOT(sPrintWorkOrderForm()),
                                      reportsMenu, _privleges->check("PrintWorkOrderPaperWork") ) );

  parent->actions.append( new Action( parent, "wo.rptPrintProductionEntrySheet", tr("Print Production Entry Sheet..."),
                                      this, SLOT(sPrintProductionEntrySheet()),
                                      reportsMenu, _privleges->check("PrintWorkOrderPaperWork") ) );

  mainMenu = new QMenu();

  mainMenu->insertItem(tr("W/O &Control"),    controlMenu    );
  mainMenu->insertItem(tr("W/O &Materials"),  materialsMenu  );
  if (_metrics->boolean("Routings"))
    mainMenu->insertItem(tr("W/O &Operations"), operationsMenu );
  mainMenu->insertItem(tr("&Displays"),       displaysMenu   );
  mainMenu->insertItem(tr("&Reports"),        reportsMenu    );
  parent->populateCustomMenu(mainMenu, "W/O");
  parent->menuBar()->insertItem(tr("W/&O"), mainMenu);
}

void moduleWO::sNewWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sExplodeWorkOrder()
{
  explodeWo(parent, "", TRUE).exec();
}

void moduleWO::sImplodeWorkOrder()
{
  implodeWo(parent, "", TRUE).exec();
}

void moduleWO::sWoTimeClock()
{
  omfgThis->handleNewWindow(new woTimeClock());
}

void moduleWO::sPrintTraveler()
{
  printWoTraveler(parent, "", TRUE).exec();
}

void moduleWO::sPrintPickList()
{
  printWoPickList(parent, "", TRUE).exec();
}

void moduleWO::sPrintRouting()
{
  printWoRouting(parent, "", TRUE).exec();
}

void moduleWO::sCloseWorkOrder()
{
  closeWo(parent, "", TRUE).exec();
}

void moduleWO::sReleaseWorkOrdersByPlannerCode()
{
  releaseWorkOrdersByPlannerCode(parent, "", TRUE).exec();
}

void moduleWO::sReprioritizeWorkOrder()
{
  reprioritizeWo(parent, "", TRUE).exec();
}

void moduleWO::sRescheduleWorkOrder()
{
  rescheduleWo(parent, "", TRUE).exec();
}

void moduleWO::sChangeWorkOrderQty()
{
  changeWoQty(parent, "", TRUE).exec();
}

void moduleWO::sPurgeClosedWorkOrders()
{
  purgeClosedWorkOrders(parent, "", TRUE).exec();
}

void moduleWO::sCreateWoMaterialRequirement()
{
  ParameterList params;
  params.append("mode", "new");

  woMaterialItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleWO::sDeleteWoMaterialRequirement()
{
  deleteWoMaterialItem(parent, "", TRUE).exec();
}

void moduleWO::sMaintainWoMaterials()
{
  omfgThis->handleNewWindow(new workOrderMaterials());
}

void moduleWO::sIssueWoMaterialBatch()
{
  issueWoMaterialBatch(parent, "", TRUE).exec();
}

void moduleWO::sIssueWoMaterialItem()
{
  issueWoMaterialItem(parent, "", TRUE).exec();
}

void moduleWO::sReturnWoMaterialBatch()
{
  returnWoMaterialBatch(parent, "", TRUE).exec();
}

void moduleWO::sReturnWoMaterialItem()
{
  returnWoMaterialItem(parent, "", TRUE).exec();
}

void moduleWO::sScrapWoMaterialFromWo()
{
  scrapWoMaterialFromWIP(parent, "", TRUE).exec();
}

void moduleWO::sPostProduction()
{
  postProduction(parent, "", TRUE).exec();
}

void moduleWO::sCorrectProductionPosting()
{
  correctProductionPosting(parent, "", TRUE).exec();
}

void moduleWO::sPostMiscProduction()
{
  postMiscProduction(parent, "", TRUE).exec();
}

void moduleWO::sCreateWoOperation()
{
  ParameterList params;
  params.append("mode", "new");

  woOperation newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleWO::sMaintainWoOperations()
{
  omfgThis->handleNewWindow(new workOrderOperations());
}

void moduleWO::sPostOperations()
{
  postOperations(parent, "", TRUE).exec();
}

void moduleWO::sCorrectOperationsPosting()
{
  correctOperationsPosting(parent, "", TRUE).exec();
}

void moduleWO::sDspWoHistoryByItem()
{
  omfgThis->handleNewWindow(new dspWoHistoryByItem());
}

void moduleWO::sDspWoHistoryByNumber()
{
  omfgThis->handleNewWindow(new dspWoHistoryByNumber());
}

void moduleWO::sDspWoHistoryByClassCode()
{
  omfgThis->handleNewWindow(new dspWoHistoryByClassCode());
}

void moduleWO::sDspWoMaterialsByComponentItem()
{
  omfgThis->handleNewWindow(new dspWoMaterialsByItem());
}

void moduleWO::sDspWoMaterialsByWo()
{
  omfgThis->handleNewWindow(new dspWoMaterialsByWorkOrder());
}

void moduleWO::sDspInventoryAvailabilityByWorkOrder()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByWorkOrder());
}

void moduleWO::sDspPendingAvailability()
{
  omfgThis->handleNewWindow(new dspPendingAvailability());
}

void moduleWO::sDspWoOperationsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspWoOperationsByWorkCenter());
}

void moduleWO::sDspWoOperationsByWo()
{
  omfgThis->handleNewWindow(new dspWoOperationsByWorkOrder());
}

void moduleWO::sDspWoOperationBufrStsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspWoOperationBufrStsByWorkCenter());
}

void moduleWO::sDspWoScheduleByItem()
{
  omfgThis->handleNewWindow(new dspWoScheduleByItem());
}

void moduleWO::sDspWoScheduleByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspWoScheduleByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspWoScheduleByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspWoScheduleByWorkCenter()
{
  ParameterList params;
  params.append("wrkcnt");

  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspWoBufferStatusByItem()
{
  //omfgThis->handleNewWindow(new dspWoBufferStatusByItem());
}

void moduleWO::sDspWoBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspWoBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspWoBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleWO::sDspMaterialUsageVarianceByBOMItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByBOMItem());
}

void moduleWO::sDspMaterialUsageVarianceByItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByItem());
}

void moduleWO::sDspMaterialUsageVarianceByComponentItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByComponentItem());
}

void moduleWO::sDspMaterialUsageVarianceByWorkOrder()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByWorkOrder());
}

void moduleWO::sDspWoEffortByWorkOrder()
{
  omfgThis->handleNewWindow(new dspWoEffortByWorkOrder());
}

void moduleWO::sDspWoEffortByUser()
{
  omfgThis->handleNewWindow(new dspWoEffortByUser());
}

void moduleWO::sDspMaterialUsageVarianceByWarehouse()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByWarehouse());
}

void moduleWO::sDspLaborVarianceByBOOItem()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByBOOItem());
}

void moduleWO::sDspLaborVarianceByItem()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByItem());
}

void moduleWO::sDspLaborVarianceByWorkCenter()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByWorkCenter());
}

void moduleWO::sDspLaborVarianceByWorkOrder()
{
  omfgThis->handleNewWindow(new dspLaborVarianceByWorkOrder());
}

void moduleWO::sDspBreederDistributionVarianceByItem()
{
  omfgThis->handleNewWindow(new dspBreederDistributionVarianceByItem());
}

void moduleWO::sDspBreederDistributionVarianceByWarehouse()
{
  omfgThis->handleNewWindow(new dspBreederDistributionVarianceByWarehouse());
}

void moduleWO::sDspWoSoStatusMismatch()
{
  omfgThis->handleNewWindow(new dspWoSoStatusMismatch());
}

void moduleWO::sDspWoSoStatus()
{
  omfgThis->handleNewWindow(new dspWoSoStatus());
}

void moduleWO::sPrintWorkOrderForm()
{
  printWoForm(parent, "", TRUE).exec();
}

void moduleWO::sPrintProductionEntrySheet()
{
  printProductionEntrySheet(parent, "", TRUE).exec();
}

