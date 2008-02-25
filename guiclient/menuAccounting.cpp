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

//  menuAccounting.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

//AP
#include "purchaseOrder.h"
#include "unpostedPurchaseOrders.h"
#include "printPurchaseOrder.h"
#include "postPurchaseOrder.h"

#include "dspUninvoicedReceivings.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "openVouchers.h"
#include "voucheringEditList.h"
#include "postVouchers.h"

#include "selectPayments.h"
#include "selectedPayments.h"
#include "prepareCheckRun.h"
#include "viewCheckRun.h"
#include "miscCheck.h"
#include "printCheck.h"
#include "printChecks.h"
#include "voidChecks.h"
#include "postCheck.h"
#include "postChecks.h"

#include "unappliedAPCreditMemos.h"
#include "apOpenItem.h"

#include "dspVendorAPHistory.h"
#include "dspCheckRegister.h"
#include "dspVoucherRegister.h"
#include "dspAPOpenItemsByVendor.h"
#include "dspTimePhasedOpenAPItems.h"

// AR
#include "invoice.h"
#include "unpostedInvoices.h"
#include "printInvoices.h"
#include "reprintInvoices.h"
#include "deliverInvoice.h"
#include "postInvoices.h"
#include "purgeInvoices.h"

#include "cashReceipt.h"
#include "cashReceiptsEditList.h"
#include "postCashReceipts.h"

#include "unappliedARCreditMemos.h"
#include "arOpenItem.h"

#include "arWorkBench.h"

#include "dspCustomerARHistory.h"
#include "dspCashReceipts.h"
#include "dspARApplications.h"
#include "dspInvoiceInformation.h"
#include "dspAROpenItemsByCustomer.h"
#include "dspAROpenItems.h"
#include "dspTimePhasedOpenARItems.h"
#include "dspInvoiceRegister.h"
#include "dspDepositsRegister.h"
#include "printJournal.h"
#include "printStatementByCustomer.h"
#include "printStatementsByCustomerType.h"

// GL
#include "glTransaction.h"
#include "glSeries.h"
#include "unpostedGlSeries.h"

#include "standardJournal.h"
#include "standardJournals.h"
#include "standardJournalGroups.h"
#include "postStandardJournal.h"
#include "postStandardJournalGroup.h"
#include "dspStandardJournalHistory.h"

#include "financialLayouts.h"
#include "financialLayout.h"
#include "dspFinancialReport.h"

#include "dspGLTransactions.h"
#include "dspSummarizedGLTransactions.h"
#include "dspGLSeries.h"
#include "dspTrialBalances.h"

#include "companies.h"
#include "profitCenters.h"
#include "subaccounts.h"
#include "accountNumbers.h"
#include "subAccntTypes.h"
#include "accountingPeriods.h"
#include "accountingYearPeriods.h"
#include "taxCodes.h"
#include "taxTypes.h"
#include "taxAuthorities.h"
#include "searchForCRMAccount.h"
#include "taxSelections.h"
#include "taxRegistrations.h"

#include "reconcileBankaccount.h"
#include "bankAdjustment.h"
#include "bankAdjustmentEditList.h"
#include "bankAdjustmentTypes.h"
#include "dspBankrecHistory.h"
#include "dspSummarizedBankrecHistory.h"

#include "budgets.h"
#include "maintainBudget.h"
#include "forwardUpdateAccounts.h"
#include "duplicateAccountNumbers.h"
#include "vendors.h"
#include "termses.h"
#include "bankAccounts.h"
#include "checkFormats.h"
#include "apAccountAssignments.h"
#include "costCategories.h"
#include "expenseCategories.h"

#include "customers.h"
#include "customerType.h"
#include "customerTypes.h"
#include "vendorTypes.h"
#include "salesCategories.h"
#include "reasonCodes.h"
#include "arAccountAssignments.h"

#include "updateLateCustCreditStatus.h"

// START_RW
#include "postGLTransactionsToExternal.h"
#include "dspRWTransactions.h"
// END_RW

#include "menuAccounting.h"

