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

//  moduleCP.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QMenu>

#include "OpenMFGGUIClient.h"

#include "createBufferStatusByItem.h"
#include "createBufferStatusByPlannerCode.h"
#include "dspInventoryBufferStatusByParameterList.h"
#include "dspWoBufferStatusByParameterList.h"
#include "dspWoOperationBufrStsByWorkCenter.h"
#include "dspPoItemsByBufferStatus.h"

#include "dspTimePhasedCapacityByWorkCenter.h"
#include "dspTimePhasedLoadByWorkCenter.h"
#include "dspTimePhasedAvailableCapacityByWorkCenter.h"
#include "dspTimePhasedDemandByPlannerCode.h"
#include "dspTimePhasedProductionByItem.h"
#include "dspTimePhasedProductionByPlannerCode.h"
#include "dspCapacityBufferStatusByWorkCenter.h"

#include "rptTimePhasedCapacityByWorkCenter.h"
#include "rptTimePhasedLoadByWorkCenter.h"
#include "rptTimePhasedAvailableCapacityByWorkCenter.h"
#include "rptCapacityBufferStatusByWorkCenter.h"

#include "workCenters.h"

#include "moduleCP.h"

moduleCP::moduleCP(OpenMFGGUIClient *Pparent) :
  QObject(Pparent, "cpModule")
{
  parent = Pparent;

//  Buffer Management
  bufferManagementMenu = new QMenu();

  parent->actions.append( new Action( parent, "cp.runBufferStatusByItem", tr("Run Buffer Status by Item..."),
                                      this, SLOT(sCreateBufferStatusByItem()),
                                      bufferManagementMenu, _privleges->check("CreateBufferStatus") ) );

  parent->actions.append( new Action( parent, "cp.runBufferStatusByPlannerCode", tr("Run Buffer Status by Planner Code..."),
                                      this, SLOT(sCreateBufferStatusByPlannerCode()),
                                      bufferManagementMenu, _privleges->check("CreateBufferStatus") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.dspInventoryBufferStatusByItemGroup", tr("Inventory Buffer Status by Item Group..."),
                                      this, SLOT(sDspInventoryBufferStatusByItemGroup()),
                                      bufferManagementMenu, _privleges->check("ViewInventoryBufferStatus") ) );

  parent->actions.append( new Action( parent, "cp.dspInventoryBufferStatusByClassCode", tr("Inventory Buffer Status by Class Code..."),
                                      this, SLOT(sDspInventoryBufferStatusByClassCode()),
                                      bufferManagementMenu, _privleges->check("ViewInventoryBufferStatus") ) );

  parent->actions.append( new Action( parent, "cp.dspInventoryBufferStatusByPlannerCode", tr("Inventory Buffer Status by Planner Code..."),
                                      this, SLOT(sDspInventoryBufferStatusByPlannerCode()),
                                      bufferManagementMenu, _privleges->check("ViewInventoryBufferStatus") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.dspCapacityBufferStatusByWorkCenter", tr("Capacity Buffer Status by Work Center..."),
                                      this, SLOT(sDspCapacityBufferStatusByWorkCenter()),
                                      bufferManagementMenu, _privleges->check("ViewWorkCenterBufferStatus") && _metrics->boolean("Routings") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.dspWoBufferStatusByItemGroup", tr("Work Order Buffer Status by Item Group..."),
                                      this, SLOT(sDspWoBufferStatusByItemGroup()),
                                      bufferManagementMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "cp.dspWoBufferStatusByClassCode", tr("Work Order Buffer Status by Class Code..."),
                                      this, SLOT(sDspWoBufferStatusByClassCode()),
                                      bufferManagementMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "cp.dspWoBufferStatusByPlannerCode", tr("Work Order Buffer Status by Planner Code..."),
                                      this, SLOT(sDspWoBufferStatusByPlannerCode()),
                                      bufferManagementMenu, (_privleges->check("MaintainWorkOrders") || _privleges->check("ViewWorkOrders")) ) );

  parent->actions.append( new Action( parent, "cp.dspWoOperationBufrStsByWorkCenter", tr("W/O Operation Buffer Status by Work Center..."),
                                      this, SLOT(sDspWoOperationBufrStsByWorkCenter()),
                                      bufferManagementMenu, (_privleges->check("MaintainWoOperations") || _privleges->check("ViewWoOperations")) && _metrics->boolean("Routings") ) );

  bufferManagementMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.dspPoLineItemsByBufferStatus", tr("P/O Items by Buffer Status..."),
                                      this, SLOT(sDspPoItemsByBufferStatus()),
                                      bufferManagementMenu, _privleges->check("ViewPurchaseOrders") ) );


