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

//  modulePO.cpp
//  Created 01/01/2001 JSL
//  Copyright (c) 2001-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include <pocluster.h>

#include "purchaseOrder.h"
#include "unpostedPurchaseOrders.h"
#include "printPurchaseOrder.h"
#include "printPurchaseOrdersByAgent.h"
#include "deliverPurchaseOrder.h"
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
#include "voucheringEditList.h"
#include "postVouchers.h"

#include "itemSource.h"
#include "itemSources.h"

#include "dspPurchaseReqsByItem.h"
#include "dspPurchaseReqsByPlannerCode.h"
#include "dspItemSitesByParameterList.h"
#include "dspPoItemsByVendor.h"
#include "dspPoItemsByItem.h"
#include "dspPoItemsByDate.h"
#include "dspPoItemsByBufferStatus.h"
#include "dspPoHistory.h"
#include "dspItemSourcesByVendor.h"
#include "dspItemSourcesByItem.h"
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
#include "printAnnodizingPurchaseRequests.h"

#include "vendor.h"
#include "searchForCRMAccount.h"
#include "vendors.h"
#include "vendorTypes.h"
#include "plannerCodes.h"
#include "rejectCodes.h"
#include "termses.h"
#include "expenseCategories.h"
#include "apAccountAssignments.h"

#include "dspItemsWithoutItemSources.h"
#include "assignItemToPlannerCode.h"
#include "assignClassCodeToPlannerCode.h"

#include "modulePO.h"

modulePO::modulePO(GUIClient *Pparent) :
  QObject(Pparent, "poModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("P/O Tools"));
  toolBar->setObjectName("P/O Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowPOToolbar"))
    parent->addToolBar(toolBar);

//  P/O | Purchase Orders
  ordersMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.newPurchaseOrder", tr("Enter New Purchase Order..."),
                                      this, SLOT(sNewPurchaseOrder()),
                                      ordersMenu, _privileges->check("MaintainPurchaseOrders")));
 //                                     QPixmap(":/images/newPurchaseOrder.xpm"), toolBar ) );

  parent->actions.append( new Action( parent, "po.listUnpostedPurchaseOrders", tr("List Unposted Purchase Orders..."),
                                      this, SLOT(sPurchaseOrderEditList()),
                                      ordersMenu, (_privileges->check("MaintainPurchaseOrders")) || (_privileges->check("ViewPurchaseOrders")),
									  QPixmap(":/images/listUnpostedPo.png"), toolBar ) );

  ordersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.printPurchaseOrder", tr("Print Purchase Order..."),
                                      this, SLOT(sPrintPurchaseOrder()),
                                      ordersMenu, _privileges->check("PrintPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.printPurchaseOrdersByAgent", tr("Print Purchase Orders by Agent..."),
                                      this, SLOT(sPrintPurchaseOrdersByAgent()),
                                      ordersMenu, _privileges->check("PrintPurchaseOrders") ) );

  if (_metrics->boolean("EnableBatchManager"))
      parent->actions.append( new Action( parent, "po.schedulePoForEmailDelivery", tr("Schedule P/O for Email Delivery..."),
                                          this, SLOT(sDeliverPurchaseOrder()),
                                          ordersMenu, _privileges->check("PrintPurchaseOrders") ) );

  ordersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.postPurchaseOrder", tr("Post Purchase Order..."),
                                      this, SLOT(sPostPurchaseOrder()),
                                      ordersMenu, _privileges->check("PostPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.postPurchaseOrdersByAgent", tr("Post Purchase Orders by Agent..."),
                                      this, SLOT(sPostPurchaseOrdersByAgent()),
                                      ordersMenu, _privileges->check("PostPurchaseOrders") ) );

  ordersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.closePurchaseOrder", tr("Close Purchase Order..."),
                                      this, SLOT(sClosePurchaseOrder()),
                                      ordersMenu, _privileges->check("MaintainPurchaseOrders") ) );

  ordersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.reschedulePurchaseOrderItem", tr("Reschedule Purchase Order Item..."),
                                      this, SLOT(sReschedulePoitem()),
                                      ordersMenu, _privileges->check("ReschedulePurchaseOrders") ) );

  parent->actions.append( new Action( parent, "wo.changePurchaseOrderItemQty", tr("Change Purchase Order Item Qty..."),
                                      this, SLOT(sChangePoitemQty()),
                                      ordersMenu, _privileges->check("ChangePurchaseOrderQty") ) );

  parent->actions.append( new Action( parent, "wo.addCommentToPurchaseOrder", tr("Add Comment to Purchase Order..."),
                                      this, SLOT(sAddPoComment()),
                                      ordersMenu, _privileges->check("MaintainPurchaseOrders") ) );


