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

//  moduleAR.cpp
//  Created 08/22/2001 JSL
//  Copyright (c) 2001-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

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
//#include "dspARAging.h"
#include "dspInvoiceRegister.h"
#include "dspDepositsRegister.h"

#include "rptCustomerARHistory.h"
#include "rptCashReceipts.h"
#include "rptARApplications.h"
#include "rptAROpenItemsByCustomer.h"
#include "rptAROpenItems.h"
#include "printJournal.h"
#include "rptInvoiceRegister.h"
#include "rptDepositsRegister.h"

#include "printStatementByCustomer.h"
#include "printStatementsByCustomerType.h"

#include "customers.h"
#include "customerType.h"
#include "customerTypes.h"
#include "taxCodes.h"
#include "termses.h"
#include "bankAccounts.h"
#include "salesCategories.h"
#include "reasonCodes.h"
#include "arAccountAssignments.h"
 
#include "moduleAR.h"

moduleAR::moduleAR(OpenMFGGUIClient *Pparent) :
 QObject(Pparent, "arModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("A/R Tools"));
  toolBar->setObjectName("A/R Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowARToolbar"))
    parent->addToolBar(toolBar);

//  Invoices
  invoicesMenu = new QMenu();

  parent->actions.append( new Action( parent, "ar.createInvoice", tr("Create Invoice..."),
                                      this, SLOT(sCreateInvoice()),
                                      invoicesMenu, _privleges->check("MaintainMiscInvoices") ) );

  parent->actions.append( new Action( parent, "ar.listUnpostedInvoices", tr("List Unposted Invoices..."),
                                      this, SLOT(sUnpostedInvoices()),
                                      invoicesMenu, _privleges->check("SelectBilling"),
									  QPixmap(":/images/unpostedInvoices.png"), toolBar ) );

  invoicesMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.arWorkBench", tr("A/R Workbench..."),
                                      this, SLOT(sArWorkBench()),
                                      invoicesMenu, _privleges->check("ViewAROpenItems") ,
									  QPixmap(":/images/arWorkbench.png"), toolBar ) );
  
  invoicesMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.printInvoices", tr("Print Invoices..."),
                                      this, SLOT(sPrintInvoices()),
                                      invoicesMenu, _privleges->check("PrintInvoices") ) );

  parent->actions.append( new Action( parent, "ar.reprintInvoices", tr("Re-Print Invoices..."),
                                      this, SLOT(sReprintInvoices()),
                                      invoicesMenu, _privleges->check("PrintInvoices") ) );

  if   (_metrics->boolean("EnableBatchManager"))
      parent->actions.append( new Action( parent, "ar.scheduleInvoiceForEmailDelivery", tr("Schedule Invoice for Email Delivery..."),
                                          this, SLOT(sDeliverInvoice()),
                                          invoicesMenu, _privleges->check("PrintInvoices")  ) );

  invoicesMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.postInvoices", tr("Post Invoices..."),
                                      this, SLOT(sPostInvoices()),
                                      invoicesMenu, _privleges->check("PostMiscInvoices") ) );

  invoicesMenu->insertSeparator();

  parent->actions.append( new Action( parent, "so.purgeInvoices", tr("Purge Invoices..."),
                                      this, SLOT(sPurgeInvoices()),
                                      invoicesMenu, _privleges->check("PurgeInvoices") ) );



//  Cash Receipts
  cashReceiptsMenu = new QMenu();

  parent->actions.append( new Action( parent, "ar.enterCashReceipt", tr("Enter Cash Receipt..."),
                                      this, SLOT(sEnterCashReceipt()),
                                      cashReceiptsMenu, _privleges->check("MaintainCashReceipts") ) );

  parent->actions.append( new Action( parent, "ar.cashReceiptEditList", tr("Cash Receipt Edit List..."),
                                      this, SLOT(sCashReceiptEditList()),
                                      cashReceiptsMenu, (_privleges->check("MaintainCashReceipts") || _privleges->check("ViewCashReceipt")),
									  QPixmap(":/images/editCashReceipts.png"), toolBar ) );

  parent->actions.append( new Action( parent, "ar.postCashReceipts", tr("Post Cash Receipts..."),
                                      this, SLOT(sPostCashReceipts()),
                                      cashReceiptsMenu, _privleges->check("PostCashReceipts") ) );



