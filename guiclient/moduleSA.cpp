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

//  moduleSA.cpp
//  Created 01/01/2001 JSL
//  Copyright (c) 2001-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include "OpenMFGGUIClient.h"

#include "dspSalesHistoryByCustomer.h"
#include "dspSalesHistoryByBilltoName.h"
#include "dspSalesHistoryByShipTo.h"
#include "dspSalesHistoryByParameterList.h"
#include "dspSalesHistoryByItem.h"
#include "dspSalesHistoryBySalesrep.h"
#include "dspBriefSalesHistoryByCustomer.h"
#include "dspBriefSalesHistoryByCustomerType.h"
#include "dspBriefSalesHistoryBySalesRep.h"
#include "dspBookingsByCustomer.h"
#include "dspBookingsByCustomerGroup.h"
#include "dspBookingsByShipTo.h"
#include "dspBookingsByProductCategory.h"
#include "dspBookingsByItem.h"
#include "dspBookingsBySalesRep.h"
#include "dspSummarizedSalesByCustomer.h"
#include "dspSummarizedSalesByCustomerType.h"
#include "dspSummarizedSalesByCustomerByItem.h"
#include "dspSummarizedSalesByCustomerTypeByItem.h"
#include "dspSummarizedSalesByItem.h"
#include "dspSummarizedSalesBySalesRep.h"
#include "dspSummarizedSalesHistoryByShippingZone.h"
#include "dspTimePhasedBookingsByItem.h"
#include "dspTimePhasedBookingsByProductCategory.h"
#include "dspTimePhasedBookingsByCustomer.h"
#include "dspTimePhasedSalesByItem.h"
#include "dspTimePhasedSalesByProductCategory.h"
#include "dspTimePhasedSalesByCustomer.h"
#include "dspTimePhasedSalesByCustomerGroup.h"
#include "dspTimePhasedSalesByCustomerByItem.h"

#include "printSASpecialCalendarForm.h"

#include "archRestoreSalesHistory.h"

#include "moduleSA.h"