//  P/O | Vouchers
  vouchersMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.uninvoicedReceipts", tr("Uninvoiced Receipts and Returns..."),
                                      this, SLOT(sDspUninvoicedReceipts()),
                                      vouchersMenu, (_privileges->check("ViewUninvoicedReceipts") || _privileges->check("MaintainUninvoicedReceipts")) ) );

  vouchersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.enterNewVoucher", tr("Enter New Voucher..."),
                                      this, SLOT(sEnterVoucher()),
                                      vouchersMenu, _privileges->check("MaintainVouchers") ) );

  parent->actions.append( new Action( parent, "po.enterNewMiscVoucher", tr("Enter New Miscellaneous Voucher..."),
                                      this, SLOT(sEnterMiscVoucher()),
                                      vouchersMenu, _privileges->check("MaintainVouchers") ) );

  parent->actions.append( new Action( parent, "po.listUnpostedVouchers", tr("List Unposted Vouchers..."),
                                      this, SLOT(sUnpostedVouchers()),
                                      vouchersMenu, (_privileges->check("MaintainVouchers") || _privileges->check("ViewVouchers")) ) );

  vouchersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.voucheringEditList", tr("Vouchering Edit List..."),
                                      this, SLOT(sVoucheringEditList()),
                                      vouchersMenu, (_privileges->check("MaintainVouchers") || _privileges->check("ViewVouchers")) ) );

  vouchersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.postVouchers", tr("Post Vouchers..."),
                                      this, SLOT(sPostVouchers()),
                                      vouchersMenu, _privileges->check("PostVouchers") ) );


