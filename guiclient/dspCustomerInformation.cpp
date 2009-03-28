/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomerInformation.h"

#include "xdialog.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include "arOpenItem.h"
#include "arWorkBench.h"
#include "cashReceipt.h"
#include "creditMemo.h"
#include "distributeInventory.h"
#include "creditcardprocessor.h"
#include "crmaccount.h"
#include "customer.h"
#include "dspInvoiceInformation.h"
#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "invoice.h"
#include "printCreditMemo.h"
#include "printInvoice.h"
#include "crmaccount.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "storedProcErrorLookup.h"

dspCustomerInformation::dspCustomerInformation(QWidget* parent, Qt::WFlags fl)
    : XWidget (parent, fl)
{
  setupUi(this);

 //_crmTab->setEnabled(false);
  //_salesTab->setEnabled(false);
  
  _todoList = new todoList(this, "todoList", Qt::Widget);
  _todoListPage->layout()->addWidget(_todoList);
  _todoList->findChild<QWidget*>("_close")->hide();
  _todoList->findChild<QWidget*>("_usr")->hide();
  _todoList->findChild<QWidget*>("_startdateGroup")->hide();
  _todoList->findChild<QWidget*>("_duedateGroup")->hide();
  _todoList->findChild<ParameterGroup*>("_usr")->setState(ParameterGroup::All);
  
  _contacts = new contacts(this, "contacts", Qt::Widget);
  _contactsPage->layout()->addWidget(_contacts);
  _contacts->findChild<QWidget*>("_close")->hide();
  _contacts->findChild<QWidget*>("_activeOnly")->hide();
  _contacts->findChild<QWidget*>("_contactsLit")->hide();
  
  _oplist = new opportunityList(this, "opportunityList", Qt::Widget);
  _opportunitiesPage->layout()->addWidget(_oplist);
  _oplist->findChild<QWidget*>("_close")->hide();
  _oplist->findChild<QWidget*>("_usr")->hide();
  _oplist->findChild<QWidget*>("_dates")->hide();
  _oplist->findChild<ParameterGroup*>("_usr")->setState(ParameterGroup::All);
  
  _quotes = new quotes(this, "quotes", Qt::Widget);
  _quotesPage->layout()->addWidget(_quotes);
  _quotes->findChild<QWidget*>("_close")->hide();
  _quotes->findChild<QWidget*>("_warehouse")->hide();
  _quotes->findChild<QWidget*>("_quoteLit")->hide();
  _quotes->findChild<WarehouseGroup*>("_warehouse")->setAll();
  _quotes->findChild<XCheckBox*>("_showExpired")->setForgetful(true);
  _quotes->findChild<XCheckBox*>("_showExpired")->setChecked(true);
  _quotes->findChild<XCheckBox*>("_showProspects")->setForgetful(true);
  _quotes->findChild<XCheckBox*>("_showProspects")->setChecked(false);
  _quotes->findChild<QWidget*>("_options")->hide();
  
  _orders = new openSalesOrders(this, "_orders", Qt::Widget);
  _ordersPage->layout()->addWidget(_orders);
  _orders->findChild<QWidget*>("_close")->hide();
  _orders->findChild<XCheckBox*>("_autoUpdate")->setForgetful(true);
  _orders->findChild<XCheckBox*>("_autoUpdate")->setChecked(false);
  _orders->findChild<XCheckBox*>("_autoUpdate")->hide();
  _orders->findChild<QWidget*>("_warehouse")->hide();
  _orders->findChild<QWidget*>("_salesOrdersLit")->hide();
  _orders->findChild<WarehouseGroup*>("_warehouse")->setAll();
  _orders->findChild<XCheckBox*>("_showClosed")->setForgetful(false);
  _orders->findChild<XCheckBox*>("_showClosed")->show();

  connect(_arhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuArhist(QMenu*, QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_creditMemo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuCreditMemo(QMenu*, QTreeWidgetItem*)));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_custList, SIGNAL(clicked()), _cust, SLOT(sEllipses()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_editCreditMemo, SIGNAL(clicked()), this, SLOT(sEditCreditMemo()));
  connect(_editInvoice, SIGNAL(clicked()), this, SLOT(sEditInvoice()));
  connect(_invoice, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuInvoice(QMenu*, QTreeWidgetItem*)));
  connect(_newCreditMemo, SIGNAL(clicked()), this, SLOT(sNewCreditMemo()));
  connect(_newInvoice, SIGNAL(clicked()), this, SLOT(sNewInvoice()));
  connect(_viewCreditMemo, SIGNAL(clicked()), this, SLOT(sViewCreditMemo()));
  connect(_viewInvoice, SIGNAL(clicked()), this, SLOT(sViewInvoice()));
  connect(_crmAccount, SIGNAL(clicked()), this, SLOT(sCRMAccount()));
  connect(_refresh, SIGNAL(clicked()), this, SLOT(sPopulate()));
  connect(_workbenchInvoice, SIGNAL(clicked()), this, SLOT(sARWorkbench()));
  connect(_cashReceiptInvoice, SIGNAL(clicked()), this, SLOT(sCashReceipt()));
  connect(_workbenchCreditMemo, SIGNAL(clicked()), this, SLOT(sARWorkbench()));
  connect(_printInvoice, SIGNAL(clicked()), this ,SLOT(sPrintInvoice()));
  connect(_printCreditMemo, SIGNAL(clicked()), this ,SLOT(sPrintCreditMemo()));
  connect(_printReceipt,    SIGNAL(clicked()), this, SLOT(sPrintCCReceipt()));
  connect(_contactsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_todoListButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_opportunitiesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_notesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_commentsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_quotesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_ordersButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_returnsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_cust, SIGNAL(valid(bool)), _crmTab, SLOT(setEnabled(bool)));
  connect(_cust, SIGNAL(valid(bool)), _salesTab, SLOT(setEnabled(bool)));

#ifndef Q_WS_MAC
  _custList->setMaximumWidth(25);
