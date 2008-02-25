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

//  moduleSO.cpp
//  Created 02/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include "salesOrder.h"
#include "openSalesOrders.h"
#include "rescheduleSoLineItems.h"
#include "quotes.h"

#include "packingListBatch.h"
#include "printPackingList.h"

#include "uninvoicedShipments.h"
#include "selectShippedOrders.h"
#include "selectOrderForBilling.h"
#include "billingEditList.h"
#include "dspBillingSelections.h"
#include "postBillingSelections.h"
#include "unpostedInvoices.h"
#include "reprintInvoices.h"
#include "deliverInvoice.h"
#include "printInvoices.h"
#include "printInvoicesByShipvia.h"
#include "purgeInvoices.h"

#include "creditMemo.h"
#include "unpostedCreditMemos.h"
#include "creditMemoEditList.h"
#include "printCreditMemos.h"
#include "reprintCreditMemos.h"
#include "postCreditMemos.h"
#include "purgeCreditMemos.h"

#include "postInvoices.h"

#include "itemListPrice.h"
#include "updateListPricesByProductCategory.h"
#include "itemPricingSchedules.h"
#include "pricingScheduleAssignments.h"
#include "sales.h"
#include "updatePricesByProductCategory.h"
#include "updatePricesByPricingSchedule.h"

#include "dspPricesByItem.h"
#include "dspPricesByCustomer.h"
#include "dspPricesByCustomerType.h"

#include "dspSalesOrdersByCustomer.h"
#include "dspSalesOrdersByItem.h"
#include "dspSalesOrdersByCustomerPO.h"
#include "dspSalesOrdersByParameterList.h"
#include "dspQuotesByCustomer.h"
#include "dspQuotesByItem.h"
#include "dspCustomersByCustomerType.h"
#include "dspCustomersByCharacteristic.h"
#include "dspCustomerInformation.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspInventoryAvailabilityBySalesOrder.h"
#include "dspSalesOrderStatus.h"
#include "dspBacklogByItem.h"
#include "dspBacklogBySalesOrder.h"
#include "dspBacklogByCustomer.h"
#include "dspBacklogByParameterList.h"
#include "dspSummarizedBacklogByWarehouse.h"
#include "dspPartiallyShippedOrders.h"
#include "dspEarnedCommissions.h"
#include "dspBriefEarnedCommissions.h"
#include "dspSummarizedTaxableSales.h"

#include "printSoForm.h"
#include "deliverSalesOrder.h"

#include "customer.h"
#include "searchForCRMAccount.h"
#include "customers.h"
#include "prospect.h"
#include "prospects.h"
#include "updateCreditStatusByCustomer.h"
#include "customerGroups.h"
#include "shipVias.h"
#include "shippingChargeTypes.h"
#include "taxCodes.h"
#include "customerTypes.h"
#include "termses.h"
#include "salesReps.h"
#include "shippingZones.h"
#include "shippingForms.h"
#include "salesAccounts.h"
#include "arAccountAssignments.h"
#include "customerFormAssignments.h"
#include "salesCategories.h"

#include "dspCustomerInformationExport.h"
#include "reassignCustomerTypeByCustomerType.h"
#include "characteristics.h"

// START_RW
#include "rwInterface.h"
#include "exportCustomers.h"
// END_RW

#include "moduleSO.h"

