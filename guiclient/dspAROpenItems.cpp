/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAROpenItems.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include <currcluster.h>

#include "applyARCreditMemo.h"
#include "arOpenItem.h"
#include "cashReceipt.h"
#include "creditMemo.h"
#include "creditcardprocessor.h"
#include "distributeInventory.h"
#include "dspInvoiceInformation.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "getGLDistDate.h"
#include "invoice.h"
#include "incident.h"
#include "printArOpenItem.h"
#include "printCreditMemo.h"
#include "printInvoice.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"

dspAROpenItems::dspAROpenItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print,          SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_printItem,      SIGNAL(clicked()), this, SLOT(sPrintItem()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_customerSelector, SIGNAL(updated()), _aropen, SLOT(clear()));
  connect(_customerSelector, SIGNAL(updated()), this, SLOT(sHandleStatementButton()));
  connect(_aropen, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApplyAropenCM()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_refund,   SIGNAL(clicked()), this, SLOT(sCCRefundCM()));
  connect(_closed, SIGNAL(toggled(bool)), this, SLOT(sClosedToggled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  QString baseBalanceTitle(tr("Balance"));
  if (! omfgThis->singleCurrency())
    baseBalanceTitle = tr("Balance (in %1)").arg(CurrDisplay::baseCurrAbbr());

  _aropen->setRootIsDecorated(TRUE);
  _aropen->addColumn(tr("Doc. Type"),     _itemColumn, Qt::AlignLeft,   true,  "doctype");
  _aropen->addColumn(tr("Posted"),          _ynColumn, Qt::AlignCenter, true,  "posted");
  _aropen->addColumn(tr("Recurring"),       _ynColumn, Qt::AlignCenter, false, "recurring");
  _aropen->addColumn(tr("Open"),            _ynColumn, Qt::AlignCenter, false, "open");
  _aropen->addColumn(tr("Doc. #"),       _orderColumn, Qt::AlignLeft,   true,  "docnumber");
  _aropen->addColumn(tr("Cust./Assign To"),_itemColumn, Qt::AlignLeft,  true,  "cust_number");
  _aropen->addColumn(tr("Name/Desc."),             -1, Qt::AlignLeft,   true,  "cust_name");
  _aropen->addColumn(tr("Order/Incident"),_itemColumn, Qt::AlignRight,  false, "ordernumber");
  _aropen->addColumn(tr("Doc. Date"),     _dateColumn, Qt::AlignCenter, true,  "docdate");
  _aropen->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignCenter, true,  "aropen_duedate");
  _aropen->addColumn(tr("Amount"),       _moneyColumn, Qt::AlignRight,  true,  "amount");
  _aropen->addColumn(tr("Paid"),         _moneyColumn, Qt::AlignRight,  true,  "paid");
  _aropen->addColumn(tr("Balance"),      _moneyColumn, Qt::AlignRight,  true,  "balance");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,   true,  "currAbbr");
  _aropen->addColumn(baseBalanceTitle,   _moneyColumn, Qt::AlignRight,  true,  "base_balance");
  _aropen->addColumn(tr("Credit Card"),            -1, Qt::AlignLeft,   false, "ccard_number");
  
  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn("currAbbr");
    _aropen->hideColumn("balance");
  }

  int menuItem;
  QMenu * newMenu = new QMenu;
  menuItem = newMenu->insertItem(tr("Invoice"), this, SLOT(sCreateInvoice()));
  newMenu->setItemEnabled(menuItem, _privileges->check("MaintainMiscInvoices"));
  menuItem = newMenu->insertItem(tr("Misc. Debit Memo"),   this, SLOT(sEnterMiscArDebitMemo()));
  newMenu->setItemEnabled(menuItem, _privileges->check("MaintainARMemos"));
  newMenu->addSeparator();
  menuItem = newMenu->insertItem(tr("Credit Memo"), this, SLOT(sNewCreditMemo()));
  newMenu->setItemEnabled(menuItem, _privileges->check("MaintainCreditMemos"));
  menuItem = newMenu->insertItem(tr("Misc. Credit Memo"),   this, SLOT(sEnterMiscArCreditMemo()));
  newMenu->setItemEnabled(menuItem, _privileges->check("MaintainARMemos"));
  _new->setMenu(newMenu);

  
  _asOf->setDate(omfgThis->dbDate(), true);
  _closed->hide();
  sHandleButtons(false);
}

