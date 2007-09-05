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

//  moduleAP.cpp
//  Created 08/22/2002 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

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
#include "prepareAPCheckRun.h"
#include "viewAPCheckRun.h"
#include "miscAPCheck.h"
#include "printAPCheck.h"
#include "printAPChecks.h"
#include "voidAPChecks.h"
#include "postAPCheck.h"
#include "postAPChecks.h"

#include "unappliedAPCreditMemos.h"
#include "apOpenItem.h"

#include "dspVendorAPHistory.h"
#include "dspCheckRegister.h"
#include "dspVoucherRegister.h"
#include "dspAPOpenItemsByVendor.h"
#include "dspTimePhasedOpenAPItems.h"
//#include "dspAPAging.h"

#include "rptVoucherRegister.h"
#include "rptVendorAPHistory.h"
#include "printJournal.h"

#include "vendors.h"
#include "termses.h"
#include "bankAccounts.h"
#include "checkFormats.h"
#include "apAccountAssignments.h"
#include "costCategories.h"
#include "expenseCategories.h"

#include "moduleAP.h"

moduleAP::moduleAP(OpenMFGGUIClient *Pparent) :
 QObject(Pparent, "apModule")
{
  parent = Pparent;
  
  toolBar = new QToolBar(tr("A/P Tools"));
  toolBar->setObjectName("A/P Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowAPToolbar"))
    parent->addToolBar(toolBar);

//  Purchase Orders
  posMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.enterPurchaseOrder", tr("Enter Purchase Order..."),
                                      this, SLOT(sEnterPurchaseOrder()),
                                      posMenu, _privleges->check("MaintainPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "ap.listUnpostedPurchaseOrders", tr("List Unposted Purchase Orders..."),
                                      this, SLOT(sUnpostedPurchaseOrders()),
                                      posMenu, (_privleges->check("MaintainPurchaseOrders") || _privleges->check("ViewPurchaseOrders")) ) );

  posMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.printPurchaseOrder", tr("Print Purchase Order..."),
                                      this, SLOT(sPrintPurchaseOrder()),
                                      posMenu, _privleges->check("PrintPurchaseOrders") ) );

  parent->actions.append( new Action( parent, "ap.postPurchaseOrder", tr("Post Purchase Order..."),
                                      this, SLOT(sPostPurchaseOrder()),
                                      posMenu, _privleges->check("PostPurchaseOrders") ) );

//  P/O | Vouchers
  vouchersMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.uninvoicedReceipts", tr("Uninvoiced Receipts and Returns..."),
                                      this, SLOT(sDspUninvoicedReceipts()),
                                      vouchersMenu, (_privleges->check("ViewUninvoicedReceipts") || _privleges->check("MaintainUninvoicedReceipts")) ) );

  vouchersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.enterNewVoucher", tr("Enter New Voucher..."),
                                      this, SLOT(sEnterVoucher()),
                                      vouchersMenu, _privleges->check("MaintainVouchers") ) );

  parent->actions.append( new Action( parent, "ap.enterNewMiscVoucher", tr("Enter New Miscellaneous Voucher..."),
                                      this, SLOT(sEnterMiscVoucher()),
                                      vouchersMenu, _privleges->check("MaintainVouchers") ) );

  parent->actions.append( new Action( parent, "ap.listUnpostedVouchers", tr("List Unposted Vouchers..."),
                                      this, SLOT(sUnpostedVouchers()),
                                      vouchersMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")),
									  QPixmap(":/images/listUnpostedVouchers.png"), toolBar ) );

  vouchersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.voucheringEditList", tr("Vouchering Edit List..."),
                                      this, SLOT(sVoucheringEditList()),
                                      vouchersMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")) ) );

  vouchersMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.postVouchers", tr("Post Vouchers..."),
                                      this, SLOT(sPostVouchers()),
                                      vouchersMenu, _privleges->check("PostVouchers") ) );


