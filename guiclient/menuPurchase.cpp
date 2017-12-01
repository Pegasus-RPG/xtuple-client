/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"
#include "inputManager.h"

#include "purchaseOrder.h"
#include "unpostedPurchaseOrders.h"
#include "printPurchaseOrder.h"
#include "printPurchaseOrdersByAgent.h"
#include "postPurchaseOrder.h"
#include "postPurchaseOrdersByAgent.h"
#include "closePurchaseOrder.h"
#include "reschedulePoitem.h"
#include "changePoitemQty.h"
#include "addPoComment.h"

#include "dspUninvoicedReceivings.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "openVouchers.h"
#include "postVouchers.h"

#include "contract.h"
#include "contracts.h"
#include "itemSource.h"
#include "itemSites.h"

#include "dspItemSources.h"
#include "dspItemSourcePrices.h"
#include "dspPurchaseReqsByItem.h"
#include "dspPurchaseReqsByPlannerCode.h"
#include "dspPoItemsByVendor.h"
#include "dspPoItemsByItem.h"
#include "dspPoItemsByDate.h"
#include "dspPoHistory.h"
#include "buyCard.h"
#include "dspPoItemReceivingsByVendor.h"
#include "dspPoItemReceivingsByItem.h"
#include "dspPoItemReceivingsByDate.h"
#include "dspPoPriceVariancesByVendor.h"
#include "dspPoPriceVariancesByItem.h"
#include "dspPoDeliveryDateVariancesByItem.h"
#include "dspPoDeliveryDateVariancesByVendor.h"
#include "dspPoReturnsByVendor.h"
#include "dspPOsByDate.h"
#include "dspPOsByVendor.h"

#include "printPoForm.h"
#include "printVendorForm.h"

#include "vendor.h"
#include "vendors.h"

#include "dspItemsWithoutItemSources.h"
#include "assignItemToPlannerCode.h"
#include "assignClassCodeToPlannerCode.h"

#include "setup.h"

#include "menuPurchase.h"