moduleSA::moduleSA(OpenMFGGUIClient *pParent) :
  QObject(pParent, "")
{
  parent = pParent;

//  Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByCustomer", tr("Sales History by Customer..."),
                                      this, SLOT(sDspSalesHistoryByCustomer()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByBillToName", tr("Sales History by Bill-To Name..."),
                                      this, SLOT(sDspSalesHistoryByBilltoName()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByShipTo", tr("Sales History by Ship-To..."),
                                      this, SLOT(sDspSalesHistoryByShipTo()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByItem", tr("Sales History by Item..."),
                                      this, SLOT(sDspSalesHistoryByItem()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryBySalesRep", tr("Sales History by Sales Rep..."),
                                      this, SLOT(sDspSalesHistoryBySalesRep()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByProductCategory", tr("Sales History by Product Category..."),
                                      this, SLOT(sDspSalesHistoryByProductCategory()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByCustomerType", tr("Sales History by Customer Type..."),
                                      this, SLOT(sDspSalesHistoryByCustomerType()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspSalesHistoryByCustomerGroup", tr("Sales History by Customer Group..."),
                                      this, SLOT(sDspSalesHistoryByCustomerGroup()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "sa.dspBriefSalesHistoryByCustomer", tr("Brief Sales History by Customer..."),
                                      this, SLOT(sDspBriefSalesHistoryByCustomer()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspBriefSalesHistoryByCustomerType", tr("Brief Sales History by Customer Type..."),
                                      this, SLOT(sDspBriefSalesHistoryByCustomerType()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.dspBriefSalesHistoryBySalesRep", tr("Brief Sales History by Sales Rep..."),
                                      this, SLOT(sDspBriefSalesHistoryBySalesRep()),
                                      displaysMenu, _privleges->check("ViewSalesHistory") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "sa.dspBookingsByCustomer", tr("Bookings by Customer..."),
                                      this, SLOT(sDspBookingsByCustomer()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspBookingsByCustomerGroup", tr("Bookings by Customer Group..."),
                                      this, SLOT(sDspBookingsByCustomerGroup()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspBookingsByShipTo", tr("Bookings by Ship-To..."),
                                      this, SLOT(sDspBookingsByShipTo()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspBookingsByItem", tr("Bookings by Item..."),
                                      this, SLOT(sDspBookingsByItem()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspBookingsByProductCategory", tr("Bookings by Product Category..."),
                                      this, SLOT(sDspBookingsByProductCategory()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspBookingsBySalesRep", tr("Bookings by Sales Rep..."),
                                      this, SLOT(sDspBookingsBySalesRep()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryByCustomer", tr("Summarized Sales History by Customer..."),
                                      this, SLOT(sDspSummarizedSalesByCustomer()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryByCustomerType", tr("Summarized Sales History by Customer Type..."),
                                      this, SLOT(sDspSummarizedSalesByCustomerType()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryByCustomerByItem", tr("Summarized Sales History by Customer by Item..."),
                                      this, SLOT(sDspSummarizedSalesByCustomerByItem()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryByCustomerTypeByItem", tr("Summarized Sales History by Customer Type by Item..."),
                                      this, SLOT(sDspSummarizedSalesByCustomerTypeByItem()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryByItem", tr("Summarized Sales History by Item..."),
                                      this, SLOT(sDspSummarizedSalesByItem()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryBySalesRep", tr("Summarized Sales History by Sales Rep..."),
                                      this, SLOT(sDspSummarizedSalesBySalesRep()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspSummarizedSalesHistoryByShippingZoneByItem", tr("Summarized Sales History by Shipping Zone by Item..."),
                                      this, SLOT(sDspSummarizedSalesHistoryByShippingZone()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "sa.dspTimePhasedBookingsByItem", tr("Time-Phased Bookings by Item..."),
                                      this, SLOT(sDspTimePhasedBookingsByItem()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedBookingsByProductCategory", tr("Time-Phased Bookings by Product Category..."),
                                      this, SLOT(sDspTimePhasedBookingsByProductCategory()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedBookingsByCustomer", tr("Time-Phased Bookings by Customer..."),
                                      this, SLOT(sDspTimePhasedBookingsByCustomer()),
                                      displaysMenu, _privleges->check("ViewSalesOrders") ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedSalesHistoryByItem", tr("Time-Phased Sales History by Item..."),
                                      this, SLOT(sDspTimePhasedSalesByItem()),
                                      displaysMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")) ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedSalesHistoryByProductCategory", tr("Time-Phased Sales History by Product Category..."),
                                      this, SLOT(sDspTimePhasedSalesByProductCategory()),
                                      displaysMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")) ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedSalesHistoryByCustomer", tr("Time-Phased Sales History by Customer..."),
                                      this, SLOT(sDspTimePhasedSalesByCustomer()),
                                      displaysMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")) ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedSalesHistoryByCustomerGroup", tr("Time-Phased Sales History by Customer Group..."),
                                      this, SLOT(sDspTimePhasedSalesByCustomerGroup()),
                                      displaysMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")) ) );

  parent->actions.append( new Action( parent, "sa.dspTimePhasedSalesHistoryByCustomerByItem", tr("Time-Phased Sales History by Customer by Item..."),
                                      this, SLOT(sDspTimePhasedSalesByCustomerByItem()),
                                      displaysMenu, (_privleges->check("ViewSalesHistory") && _privleges->check("ViewCustomerPrices")) ) );

//  Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "sa.rptPrintSASpecialCalendarForm", tr("Print S/A Special Calendar Form..."),
                                      this, SLOT(sPrintSASpecialCalendarForm()),
                                      reportsMenu, _privleges->check("ViewSalesHistory") ) );

//  Utilities
  utilitiesMenu = new QMenu();

  parent->actions.append( new Action( parent, "sa.archieveSalesHistory", tr("Archive Sales History..."),
                                      this, SLOT(sArchiveSalesHistory()),
                                      utilitiesMenu, _privleges->check("ArchiveSalesHistory") ) );

  parent->actions.append( new Action( parent, "sa.restoreSalesHistory", tr("Restore Sales History..."),
                                      this, SLOT(sRestoreSalesHistory()),
                                      utilitiesMenu, _privleges->check("RestoreSalesHistory") ) );


  mainMenu = new QMenu();
  mainMenu->insertItem(tr("Displays"), displaysMenu);
  mainMenu->insertItem(tr("Reports"), reportsMenu);
  mainMenu->insertItem(tr("Utilities"), utilitiesMenu);
  parent->populateCustomMenu(mainMenu, "S/A");
  parent->menuBar()->insertItem(tr("&S/A"), mainMenu);
}

void moduleSA::sDspSalesHistoryByCustomer()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByCustomer());
}

void moduleSA::sDspSalesHistoryByBilltoName()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByBilltoName());
}

void moduleSA::sDspSalesHistoryByShipTo()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByShipTo());
}

void moduleSA::sDspSalesHistoryByItem()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByItem());
}

void moduleSA::sDspSalesHistoryBySalesRep()
{
  omfgThis->handleNewWindow(new dspSalesHistoryBySalesrep());
}

void moduleSA::sDspSalesHistoryByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSA::sDspSalesHistoryByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSA::sDspSalesHistoryByCustomerGroup()
{
  ParameterList params;
  params.append("custgrp");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSA::sDspBriefSalesHistoryByCustomer()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryByCustomer());
}

void moduleSA::sDspBriefSalesHistoryByCustomerType()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryByCustomerType());
}

void moduleSA::sDspBriefSalesHistoryBySalesRep()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryBySalesRep());
}

void moduleSA::sDspBookingsByCustomer()
{
  omfgThis->handleNewWindow(new dspBookingsByCustomer());
}

void moduleSA::sDspBookingsByCustomerGroup()
{
  omfgThis->handleNewWindow(new dspBookingsByCustomerGroup());
}

void moduleSA::sDspBookingsByShipTo()
{
  omfgThis->handleNewWindow(new dspBookingsByShipTo());
}

void moduleSA::sDspBookingsByItem()
{
  omfgThis->handleNewWindow(new dspBookingsByItem());
}

void moduleSA::sDspBookingsByProductCategory()
{
  omfgThis->handleNewWindow(new dspBookingsByProductCategory());
}

void moduleSA::sDspBookingsBySalesRep()
{
  omfgThis->handleNewWindow(new dspBookingsBySalesRep());
}

void moduleSA::sDspSummarizedSalesByCustomer()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomer());
}

void moduleSA::sDspSummarizedSalesByCustomerType()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerType());
}

void moduleSA::sDspSummarizedSalesByCustomerByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerByItem());
}

void moduleSA::sDspSummarizedSalesByCustomerTypeByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerTypeByItem());
}

void moduleSA::sDspSummarizedSalesByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByItem());
}