//  Displays Menu
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "cp.dspTimePhasedCapacityByWorkCenter", tr("Time-Phased Capacity by Work Center..."),
                                      this, SLOT(sDspTimePhasedCapacityByWorkCenter()),
                                      displaysMenu, _privleges->check("ViewWorkCenterCapacity") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "cp.dspTimePhasedAvailableCapacityByWorkCenter", tr("Time-Phased Available Capacity by Work Center..."),
                                      this, SLOT(sDspTimePhasedAvailableCapacityByWorkCenter()),
                                      displaysMenu, _privleges->check("ViewWorkCenterCapacity") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "cp.dspTimePhasedLoadByWorkCenter", tr("Time-Phased Load by Work Center..."),
                                      this, SLOT(sDspTimePhasedLoadByWorkCenter()),
                                      displaysMenu, _privleges->check("ViewWorkCenterLoad") && _metrics->boolean("Routings") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.dspTimePhasedDemandByPlannerCode", tr("Time-Phased Demand by Planner Code..."),
                                      this, SLOT(sDspTimePhasedDemandByPlannerCode()),
                                      displaysMenu, _privleges->check("ViewProductionDemand") ) );

  parent->actions.append( new Action( parent, "cp.dspTimePhasedProductionByItem", tr("Time-Phased Production by Item..."),
                                      this, SLOT(sDspTimePhasedProductionByItem()),
                                      displaysMenu, _privleges->check("ViewProduction") ) );

  parent->actions.append( new Action( parent, "cp.dspTimePhasedProductionByPlannerCode", tr("Time-Phased Production by Planner Code..."),
                                      this, SLOT(sDspTimePhasedProductionByPlannerCode()),
                                      displaysMenu, _privleges->check("ViewProduction") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.dspCapacityBufferStatusByWorkCenter", tr("Capacity Buffer Status by Work Center..."),
                                      this, SLOT(sDspCapacityBufferStatusByWorkCenter()),
                                      displaysMenu, _privleges->check("ViewWorkCenterBufferStatus") && _metrics->boolean("Routings") ) );


//  Reports Menu
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "cp.rptTimePhasedCapacityByWorkCenter", tr("Time-Phased Capacity by Work Center..."),
                                      this, SLOT(sRptTimePhasedCapacityByWorkCenter()),
                                      reportsMenu, _privleges->check("ViewWorkCenterCapacity") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "cp.rptTimePhasedAvailableCapacityByWorkCenter", tr("Time-Phased Available Capacity by Work Center..."),
                                      this, SLOT(sRptTimePhasedAvailableCapacityByWorkCenter()),
                                      reportsMenu, _privleges->check("ViewWorkCenterCapacity") && _metrics->boolean("Routings") ) );

  parent->actions.append( new Action( parent, "cp.rptTimePhasedLoadByWorkCenter", tr("Time-Phased Load by Work Center..."),
                                      this, SLOT(sRptTimePhasedLoadByWorkCenter()),
                                      reportsMenu, _privleges->check("ViewWorkCenterLoad") && _metrics->boolean("Routings") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "cp.rptCapacityBufferStatusByWorkCenter", tr("Capacity Buffer Status by Work Center..."),
                                      this, SLOT(sRptCapacityBufferStatusByWorkCenter()),
                                      reportsMenu, _privleges->check("ViewWorkCenterBufferStatus") && _metrics->boolean("Routings") ) );