#endif

  // setup arhist list
  _arhist->addColumn(tr("Open"),          _orderColumn,    Qt::AlignCenter, true,  "open" );
  _arhist->addColumn(tr("Doc. Type"),     _dateColumn,     Qt::AlignCenter, true,  "documenttype" );
  _arhist->addColumn(tr("Doc. #"),        -1,              Qt::AlignRight,  true,  "docnumber"  );
  _arhist->addColumn(tr("Doc. Date"),     _dateColumn,     Qt::AlignCenter, true,  "docdate" );
  _arhist->addColumn(tr("Due/Dist Date"), _dateColumn,     Qt::AlignCenter, true,  "duedate" );
  _arhist->addColumn(tr("Amount"),        _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _arhist->addColumn(tr("Balance"),       _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _arhist->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,   true,  "currAbbr");
  _arhist->addColumn(tr("Base Balance"),  _bigMoneyColumn, Qt::AlignRight,  true,  "base_balance"  );

  // setup Invoice list
  _invoice->addColumn(tr("Posted"),     _ynColumn,       Qt::AlignCenter, true,  "posted" );
  _invoice->addColumn(tr("Open"),       _ynColumn,       Qt::AlignCenter, true,  "open" );
  _invoice->addColumn(tr("Recurring"),  _ynColumn,       Qt::AlignCenter, true,  "recurring" );
  _invoice->addColumn(tr("Invoice #"),  -1,              Qt::AlignLeft,   true,  "invcnumber"   );
  _invoice->addColumn(tr("S/O #"),      _orderColumn,    Qt::AlignLeft,   true,  "ordernumber"   );
  _invoice->addColumn(tr("Invc. Date"), _dateColumn,     Qt::AlignCenter, true,  "docdate"  );
  _invoice->addColumn(tr("Due Date"),   _dateColumn,     Qt::AlignCenter, true,  "duedate" );
  _invoice->addColumn(tr("Amount"),     _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _invoice->addColumn(tr("Balance"),    _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _invoice->addColumn(tr("Currency"),   _currencyColumn, Qt::AlignLeft,   true,  "currAbbr");
  if(_privileges->check("MaintainMiscInvoices"))
  {
    connect(_invoice, SIGNAL(valid(bool)), _editInvoice, SLOT(setEnabled(bool)));
    connect(_invoice, SIGNAL(itemSelected(int)), _editInvoice, SLOT(animateClick()));
  }
  else
  {
    _newInvoice->setEnabled(false);
    connect(_invoice, SIGNAL(itemSelected(int)), _viewInvoice, SLOT(animateClick()));
  }
  if(_privileges->check("MaintainMiscInvoices") || _privileges->check("ViewMiscInvoices"))
    connect(_invoice, SIGNAL(valid(bool)), _viewInvoice, SLOT(setEnabled(bool)));
  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillInvoiceList()));

  // setup CreditMemo list
  _creditMemo->addColumn(tr("Posted"),    _ynColumn,       Qt::AlignCenter, true,  "posted" );
  _creditMemo->addColumn(tr("Open"),      _ynColumn,       Qt::AlignCenter, true,  "open" );
  _creditMemo->addColumn(tr("Type"),      _ynColumn,       Qt::AlignCenter, true,  "type" );
  _creditMemo->addColumn(tr("Memo #"),     -1,             Qt::AlignLeft,   true,  "docnumber"   );
  _creditMemo->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter, true,  "docdate" );
  _creditMemo->addColumn(tr("Amount"),    _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _creditMemo->addColumn(tr("Balance"),   _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _creditMemo->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"  );
  if(!_privileges->check("MaintainCreditMemos"))
    _newCreditMemo->setEnabled(false);
  connect(_creditMemo, SIGNAL(valid(bool)), this, SLOT(sCreditMemoSelected(bool)));
  if (_privileges->check("MaintainCreditMemos") || _privileges->check("ViewCreditMemos"))
    connect(_creditMemo, SIGNAL(itemSelected(int)), _viewCreditMemo, SLOT(animateClick()));
  else
    _viewCreditMemo->setEnabled(false);
  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillCreditMemoList()));

  _payments->addColumn(tr("Type"),         _whsColumn,      Qt::AlignLeft,   true,  "type"  );
  _payments->addColumn(tr("Status"),       _whsColumn,      Qt::AlignLeft,   true,  "status"  );
  _payments->addColumn(tr("Timestamp"),    _timeDateColumn, Qt::AlignLeft,   true,  "ccpay_transaction_datetime"  );
  _payments->addColumn(tr("Entered By"),   _userColumn,     Qt::AlignLeft,   true,  "ccpay_by_username"  );
  _payments->addColumn(tr("Total Amount"), _moneyColumn,    Qt::AlignRight,  true,  "ccpay_amount" );
  _payments->addColumn(tr("Amount Currency"),     _currencyColumn, Qt::AlignLeft,   true,  "ccpay_currAbbr"  );
  _payments->addColumn(tr("Document #"),   -1,              Qt::AlignLeft,   true,  "docnumber"  );
  _payments->addColumn(tr("Reference"),    _orderColumn,    Qt::AlignLeft,   true,  "ccpay_r_ref"  );
  _payments->addColumn(tr("Allocated"),    _moneyColumn,    Qt::AlignRight,  true,  "allocated" );
  _payments->addColumn(tr("Allocated Currency"),     _currencyColumn, Qt::AlignLeft,   true,  "payco_currAbbr"  );

  if (_privileges->check("MaintainCRMAccounts") || _privileges->check("ViewCRMAccounts"))
    connect (_cust, SIGNAL(valid(bool)), _crmAccount, SLOT(setEnabled(bool)));
  if (_privileges->check("MaintainCustomerMasters"))
    connect (_cust, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  if (_privileges->check("ViewAROpenItems"))
  {
    connect (_cust, SIGNAL(valid(bool)), _workbenchInvoice, SLOT(setEnabled(bool)));
    connect (_cust, SIGNAL(valid(bool)), _workbenchCreditMemo, SLOT(setEnabled(bool)));
  }
  if (_privileges->check("MaintainCashReceipts"))
    connect (_cust, SIGNAL(valid(bool)), _cashReceiptInvoice, SLOT(setEnabled(bool)));
  if (_privileges->check("ViewAROpenItems"))
    connect (_cust, SIGNAL(valid(bool)), _workbenchCreditMemo, SLOT(setEnabled(bool)));
  if (_privileges->check("MaintainMiscInvoices"))
    connect (_cust, SIGNAL(valid(bool)), _newInvoice, SLOT(setEnabled(bool)));
  if (_privileges->check("MaintainCreditMemos"))
    connect (_cust, SIGNAL(valid(bool)), _newCreditMemo, SLOT(setEnabled(bool)));

  QMenu * _printMenu = new QMenu;
  _printMenu->addAction(tr("Customer Infomation"), this, SLOT(sPrint()));
  _printMenu->addAction(tr("Statement"),      this, SLOT(sPrintStatement()));
  _print->setMenu(_printMenu);

  _backlog->setPrecision(omfgThis->moneyVal());
  _creditLimit->setPrecision(omfgThis->moneyVal());
  _lastYearSales->setPrecision(omfgThis->moneyVal());
  _lateBalance->setPrecision(omfgThis->moneyVal());
  _openBalance->setPrecision(omfgThis->moneyVal());
  _ytdSales->setPrecision(omfgThis->moneyVal());
}