menuPurchase::menuPurchase(GUIClient *Pparent) :
  QObject(Pparent)
{
  setObjectName("poModule");
  parent = Pparent;

  toolBar = new QToolBar(tr("Purchase Tools"));
  toolBar->setObjectName("Purchase Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);
    
  mainMenu = new QMenu(parent);
  requestMenu = new QMenu(parent);
  ordersMenu = new QMenu(parent);
  vouchersMenu = new QMenu(parent);
  contractsMenu = new QMenu(parent);
  itemSourcesMenu = new QMenu(parent);
  vendorMenu = new QMenu(parent);
  lookupMenu = new QMenu(parent);
  lookupPoMenu = new QMenu(parent);
  lookupPoItemsMenu = new QMenu(parent);
  formsMenu = new QMenu(parent);
  reportsMenu = new QMenu(parent);
  reportsRcptRtrnMenu = new QMenu(parent);
  reportsPriceVarMenu = new QMenu(parent);
  reportsDelvVarMenu = new QMenu(parent);
  utilitiesMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.purch");
  requestMenu->setObjectName("menu.purch.request");
  ordersMenu->setObjectName("menu.purch.orders");
  vouchersMenu->setObjectName("menu.purch.vouchers");
  contractsMenu->setObjectName("menu.purch.contracts");
  itemSourcesMenu->setObjectName("menu.purch.itemsources");
  vendorMenu->setObjectName("menu.purch.vendor");
  lookupMenu->setObjectName("menu.purch.lookup");
  lookupPoMenu->setObjectName("menu.purch.lookuppo");
  lookupPoItemsMenu->setObjectName("menu.purch.lookuppoitems");
  formsMenu->setObjectName("menu.purch.forms");
  reportsMenu->setObjectName("menu.purch.reports");
  reportsRcptRtrnMenu->setObjectName("menu.purch.reportsrcptrtrn");
  reportsPriceVarMenu->setObjectName("menu.purch.reportspricevar");
  reportsDelvVarMenu->setObjectName("menu.purch.reportsdelvvar");
  utilitiesMenu->setObjectName("menu.purch.utilities");

  actionProperties acts[] = {
    //  Purchase | Requisitions
    { "menu", tr("Purchase &Requests"), (char*)requestMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.dspPurchaseRequestsByPlannerCode", tr("by &Planner Code..."), SLOT(sDspPurchaseReqsByPlannerCode()), requestMenu, "ViewPurchaseRequests", new QPixmap(":/images/dspPurchaseReqByPlannerCode.png"), toolBar, true , tr("Purchase Requests by Planner Code") },
    { "po.dspPurchaseRequestsByItem", tr("by &Item..."), SLOT(sDspPurchaseReqsByItem()), requestMenu, "ViewPurchaseRequests", NULL, NULL, true , NULL },

    //  Purchase | Purchase Order
    { "menu", tr("&Purchase Order"), (char*)ordersMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.newPurchaseOrder", tr("&New..."), SLOT(sNewPurchaseOrder()), ordersMenu, "MaintainPurchaseOrders", NULL, NULL, true , NULL },
    { "po.listUnpostedPurchaseOrders", tr("&List Open..."), SLOT(sPurchaseOrderEditList()), ordersMenu, "MaintainPurchaseOrders ViewPurchaseOrders", new QPixmap(":/images/listUnpostedPo.png"), toolBar, true , tr("List Open Purchase Orders") },
    { "separator", NULL, NULL, ordersMenu, "true", NULL, NULL, true , NULL },
    { "po.postPurchaseOrder", tr("&Release..."), SLOT(sPostPurchaseOrder()), ordersMenu, "ReleasePurchaseOrders", NULL, NULL, true , NULL },
    { "po.postPurchaseOrdersByAgent", tr("Release by A&gent..."), SLOT(sPostPurchaseOrdersByAgent()), ordersMenu, "ReleasePurchaseOrders", NULL, NULL, true , NULL },
    { "po.closePurchaseOrder", tr("&Close..."), SLOT(sClosePurchaseOrder()), ordersMenu, "MaintainPurchaseOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, ordersMenu, "true", NULL, NULL, true , NULL },
    { "po.reschedulePurchaseOrderItem", tr("&Reschedule..."), SLOT(sReschedulePoitem()), ordersMenu, "ReschedulePurchaseOrders", NULL, NULL, true , NULL },
    { "wo.changePurchaseOrderItemQty", tr("Change &Qty..."), SLOT(sChangePoitemQty()), ordersMenu, "ChangePurchaseOrderQty", NULL, NULL, true , NULL },
    { "wo.addCommentToPurchaseOrder", tr("&Add Comment..."), SLOT(sAddPoComment()), ordersMenu, "MaintainPurchaseOrders", NULL, NULL, true , NULL },

    //  Purchasing | Voucher
    { "menu", tr("&Voucher"), (char*)vouchersMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.enterNewVoucher", tr("&New..."), SLOT(sEnterVoucher()), vouchersMenu, "MaintainVouchers", NULL, NULL, true , NULL },
    { "po.enterNewMiscVoucher", tr("New &Miscellaneous..."), SLOT(sEnterMiscVoucher()), vouchersMenu, "MaintainVouchers", NULL, NULL, true , NULL },
    { "po.listUnpostedVouchers", tr("&List Unposted..."), SLOT(sUnpostedVouchers()), vouchersMenu, "MaintainVouchers ViewVouchers", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, vouchersMenu, "true", NULL, NULL, true , NULL },
    { "po.postVouchers", tr("&Post..."), SLOT(sPostVouchers()), vouchersMenu, "PostVouchers", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, mainMenu, "true", NULL, NULL, true , NULL },

    // Purchasing | Lookup
    { "menu",	tr("&Lookup"),           (char*)lookupMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    
    //  Purchasing | Lookup | P/Os
    { "menu", tr("&Purchase Orders"), (char*)lookupPoMenu, lookupMenu, "true", NULL, NULL, true , NULL },
    { "po.dspPOsByVendor", tr("by &Vendor..."), SLOT(sDspPOsByVendor()), lookupPoMenu, "ViewPurchaseOrders", NULL, NULL, true , NULL },
    { "po.dspPOsByDate", tr("by &Date..."), SLOT(sDspPOsByDate()), lookupPoMenu, "ViewPurchaseOrders", NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | P/O Items
    { "menu", tr("Purchase &Order Items"), (char*)lookupPoItemsMenu, lookupMenu, "true", NULL, NULL, true , NULL },
    { "po.dspPoLineItemsByVendor", tr("by &Vendor..."), SLOT(sDspPoItemsByVendor()), lookupPoItemsMenu, "ViewPurchaseOrders", NULL, NULL, true , NULL },
    { "po.dspPoLineItemsByDate", tr("by &Date..."), SLOT(sDspPoItemsByDate()), lookupPoItemsMenu, "ViewPurchaseOrders", NULL, NULL, true , NULL },
    { "po.dspPoLineItemsByItem", tr("by &Item..."), SLOT(sDspPoItemsByItem()), lookupPoItemsMenu, "ViewPurchaseOrders", NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, lookupMenu, "true", NULL, NULL, true , NULL },
    
    { "po.dspPoHistory", tr("Purchase Order &History..."), SLOT(sDspPoHistory()), lookupMenu, "ViewPurchaseOrders", NULL, NULL, true , NULL },
    
    // Purchasing | Forms
    { "menu", tr("&Forms"), (char*)formsMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.printPurchaseOrder", tr("Print Purchase &Order..."), SLOT(sPrintPurchaseOrder()), formsMenu, "PrintPurchaseOrders", NULL, NULL, true , NULL },
    { "po.printPurchaseOrdersByAgent", tr("Print Purchase Orders by &Agent..."), SLOT(sPrintPurchaseOrdersByAgent()), formsMenu, "PrintPurchaseOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, formsMenu, "true", NULL, NULL, true , NULL },
    { "po.printPoForm", tr("Print &P/O Form..."), SLOT(sPrintPOForm()), formsMenu, "PrintPurchaseOrders", NULL, NULL, true , NULL },
    { "po.printVendorForm", tr("Print &Vendor Form..."), SLOT(sPrintVendorForm()), formsMenu, "MaintainVendors ViewVendors", NULL, NULL, true , NULL },

    //  Purchasing | Reports
    { "menu", tr("&Reports"), (char*)reportsMenu, mainMenu, "true", NULL, NULL, true , NULL },
    
    { "po.itemSites", tr("Item &Sites..."), SLOT(sItemSites()), reportsMenu, "ViewItemSites", NULL, NULL, true , NULL },
    
    // Purchasing | Reports | Item Sources
    { "po.dspItemSourcePrices", tr("&Item Source Prices..."), SLOT(sDspItemSourcePrices()), reportsMenu, "ViewItemSources", NULL, NULL, true , NULL },
    { "po.dspBuyCard", tr("&Buy Card..."), SLOT(sDspBuyCard()), reportsMenu, "ViewItemSources", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, reportsMenu, "true", NULL, NULL, true , NULL },
 
    //  Purchasing | Reports | Purchase Requests
    // { "menu", tr("Purchase Re&quests"), (char*)requestMenu, reportsMenu, "true", NULL, NULL, true , NULL },
    // { "separator", NULL, NULL, reportsMenu, "true", NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | Receipts and Returns
    { "menu", tr("&Receipts and Returns"), (char*)reportsRcptRtrnMenu, reportsMenu, "true", NULL, NULL, true , NULL },
    { "po.dspReceiptsAndReturnsByVendor", tr("by &Vendor..."), SLOT(sDspReceiptsReturnsByVendor()), reportsRcptRtrnMenu, "ViewReceiptsReturns", NULL, NULL, true , NULL },
    { "po.dspReceiptsAndReturnsByDate", tr("by &Date..."), SLOT(sDspReceiptsReturnsByDate()), reportsRcptRtrnMenu, "ViewReceiptsReturns", NULL, NULL, true , NULL },
    { "po.dspReceiptsAndReturnsByItem", tr("by &Item..."), SLOT(sDspReceiptsReturnsByItem()), reportsRcptRtrnMenu, "ViewReceiptsReturns", NULL, NULL, true , NULL },

    { "po.uninvoicedReceipts", tr("&Uninvoiced Receipts and Returns..."), SLOT(sDspUninvoicedReceipts()), reportsMenu, "ViewUninvoicedReceipts MaintainUninvoicedReceipts", NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, reportsMenu, "true", NULL, NULL, true , NULL },
    
    //  Purchasing | Reports | Price Variances
    { "menu", tr("Price &Variances"), (char*)reportsPriceVarMenu, reportsMenu, "true", NULL, NULL, true , NULL },
    { "po.dspPriceVariancesByVendor", tr("by &Vendor..."), SLOT(sDspPriceVariancesByVendor()), reportsPriceVarMenu, "ViewVendorPerformance", NULL, NULL, true , NULL },
    { "po.dspPriceVariancesByItem", tr("by &Item..."), SLOT(sDspPriceVariancesByItem()), reportsPriceVarMenu, "ViewVendorPerformance", NULL, NULL, true , NULL },

    //  Purchasing | Reports | Delivery Date Variance
    { "menu", tr("&Delivery Date Variances"), (char*)reportsDelvVarMenu, reportsMenu, "true", NULL, NULL, true , NULL },
    { "po.dspDeliveryDateVariancesByVendor", tr("by &Vendor..."), SLOT(sDspPoDeliveryDateVariancesByVendor()), reportsDelvVarMenu, "ViewVendorPerformance", NULL, NULL, true , NULL },
    { "po.dspDeliveryDateVariancesByItem", tr("by &Item..."), SLOT(sDspPoDeliveryDateVariancesByItem()), reportsDelvVarMenu, "ViewVendorPerformance", NULL, NULL, true , NULL },
    
    { "po.dspRejectedMaterialByVendor", tr("Rejected &Material..."), SLOT(sDspRejectedMaterialByVendor()), reportsMenu, "ViewVendorPerformance", NULL, NULL, true , NULL },

    { "separator", NULL, NULL, mainMenu, "true", NULL, NULL, true , NULL },

    //  Purchasing | Vendor
    { "menu", tr("V&endor"), (char*)vendorMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.newVendor", tr("&New..."), SLOT(sNewVendor()), vendorMenu, "MaintainVendors", NULL, NULL, true , NULL },
    { "po.vendors", tr("&List..."), SLOT(sVendors()), vendorMenu, "MaintainVendors ViewVendors", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, vendorMenu, "true", NULL, NULL, true , NULL },
    { "po.vendorWorkBench", tr("&Workbench..."), SLOT(sVendorWorkBench()), vendorMenu, "MaintainVendors", NULL, NULL, true , NULL },

    { "separator", NULL, NULL, mainMenu, "true", NULL, NULL, true , NULL },

    //  P/O | Contract
    { "menu", tr("&Contract"), (char*)contractsMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.enterNewContract", tr("&New..."), SLOT(sNewContract()), contractsMenu, "MaintainItemSources", NULL, NULL, true , NULL },
    { "po.listContracts", tr("&List..."), SLOT(sContracts()), contractsMenu, "MaintainItemSources ViewItemSources", NULL, NULL, true , NULL },

    //  P/O | Item Source
    { "menu", tr("&Item Source"), (char*)itemSourcesMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.enterNewItemSource", tr("&New..."), SLOT(sNewItemSource()), itemSourcesMenu, "MaintainItemSources", NULL, NULL, true , NULL },
    { "po.listItemSources", tr("&List..."), SLOT(sDspItemSources()), itemSourcesMenu, "MaintainItemSources ViewItemSources", NULL, NULL, true , NULL },

    { "separator", NULL, NULL, mainMenu, "true", NULL, NULL, true , NULL },

    // Purchasing | Utilities
    { "menu", tr("&Utilities"), (char*)utilitiesMenu, mainMenu, "true", NULL, NULL, true , NULL },
    { "po.itemsWithoutItemSources", tr("&Items without Item Sources..."), SLOT(sItemsWithoutItemSources()), utilitiesMenu, "ViewItemMasters", NULL, NULL, true , NULL },
    { "po.assignItemToPlannerCode", tr("&Assign Item to Planner Code..."), SLOT(sAssignItemToPlannerCode()), utilitiesMenu, "AssignItemsToPlannerCode", NULL, NULL, true , NULL },
    { "po.assignItemsToPlannerCodeByClassCode", tr("Assign Item&s to Planner Code..."), SLOT(sAssignClassCodeToPlannerCode()), utilitiesMenu, "AssignItemsToPlannerCode", NULL, NULL, true , NULL },

    // Setup
    { "po.setup",	tr("&Setup..."),	SLOT(sSetup()),	mainMenu,	NULL,	NULL,	NULL,	true, NULL}
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Purchase");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("P&urchase"));
}

void menuPurchase::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  QAction * m = 0;
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (acts[i].actionName == QString("menu"))
    {
      m = acts[i].menu->addMenu((QMenu*)(acts[i].slot));
      if(m)
        m->setText(acts[i].actionTitle);
    }
    else if (acts[i].actionName == QString("separator"))
    {
      m = acts[i].menu->addSeparator();
    }
    else if ((acts[i].toolBar != NULL) && (!acts[i].toolTip.isEmpty()))
    {
      m = new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar,
                  acts[i].toolTip) ;
    }
    else if (acts[i].toolBar != NULL)
    {
      m = new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar,
                  acts[i].actionTitle) ;
    }
    else
    {
      m = new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv ) ;
    }
    if (m) m->setVisible(acts[i].visible);
  }
}

void menuPurchase::sNewPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sPurchaseOrderEditList()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void menuPurchase::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", true).exec();
}

void menuPurchase::sPrintPurchaseOrdersByAgent()
{
  printPurchaseOrdersByAgent(parent, "", true).exec();
}

void menuPurchase::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", true).exec();
}

void menuPurchase::sPostPurchaseOrdersByAgent()
{
  postPurchaseOrdersByAgent(parent, "", true).exec();
}

void menuPurchase::sClosePurchaseOrder()
{
  closePurchaseOrder(parent, "", true).exec();
}

void menuPurchase::sReschedulePoitem()
{
  reschedulePoitem(parent, "", true).exec();
}

void menuPurchase::sChangePoitemQty()
{
  changePoitemQty(parent, "", true).exec();
}

void menuPurchase::sAddPoComment()
{
  addPoComment(parent, "", true).exec();
}

void menuPurchase::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void menuPurchase::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void menuPurchase::sPostVouchers()
{
  postVouchers(parent, "", true).exec();
}

void menuPurchase::sNewItemSource()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(parent, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void menuPurchase::sNewContract()
{
  ParameterList params;
  params.append("mode", "new");

  contract *newdlg = new contract();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

//  contract newdlg(parent, "", true);
//  newdlg.set(params);
//  newdlg.exec();
}

void menuPurchase::sContracts()
{
  omfgThis->handleNewWindow(new contracts());
}

void menuPurchase::sDspPurchaseReqsByItem()
{
  omfgThis->handleNewWindow(new dspPurchaseReqsByItem());
}

void menuPurchase::sDspPurchaseReqsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspPurchaseReqsByPlannerCode());
}

void menuPurchase::sItemSites()
{
  ParameterList params;

  itemSites *newdlg = new itemSites();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sDspPOsByDate()
{
  omfgThis->handleNewWindow(new dspPOsByDate());
}

void menuPurchase::sDspPOsByVendor()
{
  omfgThis->handleNewWindow(new dspPOsByVendor());
}

void menuPurchase::sDspPoItemsByVendor()
{
  omfgThis->handleNewWindow(new dspPoItemsByVendor());
}

void menuPurchase::sDspPoItemsByItem()
{
  omfgThis->handleNewWindow(new dspPoItemsByItem());
}

void menuPurchase::sDspPoItemsByDate()
{
  omfgThis->handleNewWindow(new dspPoItemsByDate());
}

void menuPurchase::sDspPoHistory()
{
  omfgThis->handleNewWindow(new dspPoHistory());
}

void menuPurchase::sDspItemSources()
{
  omfgThis->handleNewWindow(new dspItemSources());
}

void menuPurchase::sDspItemSourcePrices()
{
  omfgThis->handleNewWindow(new dspItemSourcePrices());
}

void menuPurchase::sDspBuyCard()
{
  omfgThis->handleNewWindow(new buyCard());
}

void menuPurchase::sDspReceiptsReturnsByVendor()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByVendor());
}

void menuPurchase::sDspReceiptsReturnsByItem()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByItem());
}

void menuPurchase::sDspReceiptsReturnsByDate()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByDate());
}