menuAccounting::menuAccounting(GUIClient *Pparent) :
 QObject(Pparent, "wmModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("Accounting Tools"));
  toolBar->setObjectName("Accounting Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowGLToolbar"))
    parent->addToolBar(toolBar);


  mainMenu		= new QMenu();
  apMenu = new QMenu();
  apPurchaseMenu = new QMenu();
  apVoucherMenu = new QMenu();
  apMemosMenu = new QMenu();
  apPaymentsMenu = new QMenu();
  apFormsMenu = new QMenu();
  apReportsMenu = new QMenu();
  arMenu = new QMenu();
  arInvoicesMenu = new QMenu();
  arMemosMenu = new QMenu();
  arCashReceiptsMenu = new QMenu();
  arFormsMenu = new QMenu();
  arReportsMenu = new QMenu();
  glMenu = new QMenu();
  glEnterTransactionMenu = new QMenu();
  glStandardJournalsMenu = new QMenu();
  bankrecMenu		= new QMenu();
  financialReportsMenu	= new QMenu();
  reportsMenu		= new QMenu();
  calendarMenu      = new QMenu();
  coaMenu           = new QMenu();
  budgetMenu		= new QMenu();
  taxMenu           = new QMenu();
  masterInfoMenu	= new QMenu();
  utilitiesMenu		= new QMenu();

  actionProperties acts[] = { 
    // Accounting | Accounts Payable
    { "menu", tr("Accounts &Payable"), (char*)apMenu,	mainMenu, true,	NULL, NULL, true, NULL },
    
    // Accounting | Accaunts Payable | Purchase Orders
    { "menu", tr("Purchase &Order"), (char*)apPurchaseMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.enterPurchaseOrder", tr("&New..."), SLOT(sEnterPurchaseOrder()), apPurchaseMenu, _privleges->check("MaintainPurchaseOrders"), NULL, NULL, true , NULL },
    { "ap.listUnpostedPurchaseOrders", tr("&List Unposted..."), SLOT(sUnpostedPurchaseOrders()), apPurchaseMenu, (_privleges->check("MaintainPurchaseOrders") || _privleges->check("ViewPurchaseOrders")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPurchaseMenu, true, NULL, NULL, true, NULL },
    { "ap.postPurchaseOrder", tr("&Post..."), SLOT(sPostPurchaseOrder()), apPurchaseMenu, _privleges->check("PostPurchaseOrders"), NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable | Vouchers
    { "menu", tr("&Voucher"), (char*)apVoucherMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ar.enterNewVoucher", tr("&New..."), SLOT(sEnterVoucher()), apVoucherMenu, _privleges->check("MaintainVouchers"), NULL, NULL, true , NULL },
    { "ar.enterNewMiscVoucher", tr("New &Miscellaneous..."), SLOT(sEnterMiscVoucher()), apVoucherMenu, _privleges->check("MaintainVouchers"), NULL, NULL, true , NULL },
    { "ar.listUnpostedVouchers", tr("&List Unposted..."), SLOT(sUnpostedVouchers()), apVoucherMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")), new QPixmap(":/images/listUnpostedVouchers.png"), toolBar, true , "List Unposted Vouchers" },
    { "separator", NULL, NULL, apVoucherMenu, true, NULL, NULL, true, NULL },
    { "ar.postVouchers", tr("&Post..."), SLOT(sPostVouchers()), apVoucherMenu, _privleges->check("PostVouchers"), NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable | Memos
    { "menu", tr("&Memos"), (char*)apMemosMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.enterMiscCreditMemo", tr("&New Misc. Credit Memo..."), SLOT(sEnterMiscApCreditMemo()), apMemosMenu, _privleges->check("MaintainAPMemos"), NULL, NULL, true , NULL },
    { "ap.unapplidCreditMemo", tr("&List Unapplied Credit Memos..."), SLOT(sUnappliedApCreditMemos()), apMemosMenu, (_privleges->check("MaintainAPMemos") || _privleges->check("ViewAPMemos")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apMemosMenu, true, NULL, NULL, true, NULL },
    { "ap.enterMiscDebitMemo", tr("New &Misc. Debit Memo..."), SLOT(sEnterMiscApDebitMemo()), apMemosMenu, _privleges->check("MaintainAPMemos"), NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable |  Payments
    { "menu", tr("&Payments"), (char*)apPaymentsMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.selectPayments", tr("&Select..."), SLOT(sSelectPayments()), apPaymentsMenu, _privleges->check("MaintainPayments"), new QPixmap(":/images/selectPayments.png"), toolBar, true , "Select Payments" },
    { "ap.listSelectPayments", tr("&List Selected..."), SLOT(sSelectedPayments()), apPaymentsMenu, _privleges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, true, NULL, NULL, true, NULL },
    { "ap.prepareCheckRun", tr("&Prepare Check Run..."), SLOT(sPrepareCheckRun()), apPaymentsMenu, _privleges->check("MaintainPayments"), new QPixmap(":/images/prepareCheckRun.png"), toolBar, true , NULL },
    { "ap.createMiscCheck", tr("Create &Miscellaneous Check..."), SLOT(sCreateMiscCheck()), apPaymentsMenu, _privleges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "ap.viewCheckRun", tr("Vie&w Check Run..."), SLOT(sViewCheckRun()), apPaymentsMenu, _privleges->check("MaintainPayments"), new QPixmap(":/images/viewCheckRun.png"), toolBar, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, true, NULL, NULL, true, NULL },
    { "ap.voidCheckRun", tr("&Void Check Run..."), SLOT(sVoidCheckRun()), apPaymentsMenu, _privleges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, true, NULL, NULL, true, NULL },
    { "ap.postCheck", tr("Post &Check..."), SLOT(sPostCheck()), apPaymentsMenu, _privleges->check("PostPayments"), NULL, NULL, true , NULL },
    { "ap.postChecks", tr("P&ost Checks..."), SLOT(sPostChecks()), apPaymentsMenu, _privleges->check("PostPayments"), NULL, NULL, true , NULL },
                       
    { "separator", NULL, NULL, apMenu, true, NULL, NULL, true, NULL },
    
    // Accounting | Accaunts Payable | Forms
    { "menu", tr("&Forms"), (char*)apFormsMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.printPurchaseOrder", tr("Print Purchase &Order..."), SLOT(sPrintPurchaseOrder()), apFormsMenu, _privleges->check("PrintPurchaseOrders"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apFormsMenu, true, NULL, NULL, true, NULL },
    { "ap.printCheck", tr("Print &Check..."), SLOT(sPrintCheck()), apFormsMenu, _privleges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "ap.printCheckRun", tr("Print Check &Run..."), SLOT(sPrintCheckRun()), apFormsMenu, _privleges->check("MaintainPayments"), NULL, NULL, true , NULL },
    
    // Accounting | Accaunts Payable |  Reports
    { "menu", tr("&Reports"), (char*)apReportsMenu, apMenu, true, NULL, NULL, true, NULL },
    { "ap.uninvoicedReceipts", tr("&Uninvoiced Receipts and Returns..."), SLOT(sDspUninvoicedReceipts()), apReportsMenu, (_privleges->check("ViewUninvoicedReceipts") || _privleges->check("MaintainUninvoicedReceipts")), NULL, NULL, true , NULL },
    { "ap.voucheringEditList", tr("Un&posted Vouchers..."), SLOT(sVoucheringEditList()), apReportsMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.dspOpenAPItemsByVendor", tr("Open &Items by Vendor..."), SLOT(sDspAPOpenItemsByVendor()), apReportsMenu, _privleges->check("ViewAPOpenItems"), NULL, NULL, true , NULL },
    { "ap.dspAPAging", tr("A/P &Aging..."), SLOT(sDspTimePhasedOpenAPItems()), apReportsMenu, _privleges->check("ViewAPOpenItems"), new QPixmap(":/images/apAging.png"), toolBar, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.dspCheckRegister", tr("&Check Register..."), SLOT(sDspCheckRegister()), apReportsMenu, _privleges->check("MaintainPayments"), NULL, NULL, true , NULL },
    { "ap.dspVoucherRegister", tr("&Voucher Register..."), SLOT(sDspVoucherRegister()), apReportsMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.dspVendorHistory", tr("Vendor &History..."), SLOT(sDspVendorHistory()), apReportsMenu, _privleges->check("ViewAPOpenItems"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, true, NULL, NULL, true, NULL },
    { "ap.rptPayablesJournal", tr("Pa&yables Journal..."), SLOT(sRptPayablesJournal()), apReportsMenu, _privleges->check("PrintAPJournals"), NULL, NULL, true , NULL },
    { "ap.rptCheckJournal", tr("Check &Journal..."), SLOT(sRptCheckJournal()), apReportsMenu, _privleges->check("PrintAPJournals"), NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, apMenu, true, NULL, NULL, true, NULL },
    { "ap.vendors", tr("Ve&ndors..."), SLOT(sVendors()), apMenu, (_privleges->check("MaintainVendors") || _privleges->check("ViewVendors")), NULL, NULL, true , NULL },
    
    // Accounting | Accounts Receivable
    { "menu", tr("Accounts Recei&vable"),	(char*)arMenu,	mainMenu, true, NULL, NULL, true, NULL },
  
    // Accounting | Accounts Receivable | Invoices
    { "menu", tr("&Invoice"), (char*)arInvoicesMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.createInvoice", tr("&New..."), SLOT(sCreateInvoice()), arInvoicesMenu, _privleges->check("MaintainMiscInvoices"), NULL, NULL, true , NULL },
    { "ar.listUnpostedInvoices", tr("&List Unposted..."), SLOT(sUnpostedInvoices()), arInvoicesMenu, _privleges->check("SelectBilling"), new QPixmap(":/images/unpostedInvoices.png"), toolBar, true , "List Unposted Invoices" },
    { "separator", NULL, NULL, arInvoicesMenu, true, NULL, NULL, true, NULL },
    { "ar.postInvoices", tr("&Post..."), SLOT(sPostInvoices()), arInvoicesMenu, _privleges->check("PostMiscInvoices"), NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Memos
    { "menu", tr("&Memos"), (char*)arMemosMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.enterMiscCreditMemo", tr("&New Misc. Credit Memo..."), SLOT(sEnterMiscArCreditMemo()), arMemosMenu, _privleges->check("MaintainARMemos"), NULL, NULL, true , NULL },
    { "ar.unapplidCreditMemo", tr("&List Unapplied Credit Memos..."), SLOT(sUnappliedArCreditMemos()), arMemosMenu, (_privleges->check("MaintainARMemos") || _privleges->check("ViewARMemos")), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arMemosMenu, true, NULL, NULL, true, NULL },
    { "ar.enterMiscDebitMemo", tr("New &Misc. Debit Memo..."), SLOT(sEnterMiscArDebitMemo()), arMemosMenu, _privleges->check("MaintainARMemos"), NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Cash Receipts
    { "menu", tr("C&ash Receipt"), (char*)arCashReceiptsMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.enterCashReceipt", tr("&New..."), SLOT(sEnterCashReceipt()), arCashReceiptsMenu, _privleges->check("MaintainCashReceipts"), NULL, NULL, true , NULL },
    { "ar.cashReceiptEditList", tr("&Edit List..."), SLOT(sCashReceiptEditList()), arCashReceiptsMenu, (_privleges->check("MaintainCashReceipts") || _privleges->check("ViewCashReceipt")), new QPixmap(":/images/editCashReceipts.png"), toolBar, true , "Cash Receipt Edit List" },
    { "ar.postCashReceipts", tr("&Post..."), SLOT(sPostCashReceipts()), arCashReceiptsMenu, _privleges->check("PostCashReceipts"), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, arMenu, true, NULL, NULL, true, NULL },
    { "ar.arWorkBench", tr("A/R &Workbench..."), SLOT(sArWorkBench()), arMenu, _privleges->check("ViewAROpenItems") , new QPixmap(":/images/arWorkbench.png"), toolBar, true , NULL },

    // Accounting | Accounts Receivable | Forms
    { "menu", tr("&Forms"), (char*)arFormsMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.printInvoices", tr("Print &Invoices..."), SLOT(sPrintInvoices()), arFormsMenu, _privleges->check("PrintInvoices"), NULL, NULL, true , NULL },
    { "ar.reprintInvoices", tr("&Re-Print Invoices..."), SLOT(sReprintInvoices()), arFormsMenu, _privleges->check("PrintInvoices"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arFormsMenu, true, NULL, NULL, _metrics->boolean("EnableBatchManager"), NULL },
    { "ar.scheduleInvoiceForEmailDelivery", tr("&Schedule Invoice for Email Delivery..."), SLOT(sDeliverInvoice()), arFormsMenu, _privleges->check("PrintInvoices"), NULL, NULL, _metrics->boolean("EnableBatchManager") , NULL },
    { "separator", NULL, NULL, arFormsMenu, true, NULL, NULL, true, NULL },
    { "ar.printStatementByCustomer", tr("Print S&tatement by Customer..."), SLOT(sPrintStatementByCustomer()), arFormsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.printStatementsByCustomerType", tr("Print State&ments by Customer Type..."), SLOT(sPrintStatementsByCustomerType()), arFormsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Reports
    { "menu", tr("&Reports"), (char*)arReportsMenu,	arMenu, true,	 NULL, NULL, true, NULL },
    { "ar.dspInvoiceInformation", tr("&Invoice Information..."), SLOT(sDspInvoiceInformation()), arReportsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL },
    { "ar.dspOpenItems", tr("&Open Items..."), SLOT(sDspAROpenItems()), arReportsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspOpenItemsByCustomer", tr("Open Items by &Customer..."), SLOT(sDspAROpenItemsByCustomer()), arReportsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspARAging", tr("A/R A&ging..."), SLOT(sDspTimePhasedOpenItems()), arReportsMenu, _privleges->check("ViewAROpenItems"), new QPixmap(":/images/arAging.png"), toolBar, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL }, 
    { "ar.dspInvoiceRegister", tr("In&voice Register..."), SLOT(sDspInvoiceRegister()), arReportsMenu, _privleges->check("ViewInvoiceRegister"), NULL, NULL, true , NULL },
    { "ar.dspCashReceipts", tr("Cash &Receipts..."), SLOT(sDspCashReceipts()), arReportsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspARApplications", tr("A/R &Applications..."), SLOT(sDspARApplications()), arReportsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "ar.dspDepositsRegister", tr("&Deposits Register..."), SLOT(sDspDepositsRegister()), arReportsMenu, _privleges->check("ViewDepositsRegister"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL },
    { "ar.dspCustomerHistory", tr("Customer &History..."), SLOT(sDspCustomerHistory()), arReportsMenu, _privleges->check("ViewAROpenItems"), NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, true, NULL, NULL, true, NULL },
    { "ar.rptSalesJournal", tr("Sales &Journal..."), SLOT(sRptSalesJournal()), arReportsMenu, _privleges->check("PrintARJournals"), NULL, NULL, true , NULL },
    { "ar.rptCreditMemoJournal", tr("Credit &Memo Journal..."), SLOT(sRptCreditMemoJournal()), arReportsMenu, _privleges->check("PrintARJournals"), NULL, NULL, true , NULL },

    { "separator", NULL, NULL, arMenu, true, NULL, NULL, true, NULL },
    { "ar.customers", tr("&Customers..."), SLOT(sCustomers()), arMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")), NULL, NULL, true , NULL },
  
    // Accounting | G/L
    { "menu",		    tr("General &Ledger"),		(char*)glMenu,	mainMenu,		true,					NULL, NULL, true, NULL },
    
    // Accounting | G/L | Journals
    { "menu",		    tr("&Journal Entry"),		(char*)glEnterTransactionMenu,	glMenu,		true,					NULL, NULL, true, NULL },
    { "gl.simpleEntry",	    tr("S&imple..."),	SLOT(sSimpleEntry()),		glEnterTransactionMenu,	_privleges->check("PostJournalEntries"),NULL, NULL, true, NULL },
    { "gl.seriesEntry",     tr("&Series..."),	SLOT(sSeriesEntry()),		glEnterTransactionMenu,	_privleges->check("PostJournalEntries"),NULL, NULL, true, NULL },
    { "separator",	    NULL,				NULL,			        glEnterTransactionMenu,   true,					NULL, NULL, true, NULL },
    { "gl.unpostedEntries", tr("&List Unposted..."), SLOT(sUnpostedEntries()),	glEnterTransactionMenu,	_privleges->check("PostJournalEntries"), new QPixmap(":/images/journalEntries.png"), toolBar,  true, "List Unposted Journal Entries" },

    // Accounting | G/L | Standard Journals
    { "menu",			     tr("&Standard Journals"),		   (char*)glStandardJournalsMenu,	     glMenu,		   true,					      NULL, NULL, true, NULL },
    { "gl.enterNewStandardJournal",  tr("&New..."),  SLOT(sEnterStandardJournal()),    glStandardJournalsMenu, _privleges->check("MaintainStandardJournals"),     NULL, NULL, true, NULL },
    { "gl.listStandardJournals",     tr("&List..."),	   SLOT(sStandardJournals()),	     glStandardJournalsMenu, _privleges->check("MaintainStandardJournals"),     NULL, NULL, true, NULL },
    { "gl.listStandardJournalGroups",tr("List &Groups..."),SLOT(sStandardJournalGroups()),   glStandardJournalsMenu, _privleges->check("MaintainStandardJournalGroups"),NULL, NULL, true, NULL },
    { "separator",		     NULL,				   NULL,			     glStandardJournalsMenu, true,					      NULL, NULL, true, NULL },
    { "gl.postStandardJournal",	     tr("&Post..."),	   SLOT(sPostStandardJournal()),     glStandardJournalsMenu, _privleges->check("PostStandardJournals"),	      NULL, NULL, true, NULL },
    { "gl.postStandardJournalGroup", tr("Post G&roup..."), SLOT(sPostStandardJournalGroup()),glStandardJournalsMenu, _privleges->check("PostStandardJournalGroups"),    NULL, NULL, true, NULL },

    { "menu",			tr("&Bank Reconciliation"), 	(char*)bankrecMenu,		mainMenu,    true,						NULL, NULL, true, NULL },
    { "gl.reconcileBankaccnt",	tr("&Reconcile..."),SLOT(sReconcileBankaccount()),	bankrecMenu, _privleges->check("MaintainBankRec"), new QPixmap(":/images/bankReconciliation.png"), toolBar,  true, "Reconcile Bank Account" },
    { "separator",		NULL,				NULL,				bankrecMenu, true,						NULL, NULL, true, NULL },
    { "gl.enterAdjustment",	tr("&New Adjustment..."),	SLOT(sEnterAdjustment()),	bankrecMenu, _privleges->check("MaintainBankAdjustments"),	NULL, NULL, true, NULL },
    { "gl.adjustmentEditList",	tr("Adjustment Edit &List..."),	SLOT(sAdjustmentEditList()),	bankrecMenu, (_privleges->check("MaintainBankAdjustments") || _privleges->check("ViewBankAdjustments")),NULL, NULL, true, NULL },

    { "separator",		  NULL,					NULL,					mainMenu,		true,					       NULL, NULL, true, NULL },
    
    // Accounting | Reports
    { "menu",				tr("&Reports"),				(char*)reportsMenu,			mainMenu,      true,					NULL, NULL, true, NULL },
    { "gl.dspGLTransactions",		tr("G/L &Transactions..."),		SLOT(sDspGLTransactions()),		reportsMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "gl.dspSummarizedGLTransactions",	tr("Su&mmarized G/L Transactions..."),	SLOT(sDspSummarizedGLTransactions()),	reportsMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "gl.dspGLSeries",			tr("G/L &Series..."),			SLOT(sDspGLSeries()),			reportsMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "gl.dspStandardJournalHistory",	tr("Standard &Journal History..."),	SLOT(sDspStandardJournalHistory()),	reportsMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					reportsMenu,		true,					       NULL, NULL, true, NULL },
    { "gl.dspBankrecHistory",		tr("&Bank Rec. History"),		SLOT(sDspBankrecHistory()),		reportsMenu, _privleges->check("ViewBankRec"),		NULL, NULL, true, NULL },
    { "gl.dspSummarizedBankrecHistory",	tr("Summari&zed Bank Rec. History"),	SLOT(sDspSummarizedBankrecHistory()),	reportsMenu, _privleges->check("ViewBankRec"),		NULL, NULL, true, NULL },

    // Accounting | Statements
    { "menu",			  tr("Financial &Statements"),		(char*)financialReportsMenu,		mainMenu,			true,					       NULL, NULL, true, NULL },
    { "gl.createFinancialReports",tr("&New Financial Report..."),	SLOT(sNewFinancialReport()),		financialReportsMenu,		_privleges->check("MaintainFinancialLayouts"), NULL, NULL, true, NULL },
    { "gl.editFinancialReports",  tr("&List Financial Reports..."),	SLOT(sFinancialReports()),		financialReportsMenu,		_privleges->check("MaintainFinancialLayouts"), NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					financialReportsMenu,		true,					       NULL, NULL, true, NULL },
    { "gl.dspTrialBalances",	  tr("View &Trial Balances..."),		SLOT(sDspTrialBalances()),		financialReportsMenu,		_privleges->check("ViewTrialBalances"),	   new QPixmap(":/images/viewTrialBalance.png"), toolBar,  true, NULL },
    { "gl.viewFinancialReport",	  tr("View &Financial Report..."),	SLOT(sViewFinancialReport()),		financialReportsMenu,		_privleges->check("ViewFinancialReports"),   new QPixmap(":/images/viewFinancialReport.png"), toolBar, true, NULL },

    { "separator",		  NULL,					NULL,					mainMenu,		true,					       NULL, NULL, true, NULL },
    
    // Accounting | Fiscal Calendar
    { "menu", tr("&Fiscal Calendar"), (char*)calendarMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.accountingYearPeriods",	tr("Fiscal &Years..."),	SLOT(sAccountingYearPeriods()),	calendarMenu,	_privleges->check("MaintainAccountingPeriods"),	NULL, NULL, true, NULL },
    { "gl.accountingPeriods",	tr("Accounting &Periods..."),	SLOT(sAccountingPeriods()),	calendarMenu,	_privleges->check("MaintainAccountingPeriods"),	NULL, NULL, true, NULL },
    
    // Accounting | Account
    { "menu", tr("&Account"), (char*)coaMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.accountNumbers",	tr("&Chart of Accounts..."),	SLOT(sAccountNumbers()), coaMenu,	_privleges->check("MaintainChartOfAccounts"),	NULL, NULL, true, NULL },
    { "gl.companies",		tr("C&ompanies..."),		SLOT(sCompanies()),		coaMenu,	(_privleges->check("MaintainChartOfAccounts") && (_metrics->value("GLCompanySize").toInt() > 0)), NULL, NULL, true, NULL },
    { "gl.profitCenterNumber",	tr("&Profit Center Numbers..."),	SLOT(sProfitCenters()),	coaMenu,	(_privleges->check("MaintainChartOfAccounts") && (_metrics->value("GLProfitSize").toInt() > 0)), NULL, NULL, true, NULL },
    { "gl.subaccountNumbers",	tr("&Subaccount Numbers..."),	SLOT(sSubaccounts()), coaMenu,	(_privleges->check("MaintainChartOfAccounts") && (_metrics->value("GLSubaccountSize").toInt() > 0)), NULL, NULL, true, NULL },
    { "gl.subAccntTypes",	tr("Su&baccount Types..."),	SLOT(sSubAccntTypes()),	coaMenu,	_privleges->check("MaintainChartOfAccounts"),	NULL, NULL, true, NULL },

    // Accounting | Budget
    { "menu", tr("Bu&dget"), (char*)budgetMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.maintainBudget",	tr("&New Budget..."),	SLOT(sMaintainBudget()), budgetMenu,	_privleges->check("MaintainBudgets"),	NULL, NULL, true, NULL },
    { "gl.maintainBudget",	tr("&List Budgets..."),	SLOT(sBudgets()),	 budgetMenu,	(_privleges->check("MaintainBudgets") || _privleges->check("ViewBudgets")),	NULL, NULL, true, NULL },

    // Accounting | Tax
    { "menu", tr("&Tax"), (char*)taxMenu, mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.searchForTaxAuth",	tr("&Search for Tax Authority..."), SLOT(sTaxAuthoritySearch()),	taxMenu,	(_privleges->check("MaintainTaxAuthorities") || _privleges->check("ViewTaxAuthorities")), NULL, NULL, true, NULL },
    { "gl.taxAuthorities",	tr("Tax &Authorities..."),	SLOT(sTaxAuthorities()),	taxMenu,	(_privleges->check("MaintainTaxAuthorities") || _privleges->check("ViewTaxAuthorities")), NULL, NULL, true, NULL },
    { "gl.taxCodes",		tr("Tax &Codes..."),		SLOT(sTaxCodes()),		taxMenu,	(_privleges->check("MaintainTaxCodes") || _privleges->check("ViewTaxCodes")), NULL, NULL, true, NULL },
    { "gl.taxTypes",		tr("Tax &Types..."),		SLOT(sTaxTypes()),		taxMenu,	(_privleges->check("MaintainTaxTypes") || _privleges->check("ViewTaxTypes")), NULL, NULL, true, NULL },
    { "gl.taxSelections",	tr("Tax Se&lections..."),	SLOT(sTaxSelections()),		taxMenu,	(_privleges->check("MaintainTaxSel") || _privleges->check("ViewTaxSel")), NULL, NULL, true, NULL },
    { "gl.taxRegistatrions",	tr("Tax &Registrations..."),	SLOT(sTaxRegistrations()),	taxMenu,	_privleges->check("MaintainChartOfAccounts"),   NULL, NULL, true, NULL },
    
    { "separator",		  NULL,					NULL,					mainMenu,		true,					       NULL, NULL, true, NULL },
   
    // Accounting | Master Information
    { "menu",			tr("&Master Information"),	(char*)masterInfoMenu,		mainMenu,	true,						NULL, NULL, true, NULL },
    { "gl.postTransactionsToExternalAccountingSystem", tr("Post Transactions to External Accounting System..."), SLOT(sPostTransactionsToExternal()), utilitiesMenu, _privleges->check("ViewGLTransactions"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") , NULL },
    { "gl.dspRWTransactions",		tr("Display Exported Transactions..."),	SLOT(sDspRWTransactions()),	utilitiesMenu,	_privleges->check("ViewGLTransactions"), NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") , NULL },                               
    { "ap.terms", tr("Ter&ms..."), SLOT(sTerms()), masterInfoMenu, (_privleges->check("MaintainTerms") || _privleges->check("ViewTerms")), NULL, NULL, true , NULL },
    { "separator",			NULL,					NULL,				masterInfoMenu,	true,	NULL, NULL, true , NULL },
    { "ap.bankAccounts", tr("&Bank Accounts..."), SLOT(sBankAccounts()), masterInfoMenu, (_privleges->check("MaintainBankAccounts") || _privleges->check("ViewBankAccounts")), NULL, NULL, true , NULL },
    { "ap.checkFormats", tr("&Check Formats..."), SLOT(sCheckFormats()), masterInfoMenu, (_privleges->check("MaintainCheckFormats") || _privleges->check("ViewCheckFormats")), NULL, NULL, true , NULL },
    { "ap.costCategories", tr("C&ost Categories..."), SLOT(sCostCategories()), masterInfoMenu, (_privleges->check("MaintainCostCategories")) || (_privleges->check("ViewCostCategories")), NULL, NULL, true , NULL },
    { "ap.expenseCategories", tr("&Expense Categories..."), SLOT(sExpenseCategories()), masterInfoMenu, (_privleges->check("MaintainExpenseCategories")) || (_privleges->check("ViewExpenseCategories")), NULL, NULL, true , NULL },
    { "ap.apAccountAssignments", tr("A/&P Account Assignments..."), SLOT(sAPAssignments()), masterInfoMenu, (_privleges->check("MaintainVendorAccounts") || _privleges->check("ViewVendorAccounts")), NULL, NULL, true , NULL },
    { "separator",		  NULL,					NULL,					masterInfoMenu,		true,					       NULL, NULL, true, NULL },
    { "ar.customerTypes", tr("Customer &Types..."), SLOT(sCustomerTypes()), masterInfoMenu, (_privleges->check("MaintainCustomerTypes") || _privleges->check("ViewCustomerTypes")), NULL, NULL, true , NULL },
    { "ar.vendorTypes", tr("&Vendor Types..."), SLOT(sVendorTypes()), masterInfoMenu, (_privleges->check("MaintainVendorTypes")) || (_privleges->check("ViewVendorTypes")), NULL, NULL, true , NULL },
    { "ar.salesCategories", tr("&Sales Categories..."), SLOT(sSalesCategories()), masterInfoMenu, (_privleges->check("MaintainSalesCategories")) || (_privleges->check("ViewSalesCategories")), NULL, NULL, true , NULL },
    { "ar.arAccountAssignments", tr("A/R Account Assi&gnments..."), SLOT(sARAccountAssignments()), masterInfoMenu, (_privleges->check("MaintainSalesAccount") || _privleges->check("ViewSalesAccount")), NULL, NULL, true , NULL },
    { "ar.reasonCodes", tr("&Reason Codes..."), SLOT(sReasonCodes()), masterInfoMenu, _privleges->check("MaintainReasonCodes"), NULL, NULL, true , NULL },
    { "separator",		  NULL,					NULL,					masterInfoMenu,		true,					       NULL, NULL, true, NULL },
    { "gl.adjustmentTypes",	tr("&Adjustment Types..."),	SLOT(sAdjustmentTypes()),	masterInfoMenu,	(_privleges->check("MaintainAdjustmentTypes") || _privleges->check("ViewAdjustmentTypes")),	NULL, NULL, true, NULL },

    // Accounting | Utilities
    { "menu",				tr("&Utilities"),			(char*)utilitiesMenu,		mainMenu,	true,	NULL, NULL, true, NULL },
    { "gl.forwardUpdateAccounts",	tr("&Forward Update Accounts..."),	SLOT(sForwardUpdateAccounts()),	utilitiesMenu,	_privleges->check("ViewTrialBalances"),	NULL, NULL, true, NULL },
    { "gl.duplicateAccountNumbers",      tr("&Duplicate Account Numbers..."),  SLOT(sDuplicateAccountNumbers()), utilitiesMenu,  _privleges->check("MaintainChartOfAccounts"), NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					utilitiesMenu,		true,					       NULL, NULL, true, NULL },
    { "so.purgeInvoices", tr("Purge &Invoices..."), SLOT(sPurgeInvoices()), utilitiesMenu, _privleges->check("PurgeInvoices"), NULL, NULL, true , NULL },
    { "ar.updateLateCustCreditStatus", tr("&Update Late Customer Credit Status..."), SLOT(sUpdateLateCustCreditStatus()), utilitiesMenu, _privleges->check("UpdateCustomerCreditStatus"), NULL, NULL, _metrics->boolean("AutoCreditWarnLateCustomers"), NULL },
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));
  
  parent->populateCustomMenu(mainMenu, "Accounting");
  parent->menuBar()->insertItem(tr("&Accounting"), mainMenu);
}

void menuAccounting::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  for (unsigned int i = 0; i < numElems; i++)
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
    else if ((acts[i].toolBar != NULL) && (acts[i].toolBar != NULL))
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar,
                      acts[i].toolTip) );
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
					  acts[i].toolBar,
                      acts[i].actionTitle) );
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
}

//  Purchase Orders
void menuAccounting::sEnterPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sUnpostedPurchaseOrders()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void menuAccounting::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", TRUE).exec();
}

void menuAccounting::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", TRUE).exec();
}

//  Vouchers
void menuAccounting::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void menuAccounting::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void menuAccounting::sVoucheringEditList()
{
  omfgThis->handleNewWindow(new voucheringEditList());
}

void menuAccounting::sPostVouchers()
{
  postVouchers(parent, "", TRUE).exec();
}


//  Payments
void menuAccounting::sSelectPayments()
{
  omfgThis->handleNewWindow(new selectPayments());
}

void menuAccounting::sSelectedPayments()
{
  omfgThis->handleNewWindow(new selectedPayments());
}

void menuAccounting::sCreateMiscCheck()
{
  ParameterList params;
  params.append("new");

  miscCheck *newdlg = new miscCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sPrepareCheckRun()
{
  prepareCheckRun(parent, "", TRUE).exec();
}

void menuAccounting::sViewCheckRun()
{
  omfgThis->handleNewWindow(new viewCheckRun());
}

void menuAccounting::sPrintCheck()
{
  printCheck(parent, "", TRUE).exec();
}

void menuAccounting::sPrintCheckRun()
{
  printChecks(parent, "", TRUE).exec();
}

void menuAccounting::sVoidCheckRun()
{
  voidChecks newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuAccounting::sPostCheck()
{
  postCheck(parent, "", TRUE).exec();
}

void menuAccounting::sPostChecks()
{
  postChecks(parent, "", TRUE).exec();
}


//  Memos
void menuAccounting::sEnterMiscApCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnappliedApCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedAPCreditMemos());
}

void menuAccounting::sEnterMiscApDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


// AP Displays
void menuAccounting::sDspVendorHistory()
{
  omfgThis->handleNewWindow(new dspVendorAPHistory());
}

void menuAccounting::sDspCheckRegister()
{
  omfgThis->handleNewWindow(new dspCheckRegister());
}

void menuAccounting::sDspVoucherRegister()
{
  omfgThis->handleNewWindow(new dspVoucherRegister());
}

void menuAccounting::sDspAPOpenItemsByVendor()
{
  omfgThis->handleNewWindow(new dspAPOpenItemsByVendor());
}

void menuAccounting::sDspTimePhasedOpenAPItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenAPItems());
}

void menuAccounting::sCreateInvoice()
{
  invoice::newInvoice(-1);
}

void menuAccounting::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void menuAccounting::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sDeliverInvoice()
{
  deliverInvoice(parent, "", TRUE).exec();
}

void menuAccounting::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}



//  Cash Receipts
void menuAccounting::sEnterCashReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sCashReceiptEditList()
{
  omfgThis->handleNewWindow(new cashReceiptsEditList());
}

void menuAccounting::sPostCashReceipts()
{
  postCashReceipts(parent, "", TRUE).exec();
}


//  Memos
void menuAccounting::sEnterMiscArCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnappliedArCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedARCreditMemos());
}

void menuAccounting::sEnterMiscArDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

//  Displays
void menuAccounting::sArWorkBench()
{
  omfgThis->handleNewWindow(new arWorkBench());
}

void menuAccounting::sDspCustomerHistory()
{
  omfgThis->handleNewWindow(new dspCustomerARHistory());
}

void menuAccounting::sDspCashReceipts()
{
  omfgThis->handleNewWindow(new dspCashReceipts());
}

void menuAccounting::sDspARApplications()
{
  omfgThis->handleNewWindow(new dspARApplications());
}

void menuAccounting::sDspInvoiceInformation()
{
  omfgThis->handleNewWindow(new dspInvoiceInformation());
}

void menuAccounting::sDspInvoiceRegister()
{
  omfgThis->handleNewWindow(new dspInvoiceRegister());
}

void menuAccounting::sDspDepositsRegister()
{
  omfgThis->handleNewWindow(new dspDepositsRegister());
}

void menuAccounting::sRptSalesJournal()
{
  ParameterList params;
  params.append("type", SalesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptCreditMemoJournal()
{
  ParameterList params;
  params.append("type", CreditMemoJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptPayablesJournal()
{
  ParameterList params;
  params.append("type", PayablesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptCheckJournal()
{
  ParameterList params;
  params.append("type", CheckJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sDspAROpenItemsByCustomer()
{
  omfgThis->handleNewWindow(new dspAROpenItemsByCustomer());
}

void menuAccounting::sDspAROpenItems()
{
  omfgThis->handleNewWindow(new dspAROpenItems());
}

void menuAccounting::sDspTimePhasedOpenItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenARItems());
}


// General Ledger
void menuAccounting::sEnterStandardJournal()
{
  ParameterList params;
  params.append("mode", "new");

  standardJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sStandardJournals()
{
  omfgThis->handleNewWindow(new standardJournals());
}

void menuAccounting::sStandardJournalGroups()
{
  omfgThis->handleNewWindow(new standardJournalGroups());
}

void menuAccounting::sPostStandardJournal()
{
  postStandardJournal(parent, "", TRUE).exec();
}

void menuAccounting::sPostStandardJournalGroup()
{
  postStandardJournalGroup(parent, "", TRUE).exec();
}

void menuAccounting::sSimpleEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glTransaction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sSeriesEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glSeries newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnpostedEntries()
{
  omfgThis->handleNewWindow(new unpostedGlSeries());
}

//  Financial Layouts
void menuAccounting::sFinancialReports()
{
  omfgThis->handleNewWindow(new financialLayouts());
}

void menuAccounting::sViewFinancialReport()
{
  omfgThis->handleNewWindow(new dspFinancialReport());
}

void menuAccounting::sNewFinancialReport()
{
  ParameterList params;
  params.append("mode", "new");

  financialLayout newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


//  Displays
void menuAccounting::sDspGLTransactions()
{
  omfgThis->handleNewWindow(new dspGLTransactions());
}

void menuAccounting::sDspSummarizedGLTransactions()
{
  omfgThis->handleNewWindow(new dspSummarizedGLTransactions());
}

void menuAccounting::sDspGLSeries()
{
  omfgThis->handleNewWindow(new dspGLSeries());
}

void menuAccounting::sDspStandardJournalHistory()
{
  omfgThis->handleNewWindow(new dspStandardJournalHistory());
}

void menuAccounting::sDspTrialBalances()
{
  omfgThis->handleNewWindow(new dspTrialBalances());
}

//  Master Information
void menuAccounting::sCompanies()
{
  omfgThis->handleNewWindow(new companies());
}

void menuAccounting::sProfitCenters()
{
  omfgThis->handleNewWindow(new profitCenters());
}

void menuAccounting::sSubaccounts()
{
  omfgThis->handleNewWindow(new subaccounts());
}

void menuAccounting::sAccountNumbers()
{
  omfgThis->handleNewWindow(new accountNumbers());
}

void menuAccounting::sDuplicateAccountNumbers()
{
  omfgThis->handleNewWindow(new duplicateAccountNumbers());
}

void menuAccounting::sSubAccntTypes()
{
  omfgThis->handleNewWindow(new subAccntTypes());
}

void menuAccounting::sAccountingPeriods()
{
  omfgThis->handleNewWindow(new accountingPeriods());
}

void menuAccounting::sAccountingYearPeriods()
{
  omfgThis->handleNewWindow(new accountingYearPeriods());
}

void menuAccounting::sReconcileBankaccount()
{
  omfgThis->handleNewWindow(new reconcileBankaccount());
}

void menuAccounting::sEnterAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sAdjustmentEditList()
{
  omfgThis->handleNewWindow(new bankAdjustmentEditList());
}

void menuAccounting::sAdjustmentTypes()
{
  omfgThis->handleNewWindow(new bankAdjustmentTypes());
}

void menuAccounting::sTaxAuthorities()
{
  omfgThis->handleNewWindow(new taxAuthorities());
}

void menuAccounting::sTaxAuthoritySearch()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "taxauth");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void menuAccounting::sTaxTypes()
{
  omfgThis->handleNewWindow(new taxTypes());
}

void menuAccounting::sTaxSelections()
{
  omfgThis->handleNewWindow(new taxSelections());
}

void menuAccounting::sTaxRegistrations()
{
  omfgThis->handleNewWindow(new taxRegistrations());
}

void menuAccounting::sDspBankrecHistory()
{
  omfgThis->handleNewWindow(new dspBankrecHistory());
}

void menuAccounting::sDspSummarizedBankrecHistory()
{
  omfgThis->handleNewWindow(new dspSummarizedBankrecHistory());
}

void menuAccounting::sBudgets()
{
  omfgThis->handleNewWindow(new budgets());
}

void menuAccounting::sMaintainBudget()
{
  ParameterList params;
  params.append("mode", "new");

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sForwardUpdateAccounts()
{
  forwardUpdateAccounts(parent, "", TRUE).exec();
}

// START_RW
//  Utilities
void menuAccounting::sPostTransactionsToExternal()
{
  postGLTransactionsToExternal(parent, "", TRUE).exec();
}

void menuAccounting::sDspRWTransactions()
{
  omfgThis->handleNewWindow(new dspRWTransactions());
}
// END_RW

void menuAccounting::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void menuAccounting::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void menuAccounting::sBankAccounts()
{
  omfgThis->handleNewWindow(new bankAccounts());
}

void menuAccounting::sCheckFormats()
{
  omfgThis->handleNewWindow(new checkFormats());
}

void menuAccounting::sAPAssignments()
{
  omfgThis->handleNewWindow(new apAccountAssignments());
}

void menuAccounting::sCostCategories()
{
  omfgThis->handleNewWindow(new costCategories());
}

void menuAccounting::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}

void menuAccounting::sPrintStatementByCustomer()
{
  printStatementByCustomer(parent, "", TRUE).exec();
}

void menuAccounting::sPrintStatementsByCustomerType()
{
  printStatementsByCustomerType(parent, "", TRUE).exec();
}

void menuAccounting::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void menuAccounting::sCustomerTypes()
{
  omfgThis->handleNewWindow(new customerTypes());
}

void menuAccounting::sVendorTypes()
{
  omfgThis->handleNewWindow(new vendorTypes());
}

void menuAccounting::sSalesCategories()
{
  omfgThis->handleNewWindow(new salesCategories());
}

void menuAccounting::sReasonCodes()
{
  omfgThis->handleNewWindow(new reasonCodes());
}

void menuAccounting::sARAccountAssignments()
{
  omfgThis->handleNewWindow(new arAccountAssignments());
}

void menuAccounting::sUpdateLateCustCreditStatus()
{
  updateLateCustCreditStatus newdlg(parent, "", TRUE);
  newdlg.exec();
}