//  Payments
  paymentsMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.selectPayments", tr("Select Payments..."),
                                      this, SLOT(sSelectPayments()),
                                      paymentsMenu, _privleges->check("MaintainPayments"),
                                      QPixmap(":/images/selectPayments.png"), toolBar ) );

  parent->actions.append( new Action( parent, "ap.listSelectPayments", tr("List Selected Payments..."),
                                      this, SLOT(sSelectedPayments()),
                                      paymentsMenu, _privleges->check("MaintainPayments") ) );

  paymentsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.prepareCheckRun", tr("Prepare Check Run..."),
                                      this, SLOT(sPrepareCheckRun()),
                                      paymentsMenu, _privleges->check("MaintainPayments"),
                                      QPixmap(":/images/prepareCheckRun.png"), toolBar ) );

  parent->actions.append( new Action( parent, "ap.createMiscCheck", tr("Create Miscellaneous Check..."),
                                      this, SLOT(sCreateMiscCheck()),
                                      paymentsMenu, _privleges->check("MaintainPayments") ) );

  parent->actions.append( new Action( parent, "ap.viewCheckRun", tr("View Check Run..."),
                                      this, SLOT(sViewCheckRun()),
                                      paymentsMenu, _privleges->check("MaintainPayments"),
                                      QPixmap(":/images/viewCheckRun.png"), toolBar ) );

  paymentsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.printAPCheck", tr("Print A/P Check..."),
                                      this, SLOT(sPrintCheck()),
                                      paymentsMenu, _privleges->check("MaintainPayments") ) );

  parent->actions.append( new Action( parent, "ap.printCheckRun", tr("Print Check Run..."),
                                      this, SLOT(sPrintCheckRun()),
                                      paymentsMenu, _privleges->check("MaintainPayments") ) );

  paymentsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.voidCheckRun", tr("Void Check Run..."),
                                      this, SLOT(sVoidCheckRun()),
                                      paymentsMenu, _privleges->check("MaintainPayments") ) );

  paymentsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.postAPCheck", tr("Post A/P Check..."),
                                      this, SLOT(sPostCheck()),
                                      paymentsMenu, _privleges->check("PostPayments") ) );

  parent->actions.append( new Action( parent, "ap.postAPChecks", tr("Post A/P Checks..."),
                                      this, SLOT(sPostChecks()),
                                      paymentsMenu, _privleges->check("PostPayments") ) );


//  Memos
  memosMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.enterMiscCreditMemo", tr("Enter Misc. Credit Memo..."),
                                      this, SLOT(sEnterMiscCreditMemo()),
                                      memosMenu, _privleges->check("MaintainAPMemos") ) );

  parent->actions.append( new Action( parent, "ap.unapplidCreditMemo", tr("List Unapplied Credit Memos..."),
                                      this, SLOT(sUnappliedCreditMemos()),
                                      memosMenu, (_privleges->check("MaintainAPMemos") || _privleges->check("ViewAPMemos")) ) );

  memosMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.enterMiscDebitMemo", tr("Enter Misc. Debit Memo..."),
                                      this, SLOT(sEnterMiscDebitMemo()),
                                      memosMenu, _privleges->check("MaintainAPMemos") ) );


//  Displays
  displaysMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.dspVendorHistory", tr("Vendor History..."),
                                      this, SLOT(sDspVendorHistory()),
                                      displaysMenu, _privleges->check("ViewAPOpenItems") ) );

  parent->actions.append( new Action( parent, "ap.dspCheckRegister", tr("Check Register..."),
                                      this, SLOT(sDspCheckRegister()),
                                      displaysMenu, _privleges->check("MaintainPayments") ) );

  parent->actions.append( new Action( parent, "ap.dspVoucherRegister", tr("Voucher Register..."),
                                      this, SLOT(sDspVoucherRegister()),
                                      displaysMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")) ) );

  displaysMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.dspOpenAPItemsByVendor", tr("Open Items by Vendor..."),
                                      this, SLOT(sDspAPOpenItemsByVendor()),
                                      displaysMenu, _privleges->check("ViewAPOpenItems") ) );