dspAROpenItems::~dspAROpenItems()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspAROpenItems::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAROpenItems::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  if (pParams.inList("byDueDate"))
    _dueDate->setChecked(true);

  param = pParams.value("cust_id", &valid);
  if (valid)
    _customerSelector->setCustId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
    
  param = pParams.value("asofDate", &valid);
  if (valid)
  {
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(FALSE);
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAROpenItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem = -1;

  if (((XTreeWidgetItem *)pItem)->altId() < 4)
  {
    menuItem = pMenu->insertItem(tr("Print..."), this, SLOT(sPrintItem()), 0);
    if (((XTreeWidgetItem *)pItem)->altId() == 0)
    // Invoice
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewMiscInvoices") || _privileges->check("MaintainMiscInvoices"));
    else if (((XTreeWidgetItem *)pItem)->altId() == 1 && ((XTreeWidgetItem *)pItem)->id("docnumber") > -1)
    // Credit Memo
     pMenu->setItemEnabled(menuItem, _privileges->check("ViewCreditMemos") || _privileges->check("MaintainCreditMemos"));
    else
    // Open Item
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
  }
      
  pMenu->insertSeparator();
  if (((XTreeWidgetItem *)pItem)->altId() == 0 && ((XTreeWidgetItem *)pItem)->rawValue("posted") == 0)
  // Invoice
  {
    menuItem = pMenu->insertItem(tr("Edit Invoice..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainMiscInvoices"));
  }
  else if (((XTreeWidgetItem *)pItem)->altId() == 1 && ((XTreeWidgetItem *)pItem)->id("docnumber") > -1)
  // Credit Memo
  {
    menuItem = pMenu->insertItem(tr("Edit Credit Memo..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCreditMemos"));
  }
  else if (((XTreeWidgetItem *)pItem)->id() > 0)
  // Open Item
  {
    menuItem = pMenu->insertItem(tr("Edit Receivable Item..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("EditAROpenItem"));
  }
  else
  // Incident
  {
    menuItem = pMenu->insertItem(tr("Edit Incident..."), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainIncidents"));
  }

  if (((XTreeWidgetItem *)pItem)->id() > 0)
  // Open Item
  {
    menuItem = pMenu->insertItem(tr("View Receivable Item..."), this, SLOT(sView()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("EditAROpenItem") || _privileges->check("ViewAROpenItems"));
  }
  
  if (((XTreeWidgetItem *)pItem)->altId() == 0)
  // Invoice
  {
    if(((XTreeWidgetItem *)pItem)->rawValue("posted") != 0)
    {
      menuItem = pMenu->insertItem(tr("Edit Posted Invoice..."), this, SLOT(sEditInvoiceDetails()), 0);
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainMiscInvoices"));
    }

    menuItem = pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoiceDetails()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewMiscInvoices"));
  
    menuItem = pMenu->insertItem(tr("View Invoice Information..."), this, SLOT(sViewInvoice()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewMiscInvoices"));
  }
  else if (((XTreeWidgetItem *)pItem)->altId() == 1 && ((XTreeWidgetItem *)pItem)->id("docnumber") > -1)
  // Credit Memo
  {
    menuItem = pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCreditMemos") || _privileges->check("ViewCreditMemos"));
  }
  else if (((XTreeWidgetItem *)pItem)->altId() == 4)
  // Incident
  {
    menuItem = pMenu->insertItem(tr("View Incident..."), this, SLOT(sViewIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") || _privileges->check("MaintainIncidents"));
  }
  
  if (((XTreeWidgetItem *)pItem)->altId() < 2 && ((XTreeWidgetItem *)pItem)->id() == -1 && ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool())
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem(tr("Post..."), this, SLOT(sPost()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("PostARDocuments"));
    
    if (((XTreeWidgetItem *)pItem)->altId() == 0)
    {
      menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDeleteInvoice()), 0);
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainMiscInvoices"));
    }
    else
    {
      menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDeleteCreditMemo()), 0);
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCreditMemos"));
    }
  }
  
  if ((((XTreeWidgetItem *)pItem)->altId() == 1 || ((XTreeWidgetItem *)pItem)->altId() == 3) && 
      ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool() && 
      ((XTreeWidgetItem *)pItem)->rawValue("open").toBool() )
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem(tr("Apply Credit Memo..."), this, SLOT(sApplyAropenCM()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ApplyARMemos"));
  }

  if ((((XTreeWidgetItem *)pItem)->id("ordernumber") > 0 && 
      ((XTreeWidgetItem *)pItem)->altId() == 0) )
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEditSalesOrder()),0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainSalesOrders") || _privileges->check("ViewSalesOrders"));
    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()),0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewSalesOrders"));
    menuItem = pMenu->insertItem(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()),0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainSalesOrders") || _privileges->check("ViewSalesOrders"));
    menuItem = pMenu->insertItem(tr("Shipments..."), this, SLOT(sShipment()),0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainSalesOrders") || _privileges->check("ViewSalesOrders"));
  }
  
  
  if ((((XTreeWidgetItem *)pItem)->altId() == 0 || 
      ((XTreeWidgetItem *)pItem)->altId() == 2) && 
      ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool() && 
      ((XTreeWidgetItem *)pItem)->rawValue("open").toBool() )
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem(tr("New Cash Receipt..."), this, SLOT(sNewCashrcpt()),0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCashReceipts"));
  }

  if (((XTreeWidgetItem *)pItem)->id() > -1 && 
     ((XTreeWidgetItem *)pItem)->rawValue("posted").toBool() && 
     ((XTreeWidgetItem *)pItem)->rawValue("open").toBool() )
  {
    pMenu->insertSeparator();
    menuItem = pMenu->insertItem(tr("New Incident..."), this, SLOT(sIncident()), 0);
    if (!_privileges->check("AddIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspAROpenItems::sApplyAropenCM()
{
  ParameterList params;
  params.append("aropen_id", _aropen->id());

  applyARCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sCCRefundCM()
{
  if (_aropen->id("ccard_number") < 0)
  {
    QMessageBox::warning(this, tr("Cannot Refund by Credit Card"),
			 tr("<p>The application cannot refund this "
			    "transaction using a credit card."));
    return;
  }
  
  int     ccardid = -1;
  double  total   =  0.0;
  double  tax     =  0.0;
  double  freight =  0.0;
  double  duty    =  0.0;
  int     currid  = -1;
  bool    taxexempt = false;
  QString docnum;
  QString refnum;
  int     ccpayid = _aropen->currentItem()->id("ccard_number");

  q.prepare("SELECT ccpay_ccard_id, aropen_amount - aropen_paid AS balance, "
	    "       aropen_curr_id, aropen_docnumber "
            "FROM aropen "
            "     JOIN payaropen ON (aropen_id=payaropen_aropen_id) "
            "     JOIN ccpay ON (payaropen_ccpay_id=ccpay_id) "
            "WHERE ((aropen_id=:aropen_id)"
            "  AND  (ccpay_id=:ccpay_id));");
  q.bindValue(":aropen_id", _aropen->id());
  q.bindValue(":ccpay_id",  ccpayid);
  q.exec();
  if (q.first())
  {
    ccardid = q.value("ccpay_ccard_id").toInt();
    total   = q.value("balance").toDouble();
    currid  = q.value("aropen_curr_id").toInt();
    docnum  = q.value("aropen_docnumber").toString();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          tr("Could not find a Credit Card to use for "
                             "this Credit transaction."));
    return;
  }

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  CreditCardProcessor::errorMsg());
  else
  {
    int refid = _aropen->id();
    int returnVal = cardproc->credit(ccardid, -2, total, tax, taxexempt,
				     freight, duty, currid,
				     docnum, refnum, ccpayid, "aropen", refid);
    if (returnVal < 0)
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    cardproc->errorMsg());
    else if (returnVal > 0)
      QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			   cardproc->errorMsg());
    else if (! cardproc->errorMsg().isEmpty())
      QMessageBox::information(this, tr("Credit Card Processing Note"),
			   cardproc->errorMsg());
  }

  sFillList();
}

void dspAROpenItems::sDeleteCreditMemo()
{
  if (QMessageBox::question(this, tr("Delete Selected Credit Memos?"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Credit Memos?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");

    if (checkCreditMemoSitePrivs(_aropen->currentItem()->id("docnumber")))
    {
      delq.bindValue(":cmhead_id", (_aropen->currentItem()->id("docnumber")));
      delq.exec();
      if (delq.first())
      {
            if (! delq.value("result").toBool())
              systemError(this, tr("Could not delete Credit Memo."),
                          __FILE__, __LINE__);
      }
      else if (delq.lastError().type() != QSqlError::NoError)
            systemError(this,
                        tr("Error deleting Credit Memo %1\n").arg(_aropen->currentItem()->text("docnumber")) +
                        delq.lastError().databaseText(), __FILE__, __LINE__);
    }

    omfgThis->sCreditMemosUpdated();
  }
}

void dspAROpenItems::sDeleteInvoice()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Invoices"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Invoices?"),
                             tr("Delete"), tr("Cancel"), QString::null, 1, 1 ) == 0)
  {
    q.prepare("SELECT deleteInvoice(:invchead_id) AS result;");

    if (checkInvoiceSitePrivs(_aropen->currentItem()->id("docnumber")))
    {
      q.bindValue(":invchead_id", _aropen->currentItem()->id("docnumber"));
      q.exec();
      if (q.first())
      {
            int result = q.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("deleteInvoice", result),
                          __FILE__, __LINE__);
            }
      }
      else if (q.lastError().type() != QSqlError::NoError)
            systemError(this,
                        tr("Error deleting Invoice %1\n").arg(_aropen->currentItem()->text("docnumber")) +
                        q.lastError().databaseText(), __FILE__, __LINE__);
    }

    omfgThis->sInvoicesUpdated(-1, TRUE);
    omfgThis->sBillingSelectionUpdated(-1, -1);
  }
}