moduleSO::moduleSO(GUIClient *pParent) :
  QObject(pParent, "soModule")
{
  parent = pParent;

  toolBar = new QToolBar(tr("S/O Tools"));
  toolBar->setObjectName("S/O Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowSOToolbar"))
    parent->addToolBar(toolBar);

  mainMenu		= new QMenu();
  ordersMenu		= new QMenu();
  billingMenu		= new QMenu();
  billingInvoicesMenu	= new QMenu();
  billingCreditMemosMenu= new QMenu();
  pricingDisplaysMenu	= new QMenu();
  pricingMenu		= new QMenu();
  displaysMenu		= new QMenu();
  reportsMenu		= new QMenu();
  masterInfoMenu	= new QMenu();
  utilitiesMenu		= new QMenu();

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
    { "menu",	tr("Orders"),	(char*)ordersMenu,	mainMenu,	true,	NULL, NULL, true},
    { "so.newSalesOrder", 	     tr("New Sales Order..."),		SLOT(sNewSalesOrder()),   ordersMenu, _privleges->check("MaintainSalesOrders"), NULL, NULL,	 true},
    { "so.listOpenSalesOrders",      tr("List Open Sales Orders..."),	SLOT(sOpenSalesOrders()), ordersMenu, (_privleges->check("MaintainSalesOrders") || _privleges->check("ViewSalesOrders")),	new QPixmap(":/images/listOpenSalesOrders.png"), toolBar,  true},
    { "so.rescheduleAllSoLineItems", tr("Reschedule all S/O Line Items..."),	SLOT(sRescheduleSoLineItems()),  ordersMenu, _privleges->check("MaintainSalesOrders"),	 NULL, NULL, true},
    { "so.dspCustomerInformation",   tr("Customer Information Workbench..."),	SLOT(sDspCustomerInformation()), ordersMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")), new QPixmap(":/images/customerInformationWorkbench.png"), toolBar,  true},
    { "separator",	NULL,	NULL,	ordersMenu,	true,		NULL, NULL, true},
    { "so.packingListBatch", tr("Packing List Batch..."),	SLOT(sPackingListBatch()), ordersMenu, (_privleges->check("MaintainPackingListBatch") || _privleges->check("ViewPackingListBatch")),	NULL, NULL, true},
    { "so.printPackingList", tr("Print Packing List..."),	SLOT(sPrintPackingList()), ordersMenu, _privleges->check("PrintPackingLists"),	NULL, NULL, true},
    { "so.deliverSalesOrder", tr("Schedule S/O for Email Delivery..."),	SLOT(sDeliverSalesOrder()), ordersMenu, _privleges->check("ViewSalesOrders") ,	NULL, NULL, _metrics->boolean("EnableBatchManager")},

    { "separator",	NULL,	NULL,	ordersMenu,	true,		NULL, NULL, true},

    { "so.newQuote", tr("New Quote..."),	SLOT(sNewQuote()), ordersMenu, _privleges->check("MaintainQuotes"),	NULL, NULL, true},
    { "so.listQuotes", tr("List Quotes..."),	SLOT(sQuotes()), ordersMenu, (_privleges->check("MaintainQuotes") || _privleges->check("ViewQuotes")),	NULL, NULL, true},

    { "menu",	tr("Billing"),     (char*)billingMenu,		mainMenu,	true,	NULL, NULL, true},
    { "menu",	tr("&Invoices"),   (char*)billingInvoicesMenu,	billingMenu,	true,	NULL, NULL, true},
    { "so.uninvoicedShipments",		     tr("Uninvoiced Shipments..."),			SLOT(sUninvoicedShipments()), 		billingInvoicesMenu, _privleges->check("SelectBilling"),	 new QPixmap(":/images/uninvoicedShipments"), toolBar, true},
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true},
    { "so.selectAllShippedOrdersForBilling", tr("Select All Shipped Orders for Billing..."),	SLOT(sSelectShippedOrdersForBilling()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true},
    { "so.selectOrderForBilling",	     tr("Select Order for Billing..."),			SLOT(sSelectOrderForBilling()),		billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true},
    { "so.billingEditList",		     tr("Billing Edit List..."),	SLOT(sBillingEditList()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true},
    { "so.dspBillingSelections",	     tr("Billing Selections..."),	SLOT(sDspBillingSelections()), billingInvoicesMenu, _privleges->check("SelectBilling"), new QPixmap(":/images/billingSelections"), toolBar, true},
    { "so.postBillingSelections",	     tr("Post Billing Selections..."),	SLOT(sPostBillingSelections()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true},
    { "so.listUnpostedInvoices",	     tr("List Unposted Invoices..."),	SLOT(sUnpostedInvoices()), billingInvoicesMenu, _privleges->check("SelectBilling"),	NULL, NULL,  true},
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true},
    { "so.printInvoices",		     tr("Print Invoices..."),		SLOT(sPrintInvoices()), billingInvoicesMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true},
    { "so.printInvoicesByShipvia",	     tr("Print Invoices by Ship Via..."),	SLOT(sPrintInvoicesByShipvia()), billingInvoicesMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true},
    { "so.reprintInvoices",		     tr("Re-Print Invoices..."),	SLOT(sReprintInvoices()), billingInvoicesMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true},
    { "so.scheduleInvoiceForEmailDelivery",  tr("Schedule Invoice for Email Delivery..."),	SLOT(sDeliverInvoice()), billingInvoicesMenu, _privleges->check("PrintInvoices"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true},
    { "so.postInvoices",		     tr("Post Invoices..."),		SLOT(sPostInvoices()), billingInvoicesMenu, _privleges->check("PostMiscInvoices"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	true,		NULL, NULL, true},
    { "so.purgeInvoices",		     tr("Purge Invoices..."),		SLOT(sPurgeInvoices()), billingInvoicesMenu, _privleges->check("PurgeInvoices"),	NULL, NULL, true},

    { "menu",	tr("&Credit Memos"), (char*)billingCreditMemosMenu,	billingMenu,	true,	NULL, NULL, true},
    { "so.newCreditMemo",		     tr("New Credit Memo..."),		SLOT(sNewCreditMemo()), billingCreditMemosMenu, _privleges->check("MaintainCreditMemos"),	NULL, NULL, true},
    { "so.listUnpostedCreditMemos",	     tr("List Unposted Credit Memos..."),	SLOT(sUnpostedCreditMemos()), billingCreditMemosMenu, (_privleges->check("MaintainCreditMemos") || _privleges->check("ViewCreditMemos")),	NULL, NULL, true},
    { "so.creditMemoEditList",		     tr("Credit Memo Edit List..."),	SLOT(sCreditMemoEditList()), billingCreditMemosMenu, (_privleges->check("MaintainCreditMemos") || _privleges->check("ViewCreditMemos")),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingCreditMemosMenu,	true,		NULL, NULL, true},
    { "so.printCreditMemos",		     tr("Print Credit Memos..."),	SLOT(sPrintCreditMemos()), billingCreditMemosMenu, _privleges->check("PrintCreditMemos"),	NULL, NULL, true},
    { "so.reprintCreditMemos",		     tr("Re-Print Credit Memos..."),	SLOT(sReprintCreditMemos()), billingCreditMemosMenu, _privleges->check("PrintCreditMemos"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingCreditMemosMenu,	true,		NULL, NULL, true},
    { "so.postCreditMemos",		     tr("Post Credit Memos..."),	SLOT(sPostCreditMemos()), billingCreditMemosMenu, _privleges->check("PostARDocuments"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	billingCreditMemosMenu,	true,		NULL, NULL, true},
    { "so.purgeCreditMemos",		     tr("Purge Credit Memos..."),	SLOT(sPurgeCreditMemos()), billingCreditMemosMenu, _privleges->check("PurgeCreditMemos"),	NULL, NULL, true},

// START_RW
    { "separator",	NULL,	NULL,	billingMenu,	true,	NULL,	NULL,	_metrics->boolean("EnableExternalAccountingInterface")},
    { "so.exportAROpenItemsAndDistributions", tr("Export Unposted A/R Open Items and Distributions..."), SLOT(sPostAROpenAndDist()), billingMenu, _privleges->check("PostARDocuments"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface")},
// END_RW

    { "menu",	tr("Item Pricing"),       (char*)pricingMenu,	mainMenu,	true,	NULL, NULL, true},
    { "so.itemListPrice", tr("Item List Price..."),	SLOT(sItemListPrice()), pricingMenu, (_privleges->check("MaintainListPrices") || _privleges->check("ViewListPrices")),	NULL, NULL, true},
    { "so.updateListPricesByProductCategory", tr("Update List Prices by Product Category..."),	SLOT(sUpdateListPricesByProductCategory()), pricingMenu, _privleges->check("MaintainListPrices"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	pricingMenu,	true,		NULL, NULL, true},
    { "so.pricingSchedules", tr("Pricing Schedules..."),	SLOT(sPricingSchedules()), pricingMenu, (_privleges->check("MaintainListPrices") || _privleges->check("ViewListPrices")),	NULL, NULL, true},
    { "so.pricingScheduleAssignments", tr("Pricing Schedule Assignments..."),	SLOT(sPricingScheduleAssignments()), pricingMenu, _privleges->check("AssignPricingSchedules"),	NULL, NULL, true},
    { "so.sales", tr("Sales..."),	SLOT(sSales()), pricingMenu, _privleges->check("CreateSales"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	pricingMenu,	true,		NULL, NULL, true},
    { "so.updatePricesByProductCategory", tr("Update Prices by Product Category..."),	SLOT(sUpdatePricesByProductCategory()), pricingMenu, _privleges->check("UpdatePricingSchedules"),	NULL, NULL, true},
    { "so.updatePricesByPricingSchedule", tr("Update Prices by Pricing Schedule..."),	SLOT(sUpdatePricesByPricingSchedule()), pricingMenu, _privleges->check("UpdatePricingSchedules"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	pricingMenu,	true,		NULL, NULL, true},

    { "menu",	tr("&Displays"),	(char*)pricingDisplaysMenu,	pricingMenu,	true,	NULL, NULL, true},
    { "so.dspPricesByItem", tr("Prices by Item..."),	SLOT(sDspPricesByItem()), pricingDisplaysMenu, _privleges->check("ViewCustomerPrices"), NULL, NULL,	 true},
    { "so.dspPricesByCustomer", tr("Prices by Customer..."),	SLOT(sDspPricesByCustomer()), pricingDisplaysMenu, _privleges->check("ViewCustomerPrices"), NULL, NULL,	 true},
    { "so.dspPricesByCustomerType", tr("Prices by Customer Type..."),	SLOT(sDspPricesByCustomerType()), pricingDisplaysMenu, _privleges->check("ViewCustomerPrices"), NULL, NULL,	 true},

    { "menu",	tr("Displays"),           (char*)displaysMenu,	mainMenu,	true,	NULL, NULL, true},
    { "so.dspSalesOrderLookupByCustomer", tr("Sales Order Lookup by Customer..."),	SLOT(sDspOrderLookupByCustomer()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "so.dspSalesOrderLookupByCustomerType", tr("Sales Order Lookup by Customer Type..."),	SLOT(sDspOrderLookupByCustomerType()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "so.dspSalesOrderLookupByItem", tr("Sales Order Lookup by Item..."),	SLOT(sDspOrderLookupByItem()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "so.dspSalesOrderLookupByCustomerPO", tr("Sales Order Lookup by Customer PO..."),	SLOT(sDspOrderLookupByCustomerPO()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspQuoteLookupByCustomer", tr("Quote Lookup by Customer..."),	SLOT(sDspQuoteLookupByCustomer()), displaysMenu, _privleges->check("ViewQuotes"),	NULL, NULL, true},
    { "so.dspQuoteOrderLookupByItem", tr("Quote Lookup by Item..."),	SLOT(sDspQuoteLookupByItem()), displaysMenu, _privleges->check("ViewQuotes"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspCustomersByCustomerType", tr("Customers by Customer Type..."),	SLOT(sDspCustomersByCusttype()), displaysMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true},
    { "so.dspCustomersByCharacteristic", tr("Customers by Characteristic..."),	SLOT(sDspCustomersByCharacteristic()), displaysMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspInventoryAvailabilityByItem", tr("Inventory Availability by Item..."),	SLOT(sDspInventoryAvailabilityByItem()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true},
    { "so.dspInventoryAvailabilityBySalesOrder", tr("Inventory Availability by Sales Order..."),	SLOT(sDspInventoryAvailabilityBySalesOrder()), displaysMenu, _privleges->check("ViewInventoryAvailability"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspSalesOrderStatus", tr("Sales Order Status..."),	SLOT(sDspSalesOrderStatus()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspBacklogByItem", tr("Backlog by Item..."),	SLOT(sDspBacklogByItem()), displaysMenu, _privleges->check("ViewSalesOrders"), NULL, NULL,	 true},
    { "so.dspBacklogBySalesOrder", tr("Backlog by Sales Order..."),	SLOT(sDspBacklogBySalesOrder()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "so.dspBacklogByCustomer", tr("Backlog by Customer..."),	SLOT(sDspBacklogByCustomer()), displaysMenu, _privleges->check("ViewSalesOrders"), NULL, NULL,	 true},
    { "so.dspBacklogByCustomerType", tr("Backlog by Customer Type..."),	SLOT(sDspBacklogByCustomerType()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "so.dspBacklogByCustomerGroup", tr("Backlog by Customer Group..."),	SLOT(sDspBacklogByCustomerGroup()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "so.dspBacklogByProductCategory", tr("Backlog by Product Category..."),	SLOT(sDspBacklogByProductCategory()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspSummarizedBacklogByWarehouse", tr("Summarized Backlog by Warehouse..."),	SLOT(sDspSummarizedBacklogByWarehouse()), displaysMenu, _privleges->check("ViewSalesOrders"),	new QPixmap(":/images/dspSummarizedBacklogByWhse.png"), toolBar,  true},
    { "so.dspPartiallyShippedOrders", tr("Partially Shipped Orders..."),	SLOT(sDspPartiallyShippedOrders()), displaysMenu, _privleges->check("ViewSalesOrders"),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	displaysMenu,	true,		NULL, NULL, true},
    { "so.dspEarnedCommissions", tr("Earned Commissions..."),	SLOT(sDspEarnedCommissions()), displaysMenu, _privleges->check("ViewCommissions"),	NULL, NULL, true},
    { "so.dspBriefEarnedCommissions", tr("Brief Earned Commissions..."),	SLOT(sDspBriefEarnedCommissions()), displaysMenu, _privleges->check("ViewCommissions"),	NULL, NULL, true},
    { "so.dspSummarizedTaxableSales", tr("Summarized Taxable Sales..."),	SLOT(sDspSummarizedTaxableSales()), displaysMenu, _privleges->check("ViewCommissions"),	NULL, NULL, true},

    { "menu",	tr("Reports"),            (char*)reportsMenu,	mainMenu,	true,	NULL, NULL, true},
    { "so.printSalesOrderForm", tr("Print Sales Order Form..."),	SLOT(sPrintSalesOrderForm()), reportsMenu, (_privleges->check("MaintainSalesOrders") || _privleges->check("ViewSalesOrders")),	NULL, NULL, true},

    { "menu",	tr("Master Information"), (char*)masterInfoMenu,	mainMenu,	true,	NULL, NULL, true},
    { "so.enterNewCustomer", tr("Enter New Customer..."),	SLOT(sNewCustomer()), masterInfoMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true},
    { "so.searchForCustomer", tr("Search for Customer..."),	SLOT(sSearchForCustomer()), masterInfoMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true},
    { "so.customers", tr("Customers..."),	SLOT(sCustomers()), masterInfoMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	masterInfoMenu,	true,		NULL, NULL, true},
    { "so.customerGroups", tr("Customer Groups..."),	SLOT(sCustomerGroups()), masterInfoMenu, (_privleges->check("MaintainCustomerGroups") || _privleges->check("ViewCustomerGroups")),	NULL, NULL, true},
    { "so.customerTypes", tr("Customer Types..."),	SLOT(sCustomerTypes()), masterInfoMenu, (_privleges->check("MaintainCustomerTypes") || _privleges->check("ViewCustomerTypes")),	NULL, NULL, true},
    { "so.characteristics", tr("Characteristics..."),	SLOT(sCharacteristics()), masterInfoMenu, (_privleges->check("MaintainCharacteristics") || _privleges->check("ViewCharacteristics")),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	masterInfoMenu,	true,		NULL, NULL, true},
    { "so.enterNewProspect", tr("Enter New Prospect..."),	SLOT(sNewProspect()), masterInfoMenu, _privleges->check("MaintainProspectMasters"),	NULL, NULL, true},
    { "so.searchForProspect", tr("Search for Prospect..."),	SLOT(sSearchForProspect()), masterInfoMenu, (_privleges->check("MaintainProspects") || _privleges->check("ViewProspects")),	NULL, NULL, true},
    { "so.prospects", tr("Prospects..."),	SLOT(sProspects()), masterInfoMenu, (_privleges->check("MaintainProspectMasters") || _privleges->check("ViewProspectMasters")),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	masterInfoMenu,	true,		NULL, NULL, true},
    { "so.salesReps", tr("Sales Reps..."),	SLOT(sSalesReps()), masterInfoMenu, (_privleges->check("MaintainSalesReps") || _privleges->check("ViewSalesReps")),	NULL, NULL, true},
    { "so.shippingZones", tr("Shipping Zones..."),	SLOT(sShippingZones()), masterInfoMenu, (_privleges->check("MaintainShippingZones") || _privleges->check("ViewShippingZones")),	NULL, NULL, true},
    { "so.shipVias", tr("Ship Vias..."),	SLOT(sShipVias()), masterInfoMenu, (_privleges->check("MaintainShipVias") || _privleges->check("ViewShipVias")),	NULL, NULL, true},
    { "so.shippingChargeTypes", tr("Shipping Charge Types..."),	SLOT(sShippingChargeTypes()), masterInfoMenu, (_privleges->check("MaintainShippingChargeTypes") || _privleges->check("ViewShippingChargeTypes")),	NULL, NULL, true},
    { "so.taxCodes", tr("Tax Codes..."),	SLOT(sTaxCodes()), masterInfoMenu, (_privleges->check("MaintainTaxCodes") || _privleges->check("ViewTaxCodes")),	NULL, NULL, true},
    { "so.terms", tr("Terms..."),	SLOT(sTerms()), masterInfoMenu, (_privleges->check("MaintainTerms") || _privleges->check("ViewTerms")),	NULL, NULL, true},
    { "so.shippingForms", tr("Shipping Forms..."),	SLOT(sShippingForms()), masterInfoMenu, (_privleges->check("MaintainShippingForms") || _privleges->check("ViewShippingForms")),	NULL, NULL, true},
    { "so.salesCategories", tr("Sales Categories..."),	SLOT(sSalesCategories()), masterInfoMenu, (_privleges->check("MaintainSalesCategories")) || (_privleges->check("ViewSalesCategories")),	NULL, NULL, true},
    { "separator",	NULL,	NULL,	masterInfoMenu,	true,		NULL, NULL, true},
    { "so.salesAccountAssignments", tr("Sales Account Assignments..."),	SLOT(sSalesAccountAssignments()), masterInfoMenu, (_privleges->check("MaintainSalesAccount") || _privleges->check("ViewSalesAccount")),	NULL, NULL, true},
    { "so.arAccountAssignments", tr("A/R Account Assignments..."),	SLOT(sARAccountAssignments()), masterInfoMenu, (_privleges->check("MaintainSalesAccount") || _privleges->check("ViewSalesAccount")),	NULL, NULL, true},
    { "so.customerFormAssignments", tr("Customer Form Assignments..."),	SLOT(sCustomerFormAssignments()), masterInfoMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true},

    { "menu",	tr("&Utilities"),         (char*)utilitiesMenu,	mainMenu,	true,	NULL, NULL, true},
    { "so.customerInformationExport", tr("Customer Information Export..."),	SLOT(sDspCustomerInformationExport()), utilitiesMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true},
    { "so.reassignCustomerTypeByCustomerType", tr("Reassign Customer Type by Customer Type..."),	SLOT(sReassignCustomerTypeByCustomerType()), utilitiesMenu, _privleges->check("MaintainCustomerMasters"),	NULL, NULL, true},
    { "so.updateCreditStatusByCustomer", tr("Update Credit Status by Customer..."),	SLOT(sUpdateCreditStatusByCustomer()), utilitiesMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("UpdateCustomerCreditStatus")),	NULL, NULL, true},

// START_RW
    { "so.exportCustomers", tr("Export Customers..."), SLOT(sExportCustomers()), utilitiesMenu, _privleges->check("ViewCustomerMasters"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface")}
// END_RW
  };

  for (unsigned int i = 0; i < sizeof(acts) / sizeof(acts[0]); i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == QString("separator"))
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
  parent->populateCustomMenu(mainMenu, "S/O");
  parent->menuBar()->insertItem(tr("S/&O"), mainMenu);
}

//  Orders
void moduleSO::sNewSalesOrder()
{
  salesOrder::newSalesOrder(-1);
}

void moduleSO::sOpenSalesOrders()
{
  omfgThis->handleNewWindow(new openSalesOrders());
}

void moduleSO::sRescheduleSoLineItems()
{
  rescheduleSoLineItems(parent, "", TRUE).exec();
}

void moduleSO::sPackingListBatch()
{
  omfgThis->handleNewWindow(new packingListBatch());
}

void moduleSO::sPrintPackingList()
{
  printPackingList(parent, "", TRUE).exec();
}

void moduleSO::sNewQuote()
{
  ParameterList params;
  params.append("mode", "newQuote");

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sQuotes()
{
  omfgThis->handleNewWindow(new quotes());
}


//  Billing
void moduleSO::sUninvoicedShipments()
{
  omfgThis->handleNewWindow(new uninvoicedShipments());
}

void moduleSO::sSelectShippedOrdersForBilling()
{
  selectShippedOrders(parent, "", TRUE).exec();
}

void moduleSO::sSelectOrderForBilling()
{
  ParameterList params;
  params.append("mode", "new");

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sBillingEditList()
{
  omfgThis->handleNewWindow(new billingEditList());
}

void moduleSO::sDspBillingSelections()
{
  omfgThis->handleNewWindow(new dspBillingSelections());
}

void moduleSO::sPostBillingSelections()
{
  postBillingSelections(parent, "", TRUE).exec();
}

void moduleSO::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void moduleSO::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void moduleSO::sPrintInvoicesByShipvia()
{
  printInvoicesByShipvia(parent, "", TRUE).exec();
}

void moduleSO::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void moduleSO::sDeliverInvoice()
{
  deliverInvoice(parent, "", TRUE).exec();
}

void moduleSO::sDeliverSalesOrder()
{
  deliverSalesOrder(parent, "", TRUE).exec();
}

void moduleSO::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void moduleSO::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}


void moduleSO::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sUnpostedCreditMemos()
{
  omfgThis->handleNewWindow(new unpostedCreditMemos());
}

void moduleSO::sCreditMemoEditList()
{
  omfgThis->handleNewWindow(new creditMemoEditList());
}

void moduleSO::sPrintCreditMemos()
{
  printCreditMemos(parent, "", TRUE).exec();
}

void moduleSO::sReprintCreditMemos()
{
  reprintCreditMemos(parent, "", TRUE).exec();
}

void moduleSO::sPostCreditMemos()
{
  postCreditMemos(parent, "", TRUE).exec();
}

void moduleSO::sPurgeCreditMemos()
{
  purgeCreditMemos(parent, "", TRUE).exec();
}


//  S/O | Item Pricing
void moduleSO::sItemListPrice()
{
  itemListPrice newdlg(parent, "", TRUE);
  newdlg.exec();
}

void moduleSO::sUpdateListPricesByProductCategory()
{
  updateListPricesByProductCategory newdlg(parent, "", TRUE);
  newdlg.exec();
}

void moduleSO::sPricingSchedules()
{
  omfgThis->handleNewWindow(new itemPricingSchedules());
}

void moduleSO::sPricingScheduleAssignments()
{
  omfgThis->handleNewWindow(new pricingScheduleAssignments());
}

void moduleSO::sSales()
{
  omfgThis->handleNewWindow(new sales());
}

void moduleSO::sUpdatePricesByProductCategory()
{
  updatePricesByProductCategory(parent, "", TRUE).exec();
}

void moduleSO::sUpdatePricesByPricingSchedule()
{
  updatePricesByPricingSchedule(parent, "", TRUE).exec();
}

void moduleSO::sDspPricesByItem()
{
  omfgThis->handleNewWindow(new dspPricesByItem());
}

void moduleSO::sDspPricesByCustomer()
{
  omfgThis->handleNewWindow(new dspPricesByCustomer());
}

void moduleSO::sDspPricesByCustomerType()
{
  omfgThis->handleNewWindow(new dspPricesByCustomerType());
}

void moduleSO::sDspCustomersByCusttype()
{
  omfgThis->handleNewWindow(new dspCustomersByCustomerType());
}

void moduleSO::sDspCustomersByCharacteristic()
{
  omfgThis->handleNewWindow(new dspCustomersByCharacteristic());
}

void moduleSO::sDspCustomerInformation()
{
  // see notes on Mantis bug 4024 for explanation of why this is a modal dialog
  omfgThis->handleNewWindow(new dspCustomerInformation());
}

void moduleSO::sDspSalesOrderStatus()
{
  omfgThis->handleNewWindow(new dspSalesOrderStatus());
}

void moduleSO::sDspInventoryAvailabilityByItem()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByItem());
}

void moduleSO::sDspInventoryAvailabilityBySalesOrder()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityBySalesOrder());
}

void moduleSO::sDspOrderLookupByCustomer()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByCustomer());
}

void moduleSO::sDspOrderLookupByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspSalesOrdersByParameterList *newdlg = new dspSalesOrdersByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sDspOrderLookupByItem()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByItem());
}

void moduleSO::sDspOrderLookupByCustomerPO()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByCustomerPO());
}

void moduleSO::sDspQuoteLookupByCustomer()
{
  omfgThis->handleNewWindow(new dspQuotesByCustomer());
}

void moduleSO::sDspQuoteLookupByItem()
{
  omfgThis->handleNewWindow(new dspQuotesByItem());
}

void moduleSO::sDspBacklogByItem()
{
  omfgThis->handleNewWindow(new dspBacklogByItem());
}

void moduleSO::sDspBacklogBySalesOrder()
{
  omfgThis->handleNewWindow(new dspBacklogBySalesOrder());
}

void moduleSO::sDspBacklogByCustomer()
{
  omfgThis->handleNewWindow(new dspBacklogByCustomer());
}

void moduleSO::sDspBacklogByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sDspBacklogByCustomerGroup()
{
  ParameterList params;
  params.append("custgrp");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sDspBacklogByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sDspSummarizedBacklogByWarehouse()
{
  omfgThis->handleNewWindow(new dspSummarizedBacklogByWarehouse());
}

void moduleSO::sDspPartiallyShippedOrders()
{
  omfgThis->handleNewWindow(new dspPartiallyShippedOrders());
}

void moduleSO::sDspEarnedCommissions()
{
  omfgThis->handleNewWindow(new dspEarnedCommissions());
}

void moduleSO::sDspBriefEarnedCommissions()
{
  omfgThis->handleNewWindow(new dspBriefEarnedCommissions());
}

void moduleSO::sDspSummarizedTaxableSales()
{
  omfgThis->handleNewWindow(new dspSummarizedTaxableSales());
}

void moduleSO::sPrintSalesOrderForm()
{
  printSoForm(parent, "", TRUE).exec();
}


//  Master Information
void moduleSO::sNewCustomer()
{
  ParameterList params;
  params.append("mode", "new");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sSearchForCustomer()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "cust");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void moduleSO::sUpdateCreditStatusByCustomer()
{
  updateCreditStatusByCustomer(parent, "", TRUE).exec();
}

void moduleSO::sCustomerGroups()
{
  omfgThis->handleNewWindow(new customerGroups());
}

void moduleSO::sCustomerTypes()
{
  omfgThis->handleNewWindow(new customerTypes());
}

void moduleSO::sNewProspect()
{
  ParameterList params;
  params.append("mode", "new");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sSearchForProspect()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "prospect");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleSO::sProspects()
{
  omfgThis->handleNewWindow(new prospects());
}

void moduleSO::sSalesReps()
{
  omfgThis->handleNewWindow(new salesReps());
}

void moduleSO::sShippingZones()
{
  omfgThis->handleNewWindow(new shippingZones());
}

void moduleSO::sShipVias()
{
  omfgThis->handleNewWindow(new shipVias());
}

void moduleSO::sShippingChargeTypes()
{
  omfgThis->handleNewWindow(new shippingChargeTypes());
}

void moduleSO::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void moduleSO::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void moduleSO::sShippingForms()
{
  omfgThis->handleNewWindow(new shippingForms());
}

void moduleSO::sSalesCategories()
{
  omfgThis->handleNewWindow(new salesCategories());
}

void moduleSO::sSalesAccountAssignments()
{
  omfgThis->handleNewWindow(new salesAccounts());
}

void moduleSO::sARAccountAssignments()
{
  omfgThis->handleNewWindow(new arAccountAssignments());
}

void moduleSO::sCustomerFormAssignments()
{
  omfgThis->handleNewWindow(new customerFormAssignments());
}

void moduleSO::sDspCustomerInformationExport()
{
  omfgThis->handleNewWindow(new dspCustomerInformationExport());
}

void moduleSO::sReassignCustomerTypeByCustomerType()
{
  reassignCustomerTypeByCustomerType(parent, "", TRUE).exec();
}

// START_RW
void moduleSO::sPostAROpenAndDist()
{
  if ( (_metrics->value("AccountingSystem") == "RW2000") ||
       (_metrics->value("AccountingSystem") == "RealWorld91") )
  {
    if (QMessageBox::critical( omfgThis, tr("Create New AROPEN and ARDIST Files?"),
                               tr( "Creating new Export Files will delete the previous Export Files.\n"
                                   "You should make sure that the previous Export Files have been\n"
                                   "imported into RealWorld before Proceeding.\n\n"
                                   "Are you sure that you want to Create New Export Files?" ),
                                   tr("&Yes"), tr("&No"), QString::null, 0, 1  ) != 0)
      return;

    rwInterface::exportArdist(omfgThis);
    rwInterface::exportAropen(omfgThis);
  }

  if ( (_metrics->value("AccountingSystem") == "RW2000") ||
       (_metrics->value("AccountingSystem") == "RealWorld91") )
  {
    if ( QMessageBox::information( omfgThis, tr("Mark Distributions as Posted"),
                                   tr( "New ARDIST and AROPEN files have been generated in\n"
                                       "the RealWorld directory.  You should now use the RealWorld\n"
                                       "arfu/arutil tool to import these files.  After you have\n"
                                       "successfully imported the ARDIST and AROPEN files click\n"
                                       "the 'Post' button to mark these items as distributed.\n"
                                       "If, for any reason, you were unable to post the ARDIST\n"
                                       "and AROPEN files click on the 'Do Not Post' button and\n"
                                       "Re-Post Invoices to re-create the ARDIST and AROPEN files.\n" ),
                                   tr("&Post"), tr("Do &Not Post"), QString::null, 0, 1) == 0)
      XSqlQuery().exec( "SELECT postSoGLTransactions();"
                        "SELECT postAropenItems();" );
  }
}

void moduleSO::sExportCustomers()
{
  omfgThis->handleNewWindow(new exportCustomers());
}
// END_RW

void moduleSO::sCharacteristics()
{
  omfgThis->handleNewWindow(new characteristics());
}