//  P/O | Item Sources
  itemSourcesMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.enterNewItemSource", tr("Enter New Item Source..."),
                                      this, SLOT(sNewItemSource()),
                                      itemSourcesMenu, _privileges->check("MaintainItemSources") ) );

  parent->actions.append( new Action( parent, "po.listItemSources", tr("List Item Sources..."),
                                      this, SLOT(sItemSources()),
                                      itemSourcesMenu, (_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSources")) ) );


//  P/O | Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.dspPurchaseRequestsByItem", tr("Purchase Requests by Item..."),
                                      this, SLOT(sDspPurchaseReqsByItem()),
                                      displaysMenu, _privileges->check("ViewPurchaseRequests") ) );

  parent->actions.append( new Action( parent, "po.dspPurchaseRequestsByPlannerCode", tr("Purchase Requests by Planner Code..."),
                                      this, SLOT(sDspPurchaseReqsByPlannerCode()),
                                      displaysMenu, _privileges->check("ViewPurchaseRequests") ,
									  QPixmap(":/images/dspPurchaseReqByPlannerCode.png"), toolBar ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.dspItemSitesByPlannerCode", tr("Item Sites by Planner Code..."),
                                      this, SLOT(sDspItemSitesByPlannerCode()),
                                      displaysMenu, _privileges->check("ViewItemSites") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.dspPOsByDate", tr("P/Os by Date..."),
                                      this, SLOT(sDspPOsByDate()),
                                      displaysMenu, _privileges->check("ViewPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.dspPOsByVendor", tr("P/Os by Vendor..."),
                                      this, SLOT(sDspPOsByVendor()),
                                      displaysMenu, _privileges->check("ViewPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.dspPoLineItemsByVendor", tr("P/O Items by Vendor..."),
                                      this, SLOT(sDspPoItemsByVendor()),
                                      displaysMenu, _privileges->check("ViewPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.dspPoLineItemsByItem", tr("P/O Items by Item..."),
                                      this, SLOT(sDspPoItemsByItem()),
                                      displaysMenu, _privileges->check("ViewPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.dspPoLineItemsByDate", tr("P/O Items by Date..."),
                                      this, SLOT(sDspPoItemsByDate()),
                                      displaysMenu, _privileges->check("ViewPurchaseOrders") ) );

 if ( _metrics->boolean("BufferMgt"))
      parent->actions.append( new Action( parent, "po.dspPoLineItemsByBufferStatus", tr("P/O Items by Buffer Status..."),
                                          this, SLOT(sDspPoItemsByBufferStatus()),
                                          displaysMenu, _privileges->check("ViewPurchaseOrders") ) );
  
  parent->actions.append( new Action( parent, "po.dspPoHistory", tr("P/O History..."),
                                      this, SLOT(sDspPoHistory()),
                                      displaysMenu, _privileges->check("ViewPurchaseOrders") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.dspItemSourcesByVendor", tr("Item Sources by Vendor..."),
                                      this, SLOT(sDspItemSourcesByVendor()),
                                      displaysMenu, _privileges->check("ViewItemSources") ) );

  parent->actions.append( new Action( parent, "po.dspItemSourcesByItem", tr("Item Sources by Item..."),
                                      this, SLOT(sDspItemSourcesByItem()),
                                      displaysMenu, _privileges->check("ViewItemSources") ) );

  parent->actions.append( new Action( parent, "po.dspBuyCard", tr("Buy Card..."),
                                      this, SLOT(sDspBuyCard()),
                                      displaysMenu, _privileges->check("ViewItemSources") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.dspReceiptsAndReturnsByVendor", tr("Receipts and Returns by Vendor..."),
                                      this, SLOT(sDspReceiptsReturnsByVendor()),
                                      displaysMenu, _privileges->check("ViewReceiptsReturns") ) );

  parent->actions.append( new Action( parent, "po.dspReceiptsAndReturnsByItem", tr("Receipts and Returns by Item..."),
                                      this, SLOT(sDspReceiptsReturnsByItem()),
                                      displaysMenu, _privileges->check("ViewReceiptsReturns") ) );

  parent->actions.append( new Action( parent, "po.dspReceiptsAndReturnsByDate", tr("Receipts and Returns by Date..."),
                                      this, SLOT(sDspReceiptsReturnsByDate()),
                                      displaysMenu, _privileges->check("ViewReceiptsReturns") ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.dspPriceVariancesByVendor", tr("Price Variances by Vendor..."),
                                      this, SLOT(sDspPriceVariancesByVendor()),
                                      displaysMenu, _privileges->check("ViewVendorPerformance") ) );

  parent->actions.append( new Action( parent, "po.dspPriceVariancesByItem", tr("Price Variances by Item..."),
                                      this, SLOT(sDspPriceVariancesByItem()),
                                      displaysMenu, _privileges->check("ViewVendorPerformance") ) );

  parent->actions.append( new Action( parent, "po.dspDeliveryDateVariancesByVendor", tr("Delivery Date Variances by Vendor..."),
                                      this, SLOT(sDspPoDeliveryDateVariancesByVendor()),
                                      displaysMenu, _privileges->check("ViewVendorPerformance") ) );

  parent->actions.append( new Action( parent, "po.dspDeliveryDateVariancesByItem", tr("Delivery Date Variances by Item..."),
                                      this, SLOT(sDspPoDeliveryDateVariancesByItem()),
                                      displaysMenu, _privileges->check("ViewVendorPerformance") ) );

  parent->actions.append( new Action( parent, "po.dspRejectedMaterialByVendor", tr("Rejected Material by Vendor..."),
                                      this, SLOT(sDspRejectedMaterialByVendor()),
                                      displaysMenu, _privileges->check("ViewVendorPerformance") ) );


//  P/O | Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.printPoForm", tr("Print P/O Form..."),
                                      this, SLOT(sPrintPOForm()),
                                      reportsMenu, _privileges->check("PrintPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "po.printVendorForm", tr("Print Vendor Form..."),
                                      this, SLOT(sPrintVendorForm()),
                                      reportsMenu, (_privileges->check("MaintainVendors") || _privileges->check("ViewVendors")) ) );

  if(_metrics->boolean("EnablePrintAnnodizingPurchaseRequests"))
  {
    parent->actions.append( new Action( parent, "po.printAnnodizingPurchaseRequests", tr("Print Annodizing Purchase Requests..."),
                                        this, SLOT(sPrintAnnodizingPurchaseRequests()),
                                        reportsMenu, _privileges->check("ViewPurchaseRequests") ) );
  }


//  P/O | Master Information
  masterInfoMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.newVendor", tr("New Vendor..."),
                                      this, SLOT(sNewVendor()),
                                      masterInfoMenu, _privileges->check("MaintainVendors") ) );

  parent->actions.append( new Action( parent, "po.searchForVendor", tr("Search for Vendor..."),
                                      this, SLOT(sSearchForVendor()),
                                      masterInfoMenu, (_privileges->check("MaintainVendors")) || (_privileges->check("ViewVendors")) ) );

  parent->actions.append( new Action( parent, "po.vendors", tr("Vendors..."),
                                      this, SLOT(sVendors()),
                                      masterInfoMenu, (_privileges->check("MaintainVendors")) || (_privileges->check("ViewVendors")) ) );

  parent->actions.append( new Action( parent, "po.vendorTypes", tr("Vendor Types..."),
                                      this, SLOT(sVendorTypes()),
                                      masterInfoMenu, (_privileges->check("MaintainVendorTypes")) || (_privileges->check("ViewVendorTypes")) ) );

  masterInfoMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.terms", tr("Terms..."),
                                      this, SLOT(sTerms()),
                                      masterInfoMenu, (_privileges->check("MaintainTerms")) || (_privileges->check("ViewTerms")) ) );

  parent->actions.append( new Action( parent, "po.plannerCodes", tr("Planner Codes..."),
                                      this, SLOT(sPlannerCodes()),
                                      masterInfoMenu, (_privileges->check("MaintainPlannerCodes")) || (_privileges->check("ViewPlannerCodes")) ) );

  parent->actions.append( new Action( parent, "po.rejectCodes", tr("Reject Codes..."),
                                      this, SLOT(sRejectCodes()),
                                      masterInfoMenu, (_privileges->check("MaintainRejectCodes")) || (_privileges->check("ViewRejectCodes")) ) );

  parent->actions.append( new Action( parent, "po.expenseCategories", tr("Expense Categories..."),
                                      this, SLOT(sExpenseCategories()),
                                      masterInfoMenu, (_privileges->check("MaintainExpenseCategories")) || (_privileges->check("ViewExpenseCategories")) ) );

  masterInfoMenu->insertSeparator();

  parent->actions.append( new Action( parent, "po.apAccountAssignments", tr("A/P Account Assignments..."),
                                      this, SLOT(sAPAssignments()),
                                      masterInfoMenu, (_privileges->check("MaintainVendorAccounts")) || (_privileges->check("ViewVendorAccounts")) ) );