//  Memos
  memosMenu = new QMenu();

  parent->actions.append( new Action( parent, "ar.enterMiscCreditMemo", tr("Enter Misc. Credit Memo..."),
                                      this, SLOT(sEnterMiscCreditMemo()),
                                      memosMenu, _privleges->check("MaintainARMemos") ) );

  parent->actions.append( new Action( parent, "ar.unapplidCreditMemo", tr("List Unapplied Credit Memos..."),
                                      this, SLOT(sUnappliedCreditMemos()),
                                      memosMenu, (_privleges->check("MaintainARMemos") || _privleges->check("ViewARMemos")) ) );

  memosMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.enterMiscDebitMemo", tr("Enter Misc. Debit Memo..."),
                                      this, SLOT(sEnterMiscDebitMemo()),
                                      memosMenu, _privleges->check("MaintainARMemos") ) );

//  Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "ar.dspCustomerHistory", tr("Customer History..."),
                                      this, SLOT(sDspCustomerHistory()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.dspCashReceipts", tr("Cash Receipts by Customer..."),
                                      this, SLOT(sDspCashReceipts()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.dspARApplications", tr("A/R Applications..."),
                                      this, SLOT(sDspARApplications()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.dspInvoiceInformation", tr("Invoice Information..."),
                                      this, SLOT(sDspInvoiceInformation()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.dspInvoiceRegister", tr("Invoice Register..."),
                                      this, SLOT(sDspInvoiceRegister()),
                                      displaysMenu, _privleges->check("ViewInvoiceRegister") ) );

  parent->actions.append( new Action( parent, "ar.dspDepositsRegister", tr("Deposits Register..."),
                                      this, SLOT(sDspDepositsRegister()),
                                      displaysMenu, _privleges->check("ViewDepositsRegister") ) );


  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.dspOpenItems", tr("Open Items..."),
                                      this, SLOT(sDspAROpenItems()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.dspOpenItemsByCustomer", tr("Open Items by Customer..."),
                                      this, SLOT(sDspAROpenItemsByCustomer()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );
/*
  parent->actions.append( new Action( parent, "ar.dspTimePhasedOpenItems", tr("Time-Phased Open Items..."),
                                      this, SLOT(sDspTimePhasedOpenItems()),
                                      displaysMenu, _privleges->check("ViewAROpenItems") ) );
*/
  parent->actions.append( new Action( parent, "ar.dspARAging", tr("A/R Aging..."),
                                      this, SLOT(sDspTimePhasedOpenItems()),
                                      displaysMenu, _privleges->check("ViewAROpenItems"),
									  QPixmap(":/images/arAging.png"), toolBar ) );

//  Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "ar.rptCustomerHistory", tr("Customer History..."),
                                      this, SLOT(sRptCustomerHistory()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.rptCashReceipts", tr("Cash Receipts by Customer..."),
                                      this, SLOT(sRptCashReceipts()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.rptARApplications", tr("A/R Applications..."),
                                      this, SLOT(sRptARApplications()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.rptInvoiceInformation", tr("Invoice Information..."),
                                      this, SLOT(sRptInvoiceInformation()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.rptInvoiceRegister", tr("Invoice Register..."),
                                      this, SLOT(sRptInvoiceRegister()),
                                      reportsMenu, _privleges->check("ViewInvoiceRegister") ) );

  parent->actions.append( new Action( parent, "ar.rptDepositsRegister", tr("Deposits Register..."),
                                      this, SLOT(sRptDepositsRegister()),
                                      reportsMenu, _privleges->check("ViewDepositsRegister") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.rptOpenItems", tr("Open Items..."),
                                      this, SLOT(sRptAROpenItems()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.rptOpenItemsByCustomer", tr("Open Items by Customer..."),
                                      this, SLOT(sRptAROpenItemsByCustomer()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.printStatementByCustomer", tr("Print Statement by Customer..."),
                                      this, SLOT(sPrintStatementByCustomer()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );

  parent->actions.append( new Action( parent, "ar.printStatementsByCustomerType", tr("Print Statements by Customer Type..."),
                                      this, SLOT(sPrintStatementsByCustomerType()),
                                      reportsMenu, _privleges->check("ViewAROpenItems") ) );
  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ar.rptSalesJournal", tr("Sales Journal..."),
                                      this, SLOT(sRptSalesJournal()),
                                      reportsMenu, _privleges->check("PrintARJournals") ) );

  parent->actions.append( new Action( parent, "ar.rptCreditMemoJournal", tr("Credit Memo Journal..."),
                                      this, SLOT(sRptCreditMemoJournal()),
                                      reportsMenu, _privleges->check("PrintARJournals") ) );


//  Master Information
  masterInfoMenu = new QMenu();

  parent->actions.append( new Action( parent, "ar.customers", tr("Customers..."),
                                      this, SLOT(sCustomers()),
                                      masterInfoMenu, (_privleges->check("MaintainCustomerMasters") || _privleges->check("ViewCustomerMasters")) ) );

  parent->actions.append( new Action( parent, "ar.customerTypes", tr("Customer Types..."),
                                      this, SLOT(sCustomerTypes()),
                                      masterInfoMenu, (_privleges->check("MaintainCustomerTypes") || _privleges->check("ViewCustomerTypes")) ) );

  parent->actions.append( new Action( parent, "ar.taxCodes", tr("Tax Codes..."),
                                      this, SLOT(sTaxCodes()),
                                      masterInfoMenu, (_privleges->check("MaintainTaxCodes") || _privleges->check("ViewTaxCodes")) ) );

  parent->actions.append( new Action( parent, "ar.terms", tr("Terms..."),
                                      this, SLOT(sTerms()),
                                      masterInfoMenu, (_privleges->check("MaintainTerms") || _privleges->check("ViewTerms")) ) );

  parent->actions.append( new Action( parent, "ar.backAccounts", tr("Bank Accounts..."),
                                      this, SLOT(sBankAccounts()),
                                      masterInfoMenu, (_privleges->check("MaintainBankAccounts") || _privleges->check("ViewBankAccounts")) ) );

  parent->actions.append( new Action( parent, "ar.salesCategories", tr("Sales Categories..."),
                                      this, SLOT(sSalesCategories()),
                                      masterInfoMenu, (_privleges->check("MaintainSalesCategories")) || (_privleges->check("ViewSalesCategories")) ) );

  parent->actions.append( new Action( parent, "ar.reasonCodes", tr("Reason Codes..."),
                                      this, SLOT(sReasonCodes()),
                                      masterInfoMenu, _privleges->check("MaintainReasonCodes") ) );

  parent->actions.append( new Action( parent, "ar.arAccountAssignments", tr("A/R Account Assignments..."),
                                      this, SLOT(sARAccountAssignments()),
                                      masterInfoMenu, (_privleges->check("MaintainSalesAccount") || _privleges->check("ViewSalesAccount")) ) );

  mainMenu = new QMenu();
  mainMenu->insertItem(tr("Invoices"), invoicesMenu);
  mainMenu->insertItem(tr("Cash Receipts..."), cashReceiptsMenu);
  mainMenu->insertItem(tr("Debit and Credit Memos..."), memosMenu);
  mainMenu->insertItem(tr("Displays"), displaysMenu);
  mainMenu->insertItem(tr("Reports"), reportsMenu);
  mainMenu->insertItem(tr("Master Information"), masterInfoMenu);
  parent->populateCustomMenu(mainMenu, "A/R");
  parent->menuBar()->insertItem(tr("A/R"), mainMenu);
}

void moduleAR::sCreateInvoice()
{
  invoice::newInvoice(-1);
}

void moduleAR::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void moduleAR::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void moduleAR::sDeliverInvoice()
{
  deliverInvoice(parent, "", TRUE).exec();
}

void moduleAR::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void moduleAR::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void moduleAR::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}



//  Cash Receipts
void moduleAR::sEnterCashReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleAR::sCashReceiptEditList()
{
  omfgThis->handleNewWindow(new cashReceiptsEditList());
}

void moduleAR::sPostCashReceipts()
{
  postCashReceipts(parent, "", TRUE).exec();
}


//  Memos
void moduleAR::sEnterMiscCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleAR::sUnappliedCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedARCreditMemos());
}

void moduleAR::sEnterMiscDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


//  Displays
void moduleAR::sArWorkBench()
{
  omfgThis->handleNewWindow(new arWorkBench());
}

void moduleAR::sDspCustomerHistory()
{
  omfgThis->handleNewWindow(new dspCustomerARHistory());
}

void moduleAR::sDspCashReceipts()
{
  omfgThis->handleNewWindow(new dspCashReceipts());
}

void moduleAR::sDspARApplications()
{
  omfgThis->handleNewWindow(new dspARApplications());
}

void moduleAR::sDspInvoiceInformation()
{
  omfgThis->handleNewWindow(new dspInvoiceInformation());
}

void moduleAR::sDspInvoiceRegister()
{
  omfgThis->handleNewWindow(new dspInvoiceRegister());
}

void moduleAR::sDspDepositsRegister()
{
  omfgThis->handleNewWindow(new dspDepositsRegister());
}

void moduleAR::sDspAROpenItemsByCustomer()
{
  omfgThis->handleNewWindow(new dspAROpenItemsByCustomer());
}

void moduleAR::sDspAROpenItems()
{
  omfgThis->handleNewWindow(new dspAROpenItems());
}

void moduleAR::sDspTimePhasedOpenItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenARItems());
}

/*
void moduleAR::sDspARAging()
{
  omfgThis->handleNewWindow(new dspARAging());
}
*/

//  Reports
void moduleAR::sRptCustomerHistory()
{
  rptCustomerARHistory(parent, "", TRUE).exec();
}

void moduleAR::sRptCashReceipts()
{
  rptCashReceipts(parent, "", TRUE).exec();
}

void moduleAR::sRptARApplications()
{
  rptARApplications(parent, "", TRUE).exec();
}

void moduleAR::sRptInvoiceInformation()
{
  omfgThis->handleNewWindow(new dspInvoiceInformation());
}

void moduleAR::sRptInvoiceRegister()
{
  rptInvoiceRegister(parent, "", TRUE).exec();
}

void moduleAR::sRptDepositsRegister()
{
  rptDepositsRegister(parent, "", TRUE).exec();
}

void moduleAR::sRptSalesJournal()
{
  ParameterList params;
  params.append("type", SalesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleAR::sRptCreditMemoJournal()
{
  ParameterList params;
  params.append("type", CreditMemoJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleAR::sRptAROpenItemsByCustomer()
{
  rptAROpenItemsByCustomer(parent, "", TRUE).exec();
}

void moduleAR::sRptAROpenItems()
{
  rptAROpenItems(parent, "", TRUE).exec();
}

void moduleAR::sPrintStatementByCustomer()
{
  printStatementByCustomer(parent, "", TRUE).exec();
}

void moduleAR::sPrintStatementsByCustomerType()
{
  printStatementsByCustomerType(parent, "", TRUE).exec();
}


//  Master Information
void moduleAR::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void moduleAR::sCustomerTypes()
{
  omfgThis->handleNewWindow(new customerTypes());
}

void moduleAR::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void moduleAR::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void moduleAR::sBankAccounts()
{
  omfgThis->handleNewWindow(new bankAccounts());
}

void moduleAR::sSalesCategories()
{
  omfgThis->handleNewWindow(new salesCategories());
}

void moduleAR::sReasonCodes()
{
  omfgThis->handleNewWindow(new reasonCodes());
}

void moduleAR::sARAccountAssignments()
{
  omfgThis->handleNewWindow(new arAccountAssignments());
}