void dspAROpenItems::sDspShipmentStatus()
{
  if (checkSalesOrderSitePrivs(_aropen->currentItem()->id("ordernumber")))
  {
    ParameterList params;
    params.append("sohead_id", _aropen->currentItem()->id("ordernumber"));
    params.append("run");

    dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItems::sEdit()
{
  ParameterList params;
    
  if (_aropen->altId() == 0 && _aropen->id() == -1)
  // Edit Unposted Invoice
  {
    if (checkInvoiceSitePrivs(_aropen->currentItem()->id("docnumber")))
    {
      params.append("invchead_id", _aropen->currentItem()->id("docnumber"));
      params.append("mode", "edit");
      invoice* newdlg = new invoice();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      return;
    }
    return;
  }
  else if (_aropen->altId() == 1 && _aropen->currentItem()->id("docnumber") > -1 && _aropen->id() -1)
  // Edit Unposted Credit Memo
  {
    if (checkCreditMemoSitePrivs(_aropen->currentItem()->id("docnumber")))
    {
      params.append("cmhead_id", _aropen->currentItem()->id("docnumber"));
      params.append("mode", "edit");
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      return;
    }
    return;
  }
  else if (_aropen->id() > 0)
  // Edit AR Open Item
  {
    params.append("mode", "edit");
    params.append("aropen_id", _aropen->id());
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else
  // Edit Incident
  {
    params.append("mode", "edit");
    params.append("incdt_id", _aropen->currentItem()->id("ordernumber"));
    incident newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void dspAROpenItems::sEditSalesOrder()
{
  if (checkSalesOrderSitePrivs(_aropen->currentItem()->id("ordernumber")))
    salesOrder::editSalesOrder(_aropen->currentItem()->id("ordernumber"), false);
}

void dspAROpenItems::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sViewSalesOrder()
{
  if (checkSalesOrderSitePrivs(_aropen->currentItem()->id("ordernumber")))
    salesOrder::viewSalesOrder(_aropen->currentItem()->id("ordernumber"));
}

void dspAROpenItems::sViewCreditMemo()
{
  ParameterList params;
  params.append("cmhead_id", _aropen->currentItem()->id("docnumber"));
  params.append("mode", "view");
  creditMemo* newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sEnterMiscArCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspAROpenItems::sEnterMiscArDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void dspAROpenItems::sCreateInvoice()
{
  invoice::newInvoice(_customerSelector->custId());
}

void dspAROpenItems::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  if (_aropen->id() > -1)
  {
    q.prepare("SELECT aropen_cust_id FROM aropen WHERE aropen_id=:aropen_id;");
    q.bindValue(":aropen_id", _aropen->id());
    q.exec();
    if (q.first())
    {
      params.append("cust_id", q.value("aropen_cust_id").toInt());
      params.append("docnumber", _aropen->currentItem()->text("docnumber"));
    }
  }
  else
  {
    if (_customerSelector->isSelectedCust())
      params.append("cust_id", _customerSelector->custId());
  }

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sViewInvoice()
{
  ParameterList params;
  params.append("invoiceNumber", _aropen->currentItem()->text("docnumber"));
  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sEditInvoiceDetails()
{
  XTreeWidgetItem *pItem = _aropen->currentItem();
  if(pItem->rawValue("posted") != 0 &&
      QMessageBox::question(this, tr("Edit Posted Invoice?"),
                            tr("<p>This Invoice has already been posted. "
                               "Are you sure you want to edit it?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    return;
  }

  ParameterList params;
  params.append("invchead_id", _aropen->currentItem()->id("docnumber"));
  params.append("mode", "edit");
  invoice* newdlg = new invoice();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sViewInvoiceDetails()
{
  ParameterList params;
  params.append("invchead_id", _aropen->currentItem()->id("docnumber"));
  params.append("mode", "view");
  invoice* newdlg = new invoice();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sIncident()
{
  q.prepare("SELECT crmacct_id, crmacct_cntct_id_1 "
            "FROM crmacct, aropen "
            "WHERE ((aropen_id=:aropen_id) "
            "AND (crmacct_cust_id=aropen_cust_id));");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("aropen_id", _aropen->id());
    params.append("crmacct_id", q.value("crmacct_id"));
    params.append("cntct_id", q.value("crmacct_cntct_id_1"));
    incident newdlg(this, 0, TRUE);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      sFillList();
  }
}

void dspAROpenItems::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _aropen->currentItem()->id("ordernumber"));
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool dspAROpenItems::setParams(ParameterList &params)
{
  _customerSelector->appendValue(params);
  if (_docDate->isChecked())
    _dates->appendValue(params);
  else
  {
    params.append("startDueDate", _dates->startDate());
    params.append("endDueDate", _dates->endDate());
  }
  params.append("invoice", tr("Invoice"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("cashdeposit", tr("Customer Deposit"));
  params.append("asofDate", _asOf->date());
  if (_incidentsOnly->isChecked())
    params.append("incidentsOnly");
  if (_debits->isChecked())
    params.append("debitsOnly");
  else if (_credits->isChecked())
    params.append("creditsOnly");
  if (_unposted->isChecked())
    params.append("showUnposted");
  if (_closed->isChecked())
    params.append("showClosed");

  params.append("key", omfgThis->_key);
  return true;
}

void dspAROpenItems::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  params.append("includeFormatted");

  orReport report("AROpenItems", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAROpenItems::sPrintItem()
{
  ParameterList params;

  if (_aropen->altId() == 0)
  {
    // Print Invoice
    printInvoice newdlg(this, "", TRUE);
    
    if (checkInvoiceSitePrivs(_aropen->currentItem()->id("docnumber")))
    {
      params.append("invchead_id", _aropen->currentItem()->id("docnumber"));
      params.append("persistentPrint");

      newdlg.set(params);
      
      if (!newdlg.isSetup())
      {
        if(newdlg.exec() == QDialog::Rejected)
          return;
        newdlg.setSetup(TRUE);
      }

      omfgThis->sInvoicesUpdated(-1, TRUE);
      return;
    }
    else
      return;
  }
  else if (_aropen->altId() == 1 && _aropen->currentItem()->id("docnumber") > -1)
  {
    // Print Credit Memo
    if (checkCreditMemoSitePrivs(_aropen->currentItem()->id("docnumber")))
    {
      params.append("cmhead_id", _aropen->currentItem()->id("docnumber"));
      params.append("persistentPrint");

      printCreditMemo newdlg(this, "", TRUE);
      newdlg.set(params);

      if (!newdlg.isSetup())
      {
        if(newdlg.exec() == QDialog::Rejected)
          return;
        newdlg.setSetup(TRUE);
      }
      omfgThis->sCreditMemosUpdated();
      return;
    }
    else
      return;
  }
  else if (_aropen->id() > 0)
  // Print AR Open Item
  {
    params.append("aropen_id", _aropen->id());

    printArOpenItem newdlg(this, "", true);
    if (newdlg.set(params) == NoError)
      newdlg.exec();
  }
}

void dspAROpenItems::sPrintStatement()
{
  q.prepare("SELECT findCustomerForm(:cust_id, 'S') AS _reportname;");
  q.bindValue(":cust_id", _customerSelector->custId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("cust_id", _customerSelector->custId());

    orReport report(q.value("_reportname").toString(), params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
}

void dspAROpenItems::sPost()
{
  if (_aropen->altId() == 1 && _aropen->id() == -1)
    sPostCreditMemo();
  else if (_aropen->altId() == 0 || _aropen->id() == -1)
    sPostInvoice();
}

void dspAROpenItems::sPostCreditMemo()
{
  if (_aropen->altId() != 1 || _aropen->id() > 0)
    return;

  int id = _aropen->currentItem()->id("docnumber");

  if (!checkCreditMemoSitePrivs(id))
    return;
    
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeSOMemoPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Credit Memo Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE cmhead SET cmhead_gldistdate=:distdate "
		  "WHERE cmhead_id=:cmhead_id;");

  if (changeDate)
  {
    setDate.bindValue(":distdate",  newDate);
    setDate.bindValue(":cmhead_id", id);
    setDate.exec();
    if (setDate.lastError().type() != QSqlError::NoError)
    {
          systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
    
  XSqlQuery postq;
  postq.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");
 
  XSqlQuery tx;
  tx.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  postq.bindValue(":cmhead_id", id);
  postq.exec();
  if (postq.first())
  {
    int result = postq.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError( this, storedProcErrorLookup("postCreditMemo", result),
            __FILE__, __LINE__);
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
    }
  }
  // contains() string is hard-coded in stored procedure
  else if (postq.lastError().databaseText().contains("post to closed period"))
  {
    rollback.exec();
    systemError(this, tr("Could not post Credit Memo #%1 because of a missing exchange rate.")
                                      .arg(_aropen->currentItem()->text("docnumber")));
    return;
  }
  else if (postq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, tr("A System Error occurred posting Credit Memo#%1.\n%2")
            .arg(_aropen->currentItem()->text("docnumber"))
            .arg(postq.lastError().databaseText()),
          __FILE__, __LINE__);
  }

  omfgThis->sCreditMemosUpdated();
}

void dspAROpenItems::sPostInvoice()
{
  if (_aropen->altId() != 0 || _aropen->id() > 0)
    return;
    
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (!checkInvoiceSitePrivs(_aropen->currentItem()->id("docnumber")))
    return;

  if (_privileges->check("ChangeARInvcDistDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Invoice Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  int journal = -1;
  q.exec("SELECT fetchJournalNumber('AR-IN') AS result;");
  if (q.first())
  {
    journal = q.value("result").toInt();
    if (journal < 0)
    {
      systemError(this, storedProcErrorLookup("fetchJournalNumber", journal), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery xrate;
  xrate.prepare("SELECT curr_rate "
		"FROM curr_rate, invchead "
		"WHERE ((curr_id=invchead_curr_id)"
		"  AND  (invchead_id=:invchead_id)"
		"  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
  // if SUM becomes dependent on curr_id then move XRATE before it in the loop
  XSqlQuery sum;
  sum.prepare("SELECT invoicetotal(:invchead_id) AS subtotal;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE invchead SET invchead_gldistdate=:distdate "
		  "WHERE invchead_id=:invchead_id;");

  if (changeDate)
  {
    setDate.bindValue(":distdate",    newDate);
    setDate.bindValue(":invchead_id", _aropen->currentItem()->id("docnumber"));
    setDate.exec();
    if (setDate.lastError().type() != QSqlError::NoError)
      systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
  }

  sum.bindValue(":invchead_id", _aropen->currentItem()->id("docnumber"));
  if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
  {
    if (QMessageBox::question(this, tr("Invoice Has Value 0"),
                        tr("Invoice #%1 has a total value of 0.\n"
                           "Would you like to post it anyway?")
                          .arg(_aropen->currentItem()->text("docnumber")),
                        QMessageBox::Yes,
                        QMessageBox::No | QMessageBox::Default)
    == QMessageBox::No)
      return;
  }
  else if (sum.lastError().type() != QSqlError::NoError)
  {
    systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (sum.value("subtotal").toDouble() != 0)
  {
    xrate.bindValue(":invchead_id", _aropen->currentItem()->id("docnumber"));
    xrate.exec();
    if (xrate.lastError().type() != QSqlError::NoError)
    {
      systemError(this, tr("System Error posting Invoice #%1\n%2")
                          .arg(_aropen->currentItem()->text("docnumber"))
                          .arg(xrate.lastError().databaseText()),
                      __FILE__, __LINE__);
      return;
    }
    else if (!xrate.first() || xrate.value("curr_rate").isNull())
    {
      systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
                                      .arg(_aropen->currentItem()->text("docnumber")));
      return;
    }

    post.bindValue(":invchead_id", _aropen->currentItem()->id("docnumber"));
    post.bindValue(":journal",     journal);
    post.exec();
    if (post.first())
    {
      int result = post.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("postInvoice", result),
                    __FILE__, __LINE__);
    }
    // contains() string is hard-coded in stored procedure
    else if (post.lastError().databaseText().contains("post to closed period"))
      systemError(this, tr("Could not post Invoice #%1 into a closed period.")
                              .arg(_aropen->currentItem()->text("docnumber")));

  }
  else if (post.lastError().type() != QSqlError::NoError)
        systemError(this, tr("A System Error occurred posting Invoice #%1.\n%2")
                .arg(_aropen->currentItem()->text("docnumber"))
                    .arg(post.lastError().databaseText()),
                    __FILE__, __LINE__);

  omfgThis->sInvoicesUpdated(-1, TRUE);
}

void dspAROpenItems::sShipment()
{
  if (checkSalesOrderSitePrivs(_aropen->currentItem()->id("ordernumber")))
  {
    ParameterList params;
    params.append("sohead_id", _aropen->currentItem()->id("ordernumber"));

    dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItems::sFillList()
{
  MetaSQLQuery mql = mqlLoad("arOpenItems", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _aropen->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspAROpenItems::sHandleStatementButton()
{
  if (_customerSelector->isValid() &&
      _customerSelector->isSelectedCust())
  {
    if (_print->menu())
      return;
    else
      disconnect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

    QMenu * printMenu = new QMenu;
    printMenu->insertItem(tr("&List..."), this, SLOT(sPrint()), 0);
    printMenu->insertItem(tr("&Statement..."), this, SLOT(sPrintStatement()), 0);
    _print->setMenu(printMenu);
  }
  else if (_print->menu())
  {
    _print->setMenu(0);
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  }
}


void dspAROpenItems::sHandleButtons(bool valid)
{
  sHandleStatementButton();
  if (valid)
  {
    // Handle Print Item Button
    if (_aropen->altId() == 0)
    // Invoice
      _printItem->setEnabled(_privileges->check("ViewMiscInvoices") || _privileges->check("MaintainMiscInvoices"));
    else if (_aropen->altId() == 1 && _aropen->currentItem()->id("docnumber") > -1)
    // Credit Memo
      _printItem->setEnabled(_privileges->check("ViewCreditMemos") || _privileges->check("EditCreditMemos"));
    else if (_aropen->id() > 0)
    // Open Item
      _printItem->setEnabled(_privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
    else
    // Incident
      _printItem->setEnabled(false);

    // Handle Edit Button
    if (_aropen->altId() == 0 && _aropen->id() == -1 && _aropen->currentItem()->rawValue("posted") == 0)
    // Unposted Invoice
      _edit->setEnabled(_privileges->check("MaintainMiscInvoices"));
    else if (_aropen->altId() == 1 && _aropen->currentItem()->id("docnumber") > -1 && _aropen->currentItem()->rawValue("posted") == 0)
    // Unposted Credit Memo
      _edit->setEnabled(_privileges->check("MaintainCreditMemos"));
    else if (_aropen->id() > 0)
    // Open Item
      _edit->setEnabled(_privileges->check("EditAROpenItem"));
    else if (_aropen->altId() == 4)
    // Incident
      _edit->setEnabled(_privileges->check("MaintainIncidents"));
    
    // Handle View Button
    int menuItem = -1;
    QMenu * viewMenu = new QMenu;
    if (_aropen->id() > 0)
    // Open Item
    {
      viewMenu->insertItem(tr("Receivable Item..."), this, SLOT(sView()), 0);
      viewMenu->setItemEnabled(menuItem, _privileges->check("EditAROpenItem") || _privileges->check("ViewAROpenItems"));
    }
    if (_aropen->altId() == 0)
    // Invoice
    {
      viewMenu->insertItem(tr("Invoice..."), this, SLOT(sViewInvoiceDetails()), 0);
      viewMenu->setItemEnabled(menuItem, _privileges->check("ViewMiscInvoices"));
    
      viewMenu->insertItem(tr("Invoice Information..."), this, SLOT(sViewInvoice()), 0);
      viewMenu->setItemEnabled(menuItem, _privileges->check("ViewMiscInvoices"));
    }
    else if (_aropen->altId() == 1 && _aropen->id("docnumber") > -1)
    // Credit Memo
    {
      viewMenu->insertItem(tr("Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
      viewMenu->setItemEnabled(menuItem, _privileges->check("MaintainCreditMemos") || _privileges->check("ViewCreditMemos"));
    }
    else if (_aropen->altId() == 4)
    // Incident
    {
      viewMenu->insertItem(tr("Incident..."), this, SLOT(sViewIncident()), 0);
      viewMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") || _privileges->check("MaintainIncidents"));
    }
    _view->setMenu(viewMenu);
    _view->setEnabled(true);
    
    // Handle Post and Apply Button
    if ((_aropen->altId() == 1 || _aropen->altId() == 3) && 
        _aropen->currentItem()->rawValue("posted").toBool() && 
        _aropen->currentItem()->rawValue("open").toBool() )
    {
      _post->hide();
      _apply->show();
      _apply->setEnabled(_privileges->check("ApplyARMemos"));
    }
    else
    {
      _apply->hide();
      _post->show();
      _post->setEnabled(_aropen->altId() < 4 && _aropen->currentItem()->rawValue("posted") == 0 && _privileges->check("PostARDocuments"));
    }
    
    _refund->setVisible(_metrics->boolean("CCAccept") &&
                        _privileges->check("ProcessCreditCards"));
    _refund->setEnabled(_aropen->id("ccard_number") > 0 &&
                        _aropen->currentItem()->rawValue("balance").toDouble() < 0);
      
  }
  else
  {
    _printItem->setEnabled(false);
    _edit->setEnabled(false);
    _view->setEnabled(false);
    _apply->hide();
    _post->show();
    _post->setEnabled(false);
    _refund->setEnabled(false);
  }
}

bool dspAROpenItems::checkInvoiceSitePrivs(int invcid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkInvoiceSitePrivs(:invcheadid) AS result;");
    check.bindValue(":invcheadid", invcid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Invoice as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

bool dspAROpenItems::checkCreditMemoSitePrivs(int cmid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkCreditMemoSitePrivs(:cmheadid) AS result;");
    check.bindValue(":cmheadid", cmid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Credit Memo as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

bool dspAROpenItems::checkSalesOrderSitePrivs(int soid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", soid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Sales Order as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

void dspAROpenItems::sClosedToggled(bool checked)
{
  if (checked)
    _dates->setStartDate(QDate().currentDate().addDays(-90));
  else
    _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
}

