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

//  moduleSR.cpp
//  Created 08/22/2002 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

#include "packingListBatch.h"
#include "issueToShipping.h"
#include "maintainShipping.h"
#include "shipOrder.h"
#include "recallOrders.h"
#include "purgeShippingRecords.h"

#include "enterPoReceipt.h"
#include "enterPoReturn.h"
#include "unpostedPoReceipts.h"

#include "printPackingList.h"
#include "printPackingListBatchByShipvia.h"
#include "printShippingForm.h"
#include "printShippingForms.h"
#include "printLabelsBySo.h"
#include "printLabelsByInvoice.h"
#include "printLabelsByPo.h"

#include "destinations.h"
#include "carriers.h"

#include "dspShipmentsBySalesOrder.h"
#include "dspBacklogByItem.h"
#include "dspBacklogByCustomer.h"
#include "dspBacklogByParameterList.h"
#include "dspSummarizedBacklogByWarehouse.h"
#include "dspShipmentsBySalesOrder.h"
#include "dspShipmentsByDate.h"

#include "rptBacklogByItem.h"
#include "rptBacklogByCustomer.h"
#include "rptBacklogByParameterList.h"
#include "rptSummarizedBacklogByWarehouse.h"
#include "rptShipmentsBySalesOrder.h"
#include "rptShipmentsByDate.h"

#include "moduleSR.h"