/*
  parent->actions.append( new Action( parent, "ap.dspTimePhasedOpenAPItems", tr("Time-Phased Open Items..."),
                                      this, SLOT(sDspTimePhasedOpenAPItems()),
                                      displaysMenu, _privleges->check("ViewAPOpenItems") ) );
*/
  parent->actions.append( new Action( parent, "ap.dspAPAging", tr("A/P Aging..."),
                                      this, SLOT(sDspTimePhasedOpenAPItems()),
                                      displaysMenu, _privleges->check("ViewAPOpenItems"),
                                      QPixmap(":/images/apAging.png"), toolBar ) );


//  Reports
  reportsMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.rptVendorHistory", tr("Vendor History..."),
                                      this, SLOT(sRptVendorHistory()),
                                      reportsMenu, _privleges->check("ViewAPOpenItems") ) );

  parent->actions.append( new Action( parent, "ap.rptVoucherRegister", tr("Voucher Register..."),
                                      this, SLOT(sRptVoucherRegister()),
                                      reportsMenu, (_privleges->check("MaintainVouchers") || _privleges->check("ViewVouchers")) ) );

  reportsMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.rptPayablesJournal", tr("Payables Journal..."),
                                      this, SLOT(sRptPayablesJournal()),
                                      reportsMenu, _privleges->check("PrintAPJournals") ) );

  parent->actions.append( new Action( parent, "ap.rptCheckJournal", tr("Check Journal..."),
                                      this, SLOT(sRptCheckJournal()),
                                      reportsMenu, _privleges->check("PrintAPJournals") ) );

//  Master Information
  masterInfoMenu = new QMenu();

  parent->actions.append( new Action( parent, "ap.terms", tr("Terms..."),
                                      this, SLOT(sTerms()),
                                      masterInfoMenu, (_privleges->check("MaintainTerms") || _privleges->check("ViewTerms")) ) );

  parent->actions.append( new Action( parent, "ap.vendors", tr("Vendors..."),
                                      this, SLOT(sVendors()),
                                      masterInfoMenu, (_privleges->check("MaintainVendors") || _privleges->check("ViewVendors")) ) );

  parent->actions.append( new Action( parent, "ap.bankAccounts", tr("Bank Accounts..."),
                                      this, SLOT(sBankAccounts()),
                                      masterInfoMenu, (_privleges->check("MaintainBankAccounts") || _privleges->check("ViewBankAccounts")) ) );

  parent->actions.append( new Action( parent, "ap.checkFormats", tr("Check Formats..."),
                                      this, SLOT(sCheckFormats()),
                                      masterInfoMenu, (_privleges->check("MaintainCheckFormats") || _privleges->check("ViewCheckFormats")) ) );

  masterInfoMenu->insertSeparator();

  parent->actions.append( new Action( parent, "ap.apAccountAssignments", tr("A/P Account Assignments..."),
                                      this, SLOT(sAPAssignments()),
                                      masterInfoMenu, (_privleges->check("MaintainVendorAccounts") || _privleges->check("ViewVendorAccounts")) ) );

  parent->actions.append( new Action( parent, "ap.costCategories", tr("Cost Categories..."),
                                      this, SLOT(sCostCategories()),
                                      masterInfoMenu, (_privleges->check("MaintainCostCategories")) || (_privleges->check("ViewCostCategories")) ) );

  parent->actions.append( new Action( parent, "ap.expenseCategories", tr("Expense Categories..."),
                                      this, SLOT(sExpenseCategories()),
                                      masterInfoMenu, (_privleges->check("MaintainExpenseCategories")) || (_privleges->check("ViewExpenseCategories")) ) );

  mainMenu = new QMenu();
  mainMenu->insertItem(tr("Purchase Orders"),           posMenu        );
  mainMenu->insertItem(tr("Vouchers"),                  vouchersMenu   );
  mainMenu->insertItem(tr("Payments"),                  paymentsMenu   );
  mainMenu->insertItem(tr("Debit and Credit Memos..."), memosMenu      );
  mainMenu->insertItem(tr("Displays"),                  displaysMenu   );
  mainMenu->insertItem(tr("Reports"),                   reportsMenu    );
  mainMenu->insertItem(tr("Master Information"),        masterInfoMenu );
  parent->populateCustomMenu(mainMenu, "A/P");
  parent->menuBar()->insertItem(tr("A/P"), mainMenu);
}