void moduleSA::sDspSummarizedSalesBySalesRep()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesBySalesRep());
}

void moduleSA::sDspSummarizedSalesHistoryByShippingZone()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesHistoryByShippingZone());
}

void moduleSA::sDspTimePhasedBookingsByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByItem());
}

void moduleSA::sDspTimePhasedBookingsByProductCategory()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByProductCategory());
}

void moduleSA::sDspTimePhasedBookingsByCustomer()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByCustomer());
}

void moduleSA::sDspTimePhasedSalesByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByItem());
}

void moduleSA::sDspTimePhasedSalesByProductCategory()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByProductCategory());
}

void moduleSA::sDspTimePhasedSalesByCustomer()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomer());
}

void moduleSA::sDspTimePhasedSalesByCustomerGroup()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomerGroup());
}

void moduleSA::sDspTimePhasedSalesByCustomerByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomerByItem());
}


//  Reports

void moduleSA::sPrintSASpecialCalendarForm()
{
  printSASpecialCalendarForm(parent, "", TRUE).exec();
}

void moduleSA::sArchiveSalesHistory()
{
  ParameterList params;
  params.append("archieve");

  archRestoreSalesHistory newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleSA::sRestoreSalesHistory()
{
  ParameterList params;
  params.append("restore");

  archRestoreSalesHistory newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

