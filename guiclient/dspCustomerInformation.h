/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCUSTOMERINFORMATION_H
#define DSPCUSTOMERINFORMATION_H

#include "guiclient.h"
#include "xwidget.h"

#include "contacts.h"
#include "todoList.h"
#include "opportunityList.h"
#include "quotes.h"
#include "openSalesOrders.h"
#include "openReturnAuthorizations.h"

#include <QStandardItemModel>
#include <parameter.h>

#include "ui_dspCustomerInformation.h"

class dspCustomerInformation : public XWidget, public Ui::dspCustomerInformation
{
    Q_OBJECT

public:
    dspCustomerInformation(QWidget* parent = 0, const char * = 0, Qt::WFlags f = 0);
    ~dspCustomerInformation();

    virtual SetResponse set( const ParameterList & pParams );

    static void doDialog(QWidget * parent, const ParameterList & pParams);
    virtual bool checkSitePrivs(int invcid);

public slots:
    virtual void sPopulate();
    virtual void sEdit();
    virtual void sPrint();
    virtual void sEditInvOrder();
    virtual void sViewInvOrder();
    virtual void sFillInvoiceList();
    virtual void sNewInvoice();
    virtual void sEditInvoice();
    virtual void sViewInvoice();
    virtual void sPostInvoice();
    virtual void sFillCreditMemoList();
    virtual void sNewCreditMemo();
    virtual void sEditCreditMemo();
    virtual void sViewCreditMemo();
    virtual void sPostCreditMemo();
    virtual void sEditAropen();
    virtual void sViewAropen();
    virtual void sFillPaymentsList();
    virtual void sPopulateMenuInvoice(QMenu*, QTreeWidgetItem *selected);
    virtual void sPopulateMenuCreditMemo(QMenu*, QTreeWidgetItem *selected);
    virtual void sPopulateMenuArhist(QMenu*, QTreeWidgetItem *selected);
    virtual void sCreditMemoSelected(bool);
    virtual void sFillARHistory();
    virtual void sPopulateCustInfo();
    virtual void sCRMAccount();
    virtual void sARWorkbench();
    virtual void sInvShipment();
    virtual void sInvShipmentStatus();
    virtual void sCashReceipt();
    virtual void sPrintCCReceipt();
    virtual void sPrintCreditMemo();
    virtual void sPrintInvoice();
    virtual void sHandleCreditMemoPrint();
    virtual void sPrintStatement();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();
    
protected:
    todoList *_todoList;
    contacts *_contacts;
    opportunityList *_oplist;
    quotes *_quotes;
    openSalesOrders *_orders;
    openReturnAuthorizations *_returns;

private:
    int _crmacctId;

};

#endif // DSPCUSTOMERINFORMATION_H