void menuPurchase::sDspPriceVariancesByVendor()
{
  omfgThis->handleNewWindow(new dspPoPriceVariancesByVendor());
}

void menuPurchase::sDspPriceVariancesByItem()
{
  omfgThis->handleNewWindow(new dspPoPriceVariancesByItem());
}

void menuPurchase::sDspPoDeliveryDateVariancesByItem()
{
  omfgThis->handleNewWindow(new dspPoDeliveryDateVariancesByItem());
}

void menuPurchase::sDspPoDeliveryDateVariancesByVendor()
{
  omfgThis->handleNewWindow(new dspPoDeliveryDateVariancesByVendor());
}

void menuPurchase::sDspRejectedMaterialByVendor()
{
  omfgThis->handleNewWindow(new dspPoReturnsByVendor());
}

void menuPurchase::sPrintPOForm()
{
  printPoForm(parent, "", true).exec();
}

void menuPurchase::sPrintVendorForm()
{
  printVendorForm(parent, "", true).exec();
}

//  Master Information
void menuPurchase::sNewVendor()
{
  ParameterList params;
  params.append("mode", "new");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuPurchase::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void menuPurchase::sVendorWorkBench()
{
  ParameterList params;
  if (_privileges->check("MaintainVendors"))
    params.append("mode", "edit");
  else
    params.append("mode", "view");
  
  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

// Utilities
void menuPurchase::sItemsWithoutItemSources()
{
  omfgThis->handleNewWindow(new dspItemsWithoutItemSources());
}

void menuPurchase::sAssignItemToPlannerCode()
{
  assignItemToPlannerCode(parent, "", true).exec();
}

void menuPurchase::sAssignClassCodeToPlannerCode()
{
  assignClassCodeToPlannerCode(parent, "", true).exec();
}

void menuPurchase::sSetup()
{
  ParameterList params;
  params.append("module", Xt::PurchaseModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}