dspCustomerInformation::~dspCustomerInformation()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspCustomerInformation::languageChange()
{
    retranslateUi(this);
}

enum SetResponse dspCustomerInformation::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    _cust->setReadOnly(TRUE);
    _custList->setEnabled(FALSE);
  }

  if(pParams.inList("modal"))
  {
    disconnect(_creditMemo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuCreditMemo(QMenu*)));
    disconnect(_invoice, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenuInvoice(QMenu*)));
    if(_privileges->check("MaintainMiscInvoices"))
      disconnect(_invoice, SIGNAL(itemSelected(int)), _editInvoice, SLOT(animateClick()));
    else
      disconnect(_invoice, SIGNAL(itemSelected(int)), _viewInvoice, SLOT(animateClick()));
    disconnect(_creditMemo, SIGNAL(itemSelected(int)), _viewCreditMemo, SLOT(animateClick()));

    _edit->hide();
    _editCreditMemo->hide();
    _editInvoice->hide();
    _viewCreditMemo->hide();
    _viewInvoice->hide();
    _newCreditMemo->hide();
    _newInvoice->hide();
  }
  return NoError;
}

void dspCustomerInformation::sPopulateCustInfo()
{
  if (_cust->id() != -1)
  {
    _comment->setId(_cust->id());

    XSqlQuery query;
    query.prepare( "SELECT cust_name,"
                   "       formatBoolYN(cust_active) AS f_active,"
                   "       cust_address1, cust_address2, cust_address3,"
                   "       cust_city, cust_state, cust_zipcode,"
                   "       cust_contact, cust_phone, cust_fax, cust_email,"
                   "       cust_corraddress1, cust_corraddress2, cust_corraddress3,"
                   "       cust_corrcity, cust_corrstate, cust_corrzipcode,"
                   "       cust_corrcontact, cust_corrphone, cust_corrfax, cust_corremail,"
                   "       cust_creditstatus, cust_comments, cust_shipvia,"
                   "       (custtype_code || '-' || custtype_descrip) AS f_custtype,"
                   "       (terms_code || '-' || terms_descrip) AS f_terms,"
                   "       (salesrep_number || '-' || salesrep_name) AS f_salesrep,"
                   "       cust_creditlmt, crmacct_id "
                   "  FROM cust, custtype, terms, crmacct, salesrep "
                   " WHERE ((cust_custtype_id=custtype_id)"
                   "   AND  (cust_terms_id=terms_id)"
                   "   AND  (cust_id=:cust_id)"
                   "   AND  (crmacct_cust_id=cust_id) "
                   "   AND  (salesrep_id=cust_salesrep_id));" );
    query.bindValue(":cust_id", _cust->id());
    query.exec();
    if (query.first())
    {
      _name->setText(query.value("cust_name").toString());
      _active->setText(query.value("f_active").toString());
      _corrAddress1->setText(query.value("cust_corraddress1").toString());
      _corrAddress2->setText(query.value("cust_corraddress2").toString());
      _corrAddress3->setText(query.value("cust_corraddress3").toString());
      _corrCity->setText(query.value("cust_corrcity").toString());
      _corrState->setText(query.value("cust_corrstate").toString());
      _corrZipCode->setText(query.value("cust_corrzipcode").toString());
      _corrContact->setText(query.value("cust_corrcontact").toString());
      _corrPhone->setText(query.value("cust_corrphone").toString());
      _corrFAX->setText(query.value("cust_corrfax").toString());
      _corrEmail->setText(query.value("cust_corremail").toString());
      _billAddress1->setText(query.value("cust_address1").toString());
      _billAddress2->setText(query.value("cust_address2").toString());
      _billAddress3->setText(query.value("cust_address3").toString());
      _billCity->setText(query.value("cust_city").toString());
      _billState->setText(query.value("cust_state").toString());
      _billZipCode->setText(query.value("cust_zipcode").toString());
      _billContact->setText(query.value("cust_contact").toString());
      _billPhone->setText(query.value("cust_phone").toString());
      _billFAX->setText(query.value("cust_fax").toString());
      _billEmail->setText(query.value("cust_email").toString());
      _notes->setText(query.value("cust_comments").toString());
      _type->setText(query.value("f_custtype").toString());
      _terms->setText(query.value("f_terms").toString());
      _shipvia->setText(query.value("cust_shipvia").toString());
      _salesrep->setText(query.value("f_salesrep").toString());
      _creditLimit->setDouble(query.value("cust_creditlmt").toDouble());
      _crmacctId = query.value("crmacct_id").toInt();
      _todoList->findChild<CRMAcctCluster*>("_crmAccount")->setId(_crmacctId);
      _contacts->findChild<CRMAcctCluster*>("_crmAccount")->setId(_crmacctId);
      _oplist->findChild<CRMAcctCluster*>("_crmAccount")->setId(_crmacctId);
      _quotes->findChild<CustCluster*>("_cust")->setId(_cust->id());
      _orders->findChild<CustCluster*>("_cust")->setId(_cust->id());

      if (query.value("cust_creditstatus").toString() == "G")
      {
        _creditStatus->setText(tr("In Good Standing"));
        _creditStatus->setPaletteForegroundColor(QColor("black"));
      }
      else if (query.value("cust_creditstatus").toString() == "W")
      {
        _creditStatus->setText(tr("On Credit Warning"));
        _creditStatus->setPaletteForegroundColor(QColor("orange"));
      }
      else if (query.value("cust_creditstatus").toString() == "H")
      {
        _creditStatus->setText(tr("On Credit Hold"));
        _creditStatus->setPaletteForegroundColor(QColor("red"));
      }

      query.prepare( "SELECT MIN(cohist_invcdate) AS firstdate,"
                     "       MAX(cohist_invcdate) AS lastdate "
                     "FROM cohist "
                     "WHERE (cohist_cust_id=:cust_id);" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _firstSaleDate->setDate(query.value("firstdate").toDate());
        _lastSaleDate->setDate(query.value("lastdate").toDate());
      }
      query.prepare( "SELECT COALESCE(SUM(round(cohist_qtyshipped * cohist_unitprice,2)), 0) AS lysales "
                     "FROM cohist "
                     "WHERE ( (cohist_invcdate BETWEEN (DATE_TRUNC('year', CURRENT_TIMESTAMP) - INTERVAL '1 year') AND"
                     "                                 (DATE_TRUNC('year', CURRENT_TIMESTAMP) - INTERVAL '1 day'))"
                     " AND (cohist_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _lastYearSales->setDouble(query.value("lysales").toDouble());

      query.prepare( "SELECT COALESCE(SUM(round(cohist_qtyshipped * cohist_unitprice,2)), 0) AS ytdsales "
                     "FROM cohist "
                     "WHERE ( (cohist_invcdate>=DATE_TRUNC('year', CURRENT_TIMESTAMP))"
                     " AND (cohist_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _ytdSales->setDouble(query.value("ytdsales").toDouble());

      query.prepare( "SELECT COALESCE( SUM( (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
                     "                                   (coitem_price / coitem_price_invuomratio) ), 0 ) AS backlog "
                     "FROM cohead, coitem, itemsite, item "
                     "WHERE ( (coitem_cohead_id=cohead_id)"
                     " AND (coitem_itemsite_id=itemsite_id)"
                     " AND (itemsite_item_id=item_id)"
                     " AND (coitem_status='O')"
                     " AND (cohead_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _backlog->setDouble(query.value("backlog").toDouble());

      query.prepare( "SELECT COALESCE( SUM( CASE WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
                     "                                       ELSE ((aropen_amount - aropen_paid) * -1)"
                     "                                   END ), 0 ) AS balance "
                     "FROM aropen "
                     "WHERE ( (aropen_open)"
                     " AND (aropen_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _openBalance->setDouble(query.value("balance").toDouble());

      query.prepare( "SELECT noNeg( COALESCE( SUM( CASE WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
                     "                                      ELSE ((aropen_amount - aropen_paid) * -1)"
                     "                                   END ), 0 ) ) AS balance "
                     "FROM aropen "
                     "WHERE ( (aropen_open)"
                     " AND (aropen_duedate < CURRENT_DATE)"
                     " AND (aropen_cust_id=:cust_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
        _lateBalance->setDouble(query.value("balance").toDouble());
    }
  }
}

void dspCustomerInformation::sPopulate()
{
  sPopulateCustInfo();
  if (_cust->isValid())
  {
    sFillARHistory();
    sFillInvoiceList();
    sFillCreditMemoList();
    sFillPaymentsList();
    _todoList->sFillList();
    _contacts->sFillList();
    _oplist->sFillList();
    _quotes->sFillList();
    _orders->sFillList();
  }
  else
  {
    _todoList->findChild<XTreeWidget*>("_todoList")->clear();
    _contacts->findChild<XTreeWidget*>("_contacts")->clear();
    _oplist->findChild<XTreeWidget*>("_list")->clear();
    _quotes->findChild<XTreeWidget*>("_quote")->clear();
    _orders->findChild<XTreeWidget*>("_so")->clear();
  }
}

void dspCustomerInformation::sEdit()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "edit");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sPrint()
{
  if(!_cust->isValid())
  {
    QMessageBox::warning(this, tr("No Customer Selected"),
      tr("You must select a valid customer before printing.") );
    return;
  }
  ParameterList params;
  params.append("cust_id", _cust->id());

  orReport report("CustomerInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCustomerInformation::sFillARHistory()
{
  _arhist->clear();

  if(_cust->id() == -1)
    return;

  MetaSQLQuery mql = mqlLoad("arHistory", "detail");
  ParameterList params;
  params.append("invoice", tr("Invoice"));
  params.append("zeroinvoice", tr("Zero Invoice"));
  params.append("creditMemo", tr("C/M"));
  params.append("debitMemo", tr("D/M"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("cashdeposit", tr("Cash Deposit"));
  params.append("other", tr("Other"));
  params.append("cust_id", _cust->id());
  q = mql.toQuery(params);
  _arhist->populate(q, true);

  // End Population of A/R History
}

void dspCustomerInformation::sEditInvOrder()
{
    q.prepare("SELECT cohead_id "
              "FROM invchead, cohead "
              "WHERE ((cohead_number=invchead_ordernumber) "
              "  AND (invchead_id=:invoice_id));");
    q.bindValue(":invoice_id",  _invoice->id());
    q.exec();
 //   if (q.first())
 //     salesOrder::editSalesOrder(q.value("cohead_id").toInt(), false);
}

void dspCustomerInformation::sViewInvOrder()
{
    q.prepare("SELECT cohead_id "
              "FROM invchead, cohead "
              "WHERE ((cohead_number=invchead_ordernumber) "
              "  AND (invchead_id=:invoice_id));");
    q.bindValue(":invoice_id",  _invoice->id());
    q.exec();
 //   if (q.first())
 //     salesOrder::viewSalesOrder(q.value("cohead_id").toInt());
}

void dspCustomerInformation::sFillInvoiceList()
{
  QString sql("SELECT invchead_id AS id, -1 AS altId,"
            "       invchead_posted AS posted,"
            "       invchead_recurring AS recurring,"
            "       COALESCE(aropen_open, FALSE) AS open,"
            "       text(invchead_invcnumber) AS invcnumber,"
            "       text(invchead_ordernumber) AS ordernumber,"
            "       invchead_invcdate AS docdate,"
            "       aropen_duedate AS duedate,"
            "       COALESCE(aropen_amount,0) AS amount,"
            "       COALESCE(aropen_amount - aropen_paid,0) AS balance,"
            "       currConcat(COALESCE(aropen_curr_id,-1)) AS currAbbr,"
            "       CASE WHEN ((COALESCE(aropen_duedate,current_date) < current_date) "
            "                   AND COALESCE(aropen_open,FALSE)) THEN 'warning' "
            "       END AS qtforegroundrole,"
            "       'curr' AS amount_xtnumericrole,"
            "       'curr' AS balance_xtnumericrole "
            "  FROM invchead LEFT OUTER JOIN aropen"
            "         ON (aropen_docnumber=invchead_invcnumber"
            "         AND aropen_doctype='I'"
            "         AND aropen_cust_id=invchead_cust_id)"
            " WHERE (invchead_cust_id=:cust_id)");

  if (!_invoiceShowclosed->isChecked())
    sql +=  "   AND ( (COALESCE(aropen_open,FALSE)) OR (NOT invchead_posted) ) ";

  sql +=    " UNION "
            "SELECT aropen_id AS id, -2 AS altId,"
            "       true AS posted,"
            "       aropen_open AS open,"
            "       false AS recurring,"
            "       text(aropen_docnumber) AS invcnumber,"
            "       aropen_ordernumber AS ordernumber,"
            "       aropen_docdate AS docdate,"
            "       aropen_duedate AS duedate,"
            "       aropen_amount AS amount,"
            "       (aropen_amount - aropen_paid) AS balance,"
            "       currConcat(aropen_curr_id) AS currAbbr,"
            "       CASE WHEN ((COALESCE(aropen_duedate,current_date) < current_date) "
            "                   AND aropen_open) THEN 'warning' "
            "       END AS qtforegroundrole,"
            "       'curr' AS amount_xtnumericrole,"
            "       'curr' AS balance_xtnumericrole "
            "  FROM aropen LEFT OUTER JOIN invchead"
            "         ON (aropen_docnumber=invchead_invcnumber"
            "         AND aropen_doctype='I'"
            "         AND aropen_cust_id=invchead_cust_id)"
            " WHERE ((aropen_doctype='I')"
            "   AND  (invchead_id IS NULL)";

  if (!_invoiceShowclosed->isChecked())
    sql +=  "   AND (aropen_open) ";

  sql +=    "   AND  (aropen_cust_id=:cust_id) ) "
            "ORDER BY invcnumber DESC;";
  q.prepare(sql);
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _invoice->clear();
  _invoice->populate(q, true);
}

void dspCustomerInformation::sNewInvoice()
{
  invoice::newInvoice(_cust->id());
}

void dspCustomerInformation::sEditInvoice()
{
  int  invcId		= _invoice->id();
  bool invcPosted	= false;
  XTreeWidgetItem *item = static_cast<XTreeWidgetItem*>(_invoice->currentItem());

  q.prepare("SELECT invchead_id AS id, invchead_posted AS posted "
            "FROM invchead "
            "WHERE (invchead_invcnumber=:docnum);");
  q.bindValue(":docnum", item->rawValue("invcnumber"));
  q.exec();
  if (q.first())
  {
    invcId	= q.value("id").toInt();
    invcPosted	= q.value("posted").toBool();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! invcPosted ||
      QMessageBox::question(this, tr("Edit Posted Invoice?"),
                              tr("<p>This Invoice has already been posted. "
                                 "Are you sure you want to edit it?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    invoice::editInvoice(invcId);
    sFillInvoiceList();
  }
  else
  {
    invoice::viewInvoice(invcId);
  }
}

void dspCustomerInformation::sViewInvoice()
{
  ParameterList params;
  XTreeWidgetItem *item = static_cast<XTreeWidgetItem*>(_invoice->currentItem());
  if(item)
  {
    params.append("invoiceNumber", item->rawValue("invcnumber"));
    dspInvoiceInformation* newdlg = new dspInvoiceInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    params.append("mode", "view");
    params.append("aropen_id", _invoice->id());
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspCustomerInformation::sPostInvoice()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

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
  sum.prepare("SELECT COALESCE(SUM(round((invcitem_billed * invcitem_qty_invuomratio) *"
	      "                 (invcitem_price / "
	      "                  CASE WHEN (item_id IS NULL) THEN 1"
	      "                       ELSE invcitem_price_invuomratio"
	      "                  END), 2)),0) + "
	      "       invchead_freight + invchead_tax + "
	      "       invchead_misc_amount AS subtotal "
	      "  FROM invchead LEFT OUTER JOIN invcitem ON (invcitem_invchead_id=invchead_id) LEFT OUTER JOIN"
	      "       item ON (invcitem_item_id=item_id) "
	      " WHERE(invchead_id=:invchead_id) "
	      " GROUP BY invchead_freight, invchead_tax, invchead_misc_amount;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE invchead SET invchead_gldistdate=:distdate "
		  "WHERE invchead_id=:invchead_id;");

  QList<QTreeWidgetItem *> selected = _invoice->selectedItems();
  QList<QTreeWidgetItem *> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
	{
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      if (changeDate)
      {
        setDate.bindValue(":distdate",    newDate);
        setDate.bindValue(":invchead_id", id);
        setDate.exec();
        if (setDate.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
        }
      }
	}
  }

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
	  {
        int id = ((XTreeWidgetItem*)(selected[i]))->id();

        sum.bindValue(":invchead_id", id);
        if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
        {
	      if (QMessageBox::question(this, tr("Invoice Has Value 0"),
		      		  tr("Invoice #%1 has a total value of 0.\n"
			     	     "Would you like to post it anyway?")
				    .arg(selected[i]->text(0)),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default)
	      == QMessageBox::No)
	        continue;
        }
        else if (sum.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
	      continue;
        }
        else if (sum.value("subtotal").toDouble() != 0)
        {
	      xrate.bindValue(":invchead_id", id);
	      xrate.exec();
	      if (xrate.lastError().type() != QSqlError::NoError)
	      {
	        systemError(this, tr("System Error posting Invoice #%1\n%2")
			            .arg(selected[i]->text(0))
			            .arg(xrate.lastError().databaseText()),
		                __FILE__, __LINE__);
	        continue;
	      }
	      else if (!xrate.first() || xrate.value("curr_rate").isNull())
	      {
	        systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
						.arg(selected[i]->text(0)));
	        continue;
	      }
        }

        post.bindValue(":invchead_id", id);
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
        {
	    if (changeDate)
	    {
	      triedToClosed = selected;
	      break;
	    }
	    else
	      triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::NoError)
	    systemError(this, tr("A System Error occurred posting Invoice #%1.\n%2")
	    	    .arg(selected[i]->text(0))
	     		.arg(post.lastError().databaseText()),
		        __FILE__, __LINE__);
    }

      if (triedToClosed.size() > 0)
      {
        failedPostList newdlg(this, "", true);
        newdlg.sSetList(triedToClosed, _invoice->headerItem(), _invoice->header());
        tryagain = (newdlg.exec() == XDialog::Accepted);
        selected = triedToClosed;
        triedToClosed.clear();
      }
	}
  } while (tryagain);

//  if (_printJournal->isChecked())
//  {
//    ParameterList params;
//    params.append("journalNumber", journal);
//
//    orReport report("SalesJournal", params);
//    if (report.isValid())
//      report.print();
//    else
//      report.reportError(this);
//  }

  omfgThis->sInvoicesUpdated(-1, TRUE);
}

void dspCustomerInformation::sFillCreditMemoList()
{
  disconnect(_creditMemo, SIGNAL(valid(bool)), this, SLOT(sHandleCreditMemoPrint()));
  _printCreditMemo->setEnabled(FALSE);

  QString sql( "SELECT cmhead_id, -1,"
             "       false AS posted, NULL AS open, '' AS type,"
             "       text(cmhead_number) AS docnumber,"
             "       cmhead_docdate AS docdate,"
             "       0 AS amount, 0 AS balance, '' AS currAbbr,"
             "       'curr' AS amount_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole "
             "  FROM cmhead"
             " WHERE ((NOT cmhead_posted)"
             "   AND  (cmhead_cust_id=:cust_id)) "
             "UNION "
             "SELECT aropen_id, -2,"
             "       true AS posted, aropen_open AS open,"
             "       :creditmemo AS type,"
             "       text(aropen_docnumber) AS docnumber,"
             "       aropen_docdate AS docdate,"
             "       aropen_amount AS amount,"
             "       (aropen_amount - aropen_paid) AS balance,"
             "       currConcat(aropen_curr_id) AS currAbbr,"
             "       'curr' AS amount_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole "
             "  FROM aropen, cmhead "
             " WHERE ((aropen_doctype = 'C')"
             "   AND  (aropen_docnumber=cmhead_number) ");
  if (!_creditMemoShowclosed->isChecked())
    sql +=   "   AND (aropen_open) ";

  sql +=     "   AND  (aropen_cust_id=:cust_id) ) "
             "UNION "
             "SELECT aropen_id, -3,"
             "       true AS posted, aropen_open AS open,"
             "       CASE WHEN (aropen_doctype='C') THEN :creditmemo"
             "            WHEN (aropen_doctype='R') THEN :cashdeposit"
             "            else aropen_doctype"
             "       END AS type,"
             "       text(aropen_docnumber) AS docnumber,"
             "       aropen_docdate AS docdate,"
             "       aropen_amount AS amount,"
             "       (aropen_amount - aropen_paid) AS balance,"
             "       currConcat(aropen_curr_id) AS currAbbr,"
             "       'curr' AS amount_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole "
             "  FROM aropen "
             " WHERE ((aropen_doctype IN ('C', 'R'))"
             "   AND  (aropen_cust_id=:cust_id) ";
  if (!_creditMemoShowclosed->isChecked())
    sql +=   "   AND (aropen_open) ";

  sql +=     "   AND  (NOT EXISTS (SELECT cmhead_id "
             "                     FROM cmhead "
             "                     WHERE (cmhead_number=aropen_docnumber) ) ) ) "
             "ORDER BY docnumber; ";

  q.prepare(sql);
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":creditmemo", tr("CM"));
  q.bindValue(":cashdeposit", tr("CD"));
  q.exec();
  _creditMemo->clear();
  _creditMemo->populate(q, true);

  connect(_creditMemo, SIGNAL(valid(bool)), this, SLOT(sHandleCreditMemoPrint()));
}

void dspCustomerInformation::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sEditCreditMemo()
{
  int  memoType		= _creditMemo->altId();
  int  memoId		= _creditMemo->id();
  bool memoPosted	= false;

  q.prepare("SELECT -1 AS type, cmhead_id AS id, cmhead_posted AS posted "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT -2 AS type, aropen_id AS id, aropen_posted AS posted "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype='C') "
            ") ORDER BY type DESC LIMIT 1;");
  q.bindValue(":docnum", _creditMemo->currentItem()->text(3));
  q.exec();
  if (q.first())
  {
    memoType	= q.value("type").toInt();
    memoId	= q.value("id").toInt();
    memoPosted	= q.value("posted").toBool();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  if (! memoPosted ||
      QMessageBox::question(this, tr("Edit Posted Memo?"),
                            tr("<p>This Credit Memo has already been posted. "
                               "Are you sure you want to edit it?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    params.append("mode", "edit");
  else
    params.append("mode", "view");

  if(memoType == -1)
  {
    params.append("cmhead_id", memoId);

    creditMemo *newdlg = new creditMemo();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    params.append("aropen_id", memoId);
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillCreditMemoList();
  }
}

void dspCustomerInformation::sViewCreditMemo()
{
  int memoType	= _creditMemo->altId();
  int memoId	= _creditMemo->id();

  q.prepare("SELECT -1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT -2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype='C') "
            ") ORDER BY type DESC LIMIT 1;");
  q.bindValue(":docnum", _creditMemo->currentItem()->text(3));
  q.exec();
  if (q.first())
  {
    memoType = q.value("type").toInt();
    memoId = q.value("id").toInt();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(memoType == -1)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("cmhead_id", memoId);

    creditMemo *newdlg = new creditMemo();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("aropen_id", memoId);
    arOpenItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillCreditMemoList();
  }
}

void dspCustomerInformation::sPostCreditMemo()
{
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

  QList<QTreeWidgetItem *> selected = _creditMemo->selectedItems();
  QList<QTreeWidgetItem *> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

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
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
    
  XSqlQuery postq;
  postq.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        int id = ((XTreeWidgetItem*)(selected[i]))->id();
 
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
          if (changeDate)
          {
            triedToClosed = selected;
            break;
          }
          else
            triedToClosed.append(selected[i]);
        }
        else if (postq.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, tr("A System Error occurred posting Credit Memo#%1.\n%2")
                  .arg(selected[i]->text(0))
                  .arg(postq.lastError().databaseText()),
                __FILE__, __LINE__);
        }
      }

      if (triedToClosed.size() > 0)
      {
        failedPostList newdlg(this, "", true);
        newdlg.sSetList(triedToClosed, _creditMemo->headerItem(), _creditMemo->header());
        tryagain = (newdlg.exec() == XDialog::Accepted);
        selected = triedToClosed;
        triedToClosed.clear();
      }
    }
  } while (tryagain);

  omfgThis->sCreditMemosUpdated();
}

void dspCustomerInformation::sEditAropen()
{
  ParameterList params;
  params.append("mode", "edit");
  if(_arhist->id() != -1)
    params.append("aropen_id", _arhist->id());
  else
    params.append("aropen_id", _arhist->altId());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerInformation::sViewAropen()
{
  ParameterList params;
  params.append("mode", "view");
  if(_arhist->id() != -1)
    params.append("aropen_id", _arhist->id());
  else
    params.append("aropen_id", _arhist->altId());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerInformation::sCashReceipt()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sFillPaymentsList()
{
  MetaSQLQuery mql = mqlLoad("ccpayments", "list");
  ParameterList params;
  params.append("cust_id",    _cust->id());
  params.append("preauth",    tr("Preauthorization"));
  params.append("charge",     tr("Charge"));
  params.append("refund",     tr("Refund"));
  params.append("authorized", tr("Authorized"));
  params.append("approved",   tr("Approved"));
  params.append("declined",   tr("Declined"));
  params.append("voided",     tr("Voided"));
  params.append("noapproval", tr("No Approval Code"));
  q = mql.toQuery(params);
  _payments->populate(q, true);

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCustomerInformation::sPopulateMenuInvoice( QMenu * pMenu,  QTreeWidgetItem *selected)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  int menuItem;
  menuItem = pMenu->insertItem(tr("New Invoice..."), this, SLOT(sNewInvoice()), 0);
  if(!_privileges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);
  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Invoice..."), this, SLOT(sEditInvoice()), 0);
  if(!_privileges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);
  menuItem = pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
  if(!_privileges->check("ViewMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);
  if(item->rawValue("posted").toBool() == FALSE)
  {
    menuItem = pMenu->insertItem(tr("Post Invoice..."), this, SLOT(sPostInvoice()), 0);
    if(!_privileges->check("PostMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  pMenu->insertSeparator();

  if (item->rawValue("ordernumber").toString().length() != 0)
  {
    menuItem = pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEditInvOrder()), 0);
    if(!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewInvOrder()), 0);
    if(!_privileges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    pMenu->insertItem(tr("Sales Order Shipment Status..."), this, SLOT(sInvShipmentStatus()), 0);
    pMenu->insertItem(tr("Sales Order Shipments..."), this, SLOT(sInvShipment()), 0);

  }
}

void dspCustomerInformation::sPopulateMenuCreditMemo( QMenu * pMenu, QTreeWidgetItem *selected )
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  int menuItem;

  menuItem = pMenu->insertItem(tr("New Credit Memo..."), this, SLOT(sNewCreditMemo()), 0);
  if(!_privileges->check("MaintainCreditMemos"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Credit Memo..."), this, SLOT(sEditCreditMemo()), 0);
  if(((_creditMemo->altId() == -1 || _creditMemo->altId() == -2) && !_privileges->check("MaintainCreditMemos"))
   ||(_creditMemo->altId() == -3 && !_privileges->check("EditAROpenItem")))
    pMenu->setItemEnabled(menuItem, FALSE);
  menuItem = pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
  if(!_privileges->check("ViewCreditMemos"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if(item->rawValue("posted").toBool() == FALSE)
  {
    menuItem = pMenu->insertItem(tr("Post Credit Memo..."), this, SLOT(sPostCreditMemo()), 0);
    if(!_privileges->check("PostARDocuments"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspCustomerInformation::sPopulateMenuArhist( QMenu * pMenu,  QTreeWidgetItem *selected)
{
  int menuItem;

  if (((XTreeWidgetItem *)selected)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit A/R Open Item..."), this, SLOT(sEditAropen()), 0);
    if(!_privileges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);
    menuItem = pMenu->insertItem(tr("View A/R Open Item..."), this, SLOT(sViewAropen()), 0);
    if(!_privileges->check("ViewAROpenItems"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}


void dspCustomerInformation::doDialog(QWidget * parent, const ParameterList & pParams)
{
  XDialog newdlg(parent);
  newdlg.setObjectName("dspCustomerInformationDialog");
  QVBoxLayout * vbox = new QVBoxLayout(&newdlg);
  dspCustomerInformation * ci = new dspCustomerInformation(&newdlg);
  vbox->addWidget(ci);
  ParameterList params;
  params = pParams;
  params.append("modal");
  ci->set(params);
  newdlg.exec();
}

void dspCustomerInformation::sCreditMemoSelected(bool b)
{
  if(b)
  {
    if(((_creditMemo->altId() == -1 || _creditMemo->altId() == -2) && _privileges->check("MaintainCreditMemos"))
     ||(_creditMemo->altId() == -3 && _privileges->check("EditAROpenItem")))
    {
      _editCreditMemo->setEnabled(true);
      _viewCreditMemo->setEnabled(true);
    }
    else if(((_creditMemo->altId() == -1 || _creditMemo->altId() == -2) && _privileges->check("ViewCreditMemos"))
     ||(_creditMemo->altId() == -3 && _privileges->check("ViewAROpenItems")))
    {
      _editCreditMemo->setEnabled(false);
      _viewCreditMemo->setEnabled(true);
    }
    else
    {
      _editCreditMemo->setEnabled(false);
      _viewCreditMemo->setEnabled(false);
    }
  }
  else
  {
    _editCreditMemo->setEnabled(false);
    _viewCreditMemo->setEnabled(false);
  }
}

void dspCustomerInformation::sARWorkbench()
{
  ParameterList params;
  params.append("cust_id", _cust->id());

  arWorkBench* newdlg = new arWorkBench();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspCustomerInformation::sCRMAccount()
{
  ParameterList params;
  if (!_privileges->check("MaintainCRMAccounts"))
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  params.append("crmacct_id", _crmacctId);

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomerInformation::sInvShipmentStatus()
{
  q.prepare("SELECT cohead_id "
            "FROM cohead, invchead "
            "WHERE ( (cohead_number=invchead_ordernumber) "
            "AND ( invchead_id=:invoice_id)) ");
  q.bindValue(":invoice_id", _invoice->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", q.value("cohead_id").toInt());
    params.append("run");

    dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
    newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  }
}

void dspCustomerInformation::sInvShipment()
{
  q.prepare("SELECT cohead_id "
            "FROM cohead, invchead "
            "WHERE ( (cohead_number=invchead_ordernumber) "
            "AND ( invchead_id=:invoice_id)) ");
  q.bindValue(":invoice_id", _invoice->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", q.value("cohead_id").toInt());

    dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspCustomerInformation::sPrintCCReceipt()
{
  CreditCardProcessor::printReceipt(_payments->id());
}

void dspCustomerInformation::sPrintInvoice()
{
  ParameterList params;
  params.append("invchead_id", _invoice->id());

  printInvoice newdlg(this, "", TRUE);
  newdlg.set(params);

  if (!newdlg.isSetup())
  {
    newdlg.exec();
    newdlg.setSetup(TRUE);
  }
}

void dspCustomerInformation::sPrintCreditMemo()
{
  int  memoId		= _creditMemo->id();

  q.prepare("SELECT cmhead_id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum);");
  q.bindValue(":docnum", _creditMemo->currentItem()->text(3));
  q.exec();
  if (q.first())
    memoId	= q.value("cmhead_id").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("cmhead_id", memoId);

  printCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (!newdlg.isSetup())
  {
    newdlg.exec();
    newdlg.setSetup(TRUE);
  }
}

void dspCustomerInformation::sHandleCreditMemoPrint()
{
  _printCreditMemo->setEnabled(_creditMemo->altId() == -1 || _creditMemo->altId() == -2);
}

void dspCustomerInformation::sPrintStatement()
{
	 if (!_cust->isValid())
	  {
	    QMessageBox::warning( this, tr("Enter a Valid Customer Number"),
	                          tr("You must enter a valid Customer Number for this Statement.") );
	    _cust->setFocus();
	    return;
	  }

	  q.prepare("SELECT findCustomerForm(:cust_id, 'S') AS _reportname;");
	  q.bindValue(":cust_id", _cust->id());
	  q.exec();
	  if (q.first())
	  {
	    ParameterList params;
	    params.append("cust_id", _cust->id());

	    orReport report(q.value("_reportname").toString(), params);
	    if (report.isValid())
		  report.print();
	    else
		  report.reportError(this);
	  }
}

void dspCustomerInformation::sHandleButtons()
{
  if (_notesButton->isChecked())
    _remarksStack->setCurrentIndex(0);
  else
    _remarksStack->setCurrentIndex(1);
    
  if (_contactsButton->isChecked())
    _crmStack->setCurrentIndex(0);
  else if (_todoListButton->isChecked())
    _crmStack->setCurrentIndex(1);
  else
    _crmStack->setCurrentIndex(2);
    
  if (_quotesButton->isChecked())
    _salesStack->setCurrentIndex(0);
  else if (_ordersButton->isChecked())
    _salesStack->setCurrentIndex(1);
  else
    _salesStack->setCurrentIndex(2);
}

bool dspCustomerInformation::checkSitePrivs(int invcid)
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