moduleSR::moduleSR(OpenMFGGUIClient *Pparent) :
 QObject(Pparent, "srModule")
{
  parent = Pparent;
  toolBar = new QToolBar(tr("S/R Tools"));
  toolBar->setObjectName("S/R Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowSRToolbar"))
    parent->addToolBar(toolBar);

  srMenu		= new QMenu();
  shippingMenu		= new QMenu();
  receivingMenu		= new QMenu();
  formsMenu		= new QMenu();
  //ratesDisplaysMenu	= new QMenu();
  ratesMenu		= new QMenu();
  displaysMenu		= new QMenu();
  reportsMenu		= new QMenu();

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    bool		priv;
    QPixmap*		pixmap;
    QToolBar*		toolBar;
    bool		visible;
  };

  actionProperties acts[] = {
    //  S/R | Shipping
    { "menu",	tr("Shipping"),  (char*)shippingMenu,	srMenu,	true,	NULL, NULL, true },
    { "sr.packingListBatch", tr("Packing List Batch..."), SLOT(sPackingListBatch()), shippingMenu, (_privleges->check("MaintainPackingListBatch") || _privleges->check("ViewPackingListBatch")), NULL, NULL, true},
    { "sr.printPackingListBatchByShipvia", tr("Print Packing List Batch by Ship Via..."), SLOT(sPrintPackingListBatchByShipvia()), shippingMenu, _privleges->check("PrintPackingLists"), NULL, NULL, true},

    { "separator", NULL, NULL, shippingMenu, true, NULL, NULL, true},
    { "sr.issueStockToShipping", tr("Issue Stock to Shipping..."), SLOT(sIssueStockToShipping()), shippingMenu, _privleges->check("IssueStockToShipping"), new QPixmap(":/images/issueStockToShipping.png"), toolBar,  true},
    { "sr.maintainShippingContents", tr("Maintain Shipping Contents..."), SLOT(sDspShippingContents()), shippingMenu, _privleges->check("ViewShipping"), NULL, NULL, true},
    { "separator", NULL, NULL, shippingMenu, true, NULL, NULL, true},
    { "sr.shipOrder", tr("Ship Order..."), SLOT(sShipOrders()), shippingMenu, _privleges->check("ShipOrders"), NULL, NULL, true},
    { "sr.recallOrdersToShipping", tr("Recall Orders to Shipping..."), SLOT(sRecallOrders()), shippingMenu, _privleges->check("RecallOrders"), NULL, NULL, true},
    { "separator", NULL, NULL, shippingMenu, true, NULL, NULL, true},
    { "sr.purgeShippingRecords", tr("Purge Shipping Records..."), SLOT(sPurgeShippingRecords()), shippingMenu, _privleges->check("PurgeShippingRecords"), NULL, NULL, true},

    //  S/R | Receiving
    { "menu",	tr("Receiving"),  (char*)receivingMenu,	srMenu,	true,	NULL, NULL, true },
    { "sr.enterReceipt", tr("Enter Receipt..."), SLOT(sEnterReceipt()), receivingMenu, _privleges->check("EnterReceipts"), NULL, NULL, true},
    { "sr.postReceipts", tr("List Unposted Receipts..."), SLOT(sPostReceipts()), receivingMenu, _privleges->check("EnterReceipts"), new QPixmap(":/images/postReceipts.png"), toolBar,  true},
    { "separator", NULL, NULL, receivingMenu, true, NULL, NULL, true},
    { "sr.enterReturn", tr("Enter Return..."), SLOT(sEnterReturn()), receivingMenu, _privleges->check("EnterReturns"), NULL, NULL, true},


    //  S/R | Forms
    { "menu",	tr("Forms"),  (char*)formsMenu,	srMenu,	true,	NULL, NULL, true },
    { "sr.printPackingList", tr("Print Packing List..."), SLOT(sPrintPackingLists()), formsMenu, _privleges->check("PrintPackingLists"), NULL, NULL, true},
    { "sr.printShippingForm", tr("Print Shipping Form..."), SLOT(sPrintShippingForm()), formsMenu, _privleges->check("PrintBillsOfLading"), NULL, NULL, true},
    { "sr.printShippingForms", tr("Print Shipping Forms..."), SLOT(sPrintShippingForms()), formsMenu, _privleges->check("PrintBillsOfLading"), NULL, NULL, true},
    { "sr.printShippingLabelsBySo", tr("Print Shipping Labels by S/O #..."), SLOT(sPrintShippingLabelsBySo()), formsMenu, _privleges->check("ViewShipping"), NULL, NULL, true},
    { "sr.printShippingLabelsByInvoice", tr("Print Shipping Labels by Invoice..."), SLOT(sPrintShippingLabelsByInvoice()), formsMenu, _privleges->check("ViewShipping"), NULL, NULL, true},
    { "sr.printReceivingLabelsByPo", tr("Print Receiving Labels by P/O #..."), SLOT(sPrintReceivingLabelsByPo()), formsMenu, _privleges->check("ViewShipping"), NULL, NULL, true},

    //  S/R | Rates
    { "menu",	tr("Rates"),	(char*)ratesMenu,	srMenu,	true, NULL, NULL, true},
    { "sr.destinations", tr("Destinations..."), SLOT(sDestinations()), ratesMenu, (_privleges->check("MaintainDestinations") || _privleges->check("ViewDestinations")), NULL, NULL, true},
    { "separator", NULL, NULL, ratesMenu, true, NULL, NULL, true},
    { "sr.carries", tr("Carriers..."), SLOT(sCarriers()), ratesMenu, (_privleges->check("MaintainCarriers") || _privleges->check("ViewCarriers")), NULL, NULL, true},
    
#if 0
    { "separator", NULL, NULL, ratesMenu, true, NULL, NULL, true},
    { "sr.enterNewRate", tr("Enter New Rate..."), SLOT(sAddRate()), ratesMenu, TRUE, NULL, NULL, true},
    { "separator", NULL, NULL, ratesMenu, true, NULL, NULL, true},

  //  S/R | Rates | Displays
    { "menu",	tr("Displays"),  (char*)ratesDisplaysMenu,	ratesMenu,	true,	NULL, NULL, true },
    { "sr.dspRatesByDestination", tr("Rates by Destination..."), SLOT(sDspRatesByDestination()), ratesDisplayMenu, TRUE, NULL, NULL, true},
    { "sr.dspRatesByCarrier", tr("Rates by Carrier..."), SLOT(sDspRatesByCarrier()), ratesDisplayMenu, TRUE, NULL, NULL, true},
#endif

    //  S/R | Displays
    { "menu",	tr("Displays"),	(char*)displaysMenu,	srMenu,	true, NULL, NULL, true},
    { "sr.dspBacklogByItem", tr("Backlog by Item..."), SLOT(sDspBacklogByItem()), displaysMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "sr.dspBacklogByCustomer", tr("Backlog by Customer..."), SLOT(sDspBacklogByCustomer()), displaysMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "sr.dspBacklogByProductCategory", tr("Backlog by Product Category..."), SLOT(sDspBacklogByProductCategory()), displaysMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "separator", NULL, NULL, displaysMenu, true, NULL, NULL, true},
    { "sr.dspSummarizedBacklogByWarehouse", tr("Summarized Backlog by Warehouse..."), SLOT(sDspSummarizedBacklogByWarehouse()), displaysMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "separator", NULL, NULL, displaysMenu, true, NULL, NULL, true},
    { "sr.dspShipmentsBySalesOrder", tr("Shipments by Sales Order..."), SLOT(sDspShipmentsBySalesOrder()), displaysMenu, _privleges->check("ViewShipping"), NULL, NULL, true},
    { "sr.dspShipmentsByDate", tr("Shipments by Date..."), SLOT(sDspShipmentsByDate()), displaysMenu, _privleges->check("ViewShipping"), NULL, NULL, true},

    //  S/R | Reports
    { "menu",	tr("Reports"),	(char*)reportsMenu,	srMenu,	true, NULL, NULL, true},
    { "sr.rptBacklogByItem", tr("Backlog by Item..."), SLOT(sRptBacklogByItem()), reportsMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "sr.rptBacklogByCustomer", tr("Backlog by Customer..."), SLOT(sRptBacklogByCustomer()), reportsMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "sr.rptBacklogByProductCategory", tr("Backlog by Product Category..."), SLOT(sRptBacklogByProductCategory()), reportsMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true},
    { "sr.rptSummarizedBacklogByWarehouse", tr("Summarized Backlog by Warehouse..."), SLOT(sRptSummarizedBacklogByWarehouse()), reportsMenu, _privleges->check("ViewSalesOrders"), NULL, NULL, true},
    { "separator", NULL, NULL, reportsMenu, true, NULL, NULL, true},
    { "sr.rptShipmentsBySalesOrder", tr("Shipments by Sales Order..."), SLOT(sRptShipmentsBySalesOrder()), reportsMenu, _privleges->check("ViewShipping"), NULL, NULL, true},
    { "sr.rptShipmentsByDate", tr("Shipments by Date..."), SLOT(sRptShipmentsByDate()), reportsMenu, _privleges->check("ViewShipping"), NULL, NULL, true},
  };

  for (unsigned int i = 0; i < sizeof(acts) / sizeof(acts[0]); i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == "menu")
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == "separator")
    {
      acts[i].menu->addSeparator();
    }
    else if (acts[i].toolBar != NULL)
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar) );
    }
    else
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv ) );
    }
  }
  parent->populateCustomMenu(srMenu, "S/R");
  parent->menuBar()->insertItem(tr("S/&R"), srMenu);
}