//  P/O | Utilities
  utilitiesMenu = new QMenu();

  parent->actions.append( new Action( parent, "po.itemsWithoutItemSources", tr("Items without Item Sources..."),
                                      this, SLOT(sItemsWithoutItemSources()),
                                      utilitiesMenu, _privileges->check("ViewItemMasters") ) );

  parent->actions.append( new Action( parent, "po.assignItemToPlannerCode", tr("Assign Item to Planner Code..."),
                                      this, SLOT(sAssignItemToPlannerCode()),
                                      utilitiesMenu, _privileges->check("AssignItemsToPlannerCode") ) );

  parent->actions.append( new Action( parent, "po.assignItemsToPlannerCodeByClassCode", tr("Assign Items to Planner Code by Class Code..."),
                                      this, SLOT(sAssignClassCodeToPlannerCode()),
                                      utilitiesMenu, _privileges->check("AssignItemsToPlannerCode") ) );


//  P/O
  mainMenu = new QMenu();
  mainMenu->insertItem(tr("Purchase Orders"),     ordersMenu          );
  mainMenu->insertItem(tr("Vouchers"),            vouchersMenu        );
  mainMenu->insertItem(tr("Item Sources"),        itemSourcesMenu     );
  mainMenu->insertItem(tr("Displays"),            displaysMenu        );
  mainMenu->insertItem(tr("Reports"),             reportsMenu         );
  mainMenu->insertItem(tr("Master Information"),  masterInfoMenu      );
  mainMenu->insertItem(tr("&Utilities"),          utilitiesMenu       );
  parent->populateCustomMenu(mainMenu, "P/O");
  parent->menuBar()->insertItem(tr("&P/O"), mainMenu);
}

void modulePO::sNewPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePO::sPurchaseOrderEditList()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void modulePO::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", TRUE).exec();
}

void modulePO::sPrintPurchaseOrdersByAgent()
{
  printPurchaseOrdersByAgent(parent, "", TRUE).exec();
}

void modulePO::sDeliverPurchaseOrder()
{
  deliverPurchaseOrder(parent, "", TRUE).exec();
}

void modulePO::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", TRUE).exec();
}

void modulePO::sPostPurchaseOrdersByAgent()
{
  postPurchaseOrdersByAgent(parent, "", TRUE).exec();
}

void modulePO::sClosePurchaseOrder()
{
  closePurchaseOrder(parent, "", TRUE).exec();
}

void modulePO::sReschedulePoitem()
{
  reschedulePoitem(parent, "", TRUE).exec();
}

void modulePO::sChangePoitemQty()
{
  changePoitemQty(parent, "", TRUE).exec();
}

void modulePO::sAddPoComment()
{
  addPoComment(parent, "", TRUE).exec();
}

void modulePO::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void modulePO::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePO::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePO::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void modulePO::sVoucheringEditList()
{
  omfgThis->handleNewWindow(new voucheringEditList());
}