//  Purchase Orders
void moduleAP::sEnterPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleAP::sUnpostedPurchaseOrders()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void moduleAP::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", TRUE).exec();
}

void moduleAP::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", TRUE).exec();
}

//  Vouchers
void moduleAP::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void moduleAP::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleAP::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleAP::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void moduleAP::sVoucheringEditList()
{
  omfgThis->handleNewWindow(new voucheringEditList());
}

void moduleAP::sPostVouchers()
{
  postVouchers(parent, "", TRUE).exec();
}


//  Payments
void moduleAP::sSelectPayments()
{
  omfgThis->handleNewWindow(new selectPayments());
}

void moduleAP::sSelectedPayments()
{
  omfgThis->handleNewWindow(new selectedPayments());
}

void moduleAP::sCreateMiscCheck()
{
  ParameterList params;
  params.append("new");

  miscAPCheck *newdlg = new miscAPCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleAP::sPrepareCheckRun()
{
  prepareAPCheckRun(parent, "", TRUE).exec();
}

void moduleAP::sViewCheckRun()
{
  omfgThis->handleNewWindow(new viewAPCheckRun());
}

void moduleAP::sPrintCheck()
{
  printAPCheck(parent, "", TRUE).exec();
}

void moduleAP::sPrintCheckRun()
{
  printAPChecks(parent, "", TRUE).exec();
}

void moduleAP::sVoidCheckRun()
{
  voidAPChecks newdlg(parent, "", TRUE);
  newdlg.exec();
}

void moduleAP::sPostCheck()
{
  postAPCheck(parent, "", TRUE).exec();
}

void moduleAP::sPostChecks()
{
  postAPChecks(parent, "", TRUE).exec();
}


//  Memos
void moduleAP::sEnterMiscCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleAP::sUnappliedCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedAPCreditMemos());
}

void moduleAP::sEnterMiscDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


//  Displays
void moduleAP::sDspVendorHistory()
{
  omfgThis->handleNewWindow(new dspVendorAPHistory());
}

void moduleAP::sDspCheckRegister()
{
  omfgThis->handleNewWindow(new dspCheckRegister());
}

void moduleAP::sDspVoucherRegister()
{
  omfgThis->handleNewWindow(new dspVoucherRegister());
}

void moduleAP::sDspAPOpenItemsByVendor()
{
  omfgThis->handleNewWindow(new dspAPOpenItemsByVendor());
}

void moduleAP::sDspTimePhasedOpenAPItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenAPItems());
}

/*
void moduleAP::sDspAPAging()
{
  omfgThis->handleNewWindow(new dspAPAging());
}
*/

//  Reports
void moduleAP::sRptVendorHistory()
{
  rptVendorAPHistory(parent, "", TRUE).exec();
}

void moduleAP::sRptVoucherRegister()
{
  rptVoucherRegister(parent, "", TRUE).exec();
}

void moduleAP::sRptPayablesJournal()
{
  ParameterList params;
  params.append("type", PayablesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleAP::sRptCheckJournal()
{
  ParameterList params;
  params.append("type", CheckJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

//  Master Information
void moduleAP::sTerms()
{
  omfgThis->handleNewWindow(new termses());
}

void moduleAP::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void moduleAP::sBankAccounts()
{
  omfgThis->handleNewWindow(new bankAccounts());
}

void moduleAP::sCheckFormats()
{
  omfgThis->handleNewWindow(new checkFormats());
}

void moduleAP::sAPAssignments()
{
  omfgThis->handleNewWindow(new apAccountAssignments());
}

void moduleAP::sCostCategories()
{
  omfgThis->handleNewWindow(new costCategories());
}

void moduleAP::sExpenseCategories()
{
  omfgThis->handleNewWindow(new expenseCategories());
}



