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

//  moduleAR.h
//  Created 08/22/2001 JSL
//  Copyright (c) 2001-2007, OpenMFG, LLC

#ifndef moduleAR_h
#define moduleAR_h

#include <QObject>

class QToolBar;
class QMenu;

class OpenMFGGUIClient;

class moduleAR : public QObject
{
  Q_OBJECT

  public:
    moduleAR(OpenMFGGUIClient *);

  public slots:
    void sCreateInvoice();
    void sUnpostedInvoices();
    void sPrintInvoices();
    void sReprintInvoices();
    void sDeliverInvoice();
    void sPostInvoices();
    void sPurgeInvoices();

    void sEnterCashReceipt();
    void sCashReceiptEditList();
    void sPostCashReceipts();

    void sEnterFinanceCharge();
    void sCreateFinanceChargesByCustomerType();
    void sUnpostedFinanceCharges();
    void sPostFinanceCharges();

    void sEnterMiscCreditMemo();
    void sUnappliedCreditMemos();
    void sEnterMiscDebitMemo();

    void sArWorkBench();
    void sDspCustomerHistory();
    void sDspCashReceipts();
    void sDspARApplications();
    void sDspInvoiceInformation();
    void sDspAROpenItemsByCustomer();
    void sDspAROpenItems();
    void sDspTimePhasedOpenItems();
    void sDspInvoiceRegister();
    void sDspDepositsRegister();

    void sRptCustomerHistory();
    void sRptCashReceipts();
    void sRptARApplications();
    void sRptInvoiceInformation();
    void sRptAROpenItemsByCustomer();
    void sRptAROpenItems();
    void sRptSalesJournal();
    void sRptCreditMemoJournal();
    void sRptInvoiceRegister();
    void sRptDepositsRegister();

    void sPrintStatementByCustomer();
    void sPrintStatementsByCustomerType();

    void sCustomers();
    void sCustomerTypes();
    void sTaxCodes();
    void sTerms();
    void sBankAccounts();
    void sSalesCategories();
    void sReasonCodes();
    void sARAccountAssignments();

  private:
    OpenMFGGUIClient *parent;

    QToolBar *toolBar;
    QMenu *mainMenu;
    QMenu *invoicesMenu;
    QMenu *cashReceiptsMenu;
    QMenu *memosMenu;
    QMenu *financeChargesMenu;
    QMenu *displaysMenu;
    QMenu *reportsMenu;
    QMenu *masterInfoMenu;
};

#endif