//  Master Information
  masterInfoMenu = new QMenu();

  parent->actions.append( new Action( parent, "cp.workCenters", tr("Work Centers..."),
                                      this, SLOT(sWorkCenters()),
                                      masterInfoMenu, (_privleges->check("MaintainWorkCenters") && _privleges->check("ViewWorkCenters") && _metrics->boolean("Routings")) ) );

  mainMenu = new QMenu();

  if (_metrics->boolean("BufferMgt"))
    mainMenu->insertItem(tr("Buffer Management"), bufferManagementMenu);
  mainMenu->insertItem(tr("&Displays"), displaysMenu);
  mainMenu->insertItem(tr("&Reports"), reportsMenu);
  mainMenu->insertItem(tr("&Master Information"), masterInfoMenu);
  parent->populateCustomMenu(mainMenu, "C/P");
  parent->menuBar()->insertItem(tr("&C/P"), mainMenu);
}

void moduleCP::sDspTimePhasedCapacityByWorkCenter()
{
  omfgThis->handleNewWindow(new dspTimePhasedCapacityByWorkCenter());
}

void moduleCP::sDspTimePhasedAvailableCapacityByWorkCenter()
{
  omfgThis->handleNewWindow(new dspTimePhasedAvailableCapacityByWorkCenter());
}

void moduleCP::sDspTimePhasedLoadByWorkCenter()
{
  omfgThis->handleNewWindow(new dspTimePhasedLoadByWorkCenter());
}

void moduleCP::sDspTimePhasedProductionByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedProductionByItem());
}

void moduleCP::sDspTimePhasedDemandByPlannerCode()
{
  omfgThis->handleNewWindow(new dspTimePhasedDemandByPlannerCode());
}

void moduleCP::sDspTimePhasedProductionByPlannerCode()
{
  omfgThis->handleNewWindow(new dspTimePhasedProductionByPlannerCode());
}

void moduleCP::sCreateBufferStatusByItem()
{
  createBufferStatusByItem(parent, "", TRUE).exec();
}

void moduleCP::sCreateBufferStatusByPlannerCode()
{
  createBufferStatusByPlannerCode(parent, "", TRUE).exec();
}

void moduleCP::sDspInventoryBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCP::sDspInventoryBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCP::sDspInventoryBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspInventoryBufferStatusByParameterList *newdlg = new dspInventoryBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCP::sDspCapacityBufferStatusByWorkCenter()
{
  omfgThis->handleNewWindow(new dspCapacityBufferStatusByWorkCenter());
}

void moduleCP::sDspWoBufferStatusByItemGroup()
{
  ParameterList params;
  params.append("itemgrp");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCP::sDspWoBufferStatusByClassCode()
{
  ParameterList params;
  params.append("classcode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCP::sDspWoBufferStatusByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspWoBufferStatusByParameterList *newdlg = new dspWoBufferStatusByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleCP::sDspWoOperationBufrStsByWorkCenter()
{
  omfgThis->handleNewWindow(new dspWoOperationBufrStsByWorkCenter());
}

void moduleCP::sDspPoItemsByBufferStatus()
{
  omfgThis->handleNewWindow(new dspPoItemsByBufferStatus());
}

void moduleCP::sRptTimePhasedCapacityByWorkCenter()
{
  rptTimePhasedCapacityByWorkCenter(parent, "", TRUE).exec();
}

void moduleCP::sRptTimePhasedAvailableCapacityByWorkCenter()
{
  rptTimePhasedAvailableCapacityByWorkCenter(parent, "", TRUE).exec();
}

void moduleCP::sRptTimePhasedLoadByWorkCenter()
{
  rptTimePhasedLoadByWorkCenter(parent, "", TRUE).exec();
}

void moduleCP::sRptCapacityBufferStatusByWorkCenter()
{
  rptCapacityBufferStatusByWorkCenter(parent, "", TRUE).exec();
}

void moduleCP::sWorkCenters()
{
  omfgThis->handleNewWindow(new workCenters());
}