void modulePO::sPostVouchers()
{
  postVouchers(parent, "", TRUE).exec();
}

void modulePO::sNewItemSource()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void modulePO::sItemSources()
{
  omfgThis->handleNewWindow(new itemSources());
}

void modulePO::sDspPurchaseReqsByItem()
{
  omfgThis->handleNewWindow(new dspPurchaseReqsByItem());
}

void modulePO::sDspPurchaseReqsByPlannerCode()
{
  omfgThis->handleNewWindow(new dspPurchaseReqsByPlannerCode());
}

void modulePO::sDspItemSitesByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  dspItemSitesByParameterList *newdlg = new dspItemSitesByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePO::sDspPOsByDate()
{
  omfgThis->handleNewWindow(new dspPOsByDate());
}

void modulePO::sDspPOsByVendor()
{
  omfgThis->handleNewWindow(new dspPOsByVendor());
}

void modulePO::sDspPoItemsByVendor()
{
  omfgThis->handleNewWindow(new dspPoItemsByVendor());
}

void modulePO::sDspPoItemsByItem()
{
  omfgThis->handleNewWindow(new dspPoItemsByItem());
}

void modulePO::sDspPoItemsByDate()
{
  omfgThis->handleNewWindow(new dspPoItemsByDate());
}

void modulePO::sDspPoItemsByBufferStatus()
{
  omfgThis->handleNewWindow(new dspPoItemsByBufferStatus());
}

void modulePO::sDspPoHistory()
{
  omfgThis->handleNewWindow(new dspPoHistory());
}

void modulePO::sDspItemSourcesByVendor()
{
  omfgThis->handleNewWindow(new dspItemSourcesByVendor());
}

void modulePO::sDspItemSourcesByItem()
{
  omfgThis->handleNewWindow(new dspItemSourcesByItem());
}

void modulePO::sDspBuyCard()
{
  omfgThis->handleNewWindow(new buyCard());
}

void modulePO::sDspReceiptsReturnsByVendor()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByVendor());
}

void modulePO::sDspReceiptsReturnsByItem()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByItem());
}

void modulePO::sDspReceiptsReturnsByDate()
{
  omfgThis->handleNewWindow(new dspPoItemReceivingsByDate());
}

void modulePO::sDspPriceVariancesByVendor()
{
  omfgThis->handleNewWindow(new dspPoPriceVariancesByVendor());
}

void modulePO::sDspPriceVariancesByItem()
{
  omfgThis->handleNewWindow(new dspPoPriceVariancesByItem());
}

void modulePO::sDspPoDeliveryDateVariancesByItem()
{
  omfgThis->handleNewWindow(new dspPoDeliveryDateVariancesByItem());
}

void modulePO::sDspPoDeliveryDateVariancesByVendor()
{
  omfgThis->handleNewWindow(new dspPoDeliveryDateVariancesByVendor());
}

void modulePO::sDspRejectedMaterialByVendor()
{
  omfgThis->handleNewWindow(new dspPoReturnsByVendor());
}

void modulePO::sPrintPOForm()
{
  printPoForm(parent, "", TRUE).exec();
}

void modulePO::sPrintVendorForm()
{
  printVendorForm(parent, "", TRUE).exec();
}

void modulePO::sPrintAnnodizingPurchaseRequests()
{
  printAnnodizingPurchaseRequests(parent, "", TRUE).exec();
}

//  Master Information
void modulePO::sNewVendor()
{
  ParameterList params;
  params.append("mode", "new");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePO::sSearchForVendor()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "vend");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void modulePO::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void modulePO::sVendorTypes()
{
  omfgThis->handleNewWindow(new vendorTypes());
}

void modulePO::sPlannerCodes()
{
  omfgThis->handleNewWindow(new plannerCodes());
}

void modulePO::sRejectCodes()
{
  omfgThis->handleNewWindow(new rejectCodes());
}

void modulePO::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void modulePO::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}


// Utilities
void modulePO::sItemsWithoutItemSources()
{
  omfgThis->handleNewWindow(new dspItemsWithoutItemSources());
}

void modulePO::sAssignItemToPlannerCode()
{
  assignItemToPlannerCode(parent, "", TRUE).exec();
}

void modulePO::sAssignClassCodeToPlannerCode()
{
  assignClassCodeToPlannerCode(parent, "", TRUE).exec();
}

void modulePO::sAPAssignments()
{
  omfgThis->handleNewWindow(new apAccountAssignments());
}