void moduleSR::sPackingListBatch()
{
  omfgThis->handleNewWindow(new packingListBatch());
}

void moduleSR::sDspShippingContents()
{
  omfgThis->handleNewWindow(new maintainShipping());
}

void moduleSR::sIssueStockToShipping()
{
  omfgThis->handleNewWindow(new issueToShipping());
}

void moduleSR::sPrintPackingLists()
{
  printPackingList(parent, "", TRUE).exec();
}

void moduleSR::sPrintPackingListBatchByShipvia()
{
  printPackingListBatchByShipvia(parent, "", TRUE).exec();
}

void moduleSR::sPrintShippingForm()
{
  printShippingForm(parent, "", TRUE).exec();
}

void moduleSR::sPrintShippingForms()
{
  printShippingForms(parent, "", TRUE).exec();
}

void moduleSR::sPrintShippingLabelsBySo()
{
  printLabelsBySo(parent, "", TRUE).exec();
}

void moduleSR::sPrintShippingLabelsByInvoice()
{
  printLabelsByInvoice(parent, "", TRUE).exec();
}

void moduleSR::sPrintReceivingLabelsByPo()
{
  printLabelsByPo(parent, "", TRUE).exec();
}


void moduleSR::sShipOrders()
{
  shipOrder(parent, "", TRUE).exec();
}

void moduleSR::sRecallOrders()
{
  omfgThis->handleNewWindow(new recallOrders());
}

void moduleSR::sPurgeShippingRecords()
{
  purgeShippingRecords(parent, "", TRUE).exec();
}

void moduleSR::sEnterReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSR::sEnterReturn()
{
  ParameterList params;
  params.append("mode", "new");

  enterPoReturn *newdlg = new enterPoReturn();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSR::sPostReceipts()
{
  //ParameterList params;

  unpostedPoReceipts *newdlg = new unpostedPoReceipts();
  //newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSR::sDestinations()
{
  omfgThis->handleNewWindow(new destinations());
}

void moduleSR::sCarriers()
{
  omfgThis->handleNewWindow(new carriers());
}

void moduleSR::sAddRate()
{
}

void moduleSR::sDspRatesByDestination()
{
}

void moduleSR::sDspRatesByCarrier()
{
}

void moduleSR::sDspBacklogByItem()
{
  omfgThis->handleNewWindow(new dspBacklogByItem());
}

void moduleSR::sDspBacklogByCustomer()
{
  omfgThis->handleNewWindow(new dspBacklogByCustomer());
}

void moduleSR::sDspBacklogByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSR::sDspSummarizedBacklogByWarehouse()
{
  omfgThis->handleNewWindow(new dspSummarizedBacklogByWarehouse());
}

void moduleSR::sDspShipmentsBySalesOrder()
{
  omfgThis->handleNewWindow(new dspShipmentsBySalesOrder());
}

void moduleSR::sDspShipmentsByDate()
{
  omfgThis->handleNewWindow(new dspShipmentsByDate());
}


void moduleSR::sRptBacklogByItem()
{
  rptBacklogByItem(parent, "", TRUE).exec();
}

void moduleSR::sRptBacklogByCustomer()
{
  rptBacklogByCustomer(parent, "", TRUE).exec();
}

void moduleSR::sRptBacklogByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  rptBacklogByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleSR::sRptSummarizedBacklogByWarehouse()
{
  rptSummarizedBacklogByWarehouse(parent, "", TRUE).exec();
}

void moduleSR::sRptShipmentsBySalesOrder()
{
  rptShipmentsBySalesOrder(parent, "", TRUE).exec();
}

void moduleSR::sRptShipmentsByDate()
{
  rptShipmentsByDate(parent, "", TRUE).exec();
}

