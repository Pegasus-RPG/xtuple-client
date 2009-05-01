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
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include <currcluster.h>

#include "arOpenItem.h"
#include "creditMemo.h"
#include "dspInvoiceInformation.h"
#include "invoice.h"
#include "incident.h"

dspAROpenItems::dspAROpenItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  QString baseBalanceTitle(tr("Balance"));
  if (! omfgThis->singleCurrency())
    baseBalanceTitle = tr("Balance\n(in %1)").arg(CurrDisplay::baseCurrAbbr());

  _aropen->setRootIsDecorated(TRUE);
  _aropen->addColumn(tr("Doc. Type"),     _itemColumn, Qt::AlignLeft,   true,  "doctype");
  _aropen->addColumn(tr("Posted"),          _ynColumn, Qt::AlignCenter, false, "posted");
  _aropen->addColumn(tr("Recurring"),       _ynColumn, Qt::AlignCenter, false, "recurring");
  _aropen->addColumn(tr("Open"),            _ynColumn, Qt::AlignCenter, false, "open");
  _aropen->addColumn(tr("Doc. #"),       _orderColumn, Qt::AlignLeft,   true,  "docnumber");
  _aropen->addColumn(tr("Cust./Assign To"),_itemColumn, Qt::AlignLeft,  true,  "cust_number");
  _aropen->addColumn(tr("Name/Desc."),             -1, Qt::AlignLeft,   true,  "cust_name");
  _aropen->addColumn(tr("Order/Incident"),_itemColumn, Qt::AlignRight,  false, "ordernumber");
  _aropen->addColumn(tr("Doc. Date"),     _dateColumn, Qt::AlignCenter, false, "docdate");
  _aropen->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignCenter, true,  "aropen_duedate");
  _aropen->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight,  false, "amount");
  _aropen->addColumn(tr("Paid"),      _bigMoneyColumn, Qt::AlignRight,  false, "paid");
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight,  true,  "balance");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,   true,  "currAbbr");
  _aropen->addColumn(baseBalanceTitle,_bigMoneyColumn, Qt::AlignRight,  true,  "base_balance");

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn("currAbbr");
    _aropen->hideColumn("base_balance");
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _unposted->hide();
  _closed->hide();
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
  int menuItem;

  if (((XTreeWidgetItem *)pItem)->id() != -1)
  {
    if (((XTreeWidgetItem *)pItem)->id() > 0)
    {
      menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
      if (!_privileges->check("EditAROpenItem"))
        pMenu->setItemEnabled(menuItem, FALSE);

      pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    }

    if (((XTreeWidgetItem *)pItem)->altId() == 0)
    {
      pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
      pMenu->insertItem(tr("View Invoice Details..."), this, SLOT(sViewInvoiceDetails()), 0);
    }
    else if (((XTreeWidgetItem *)pItem)->id() == 1 && ((XTreeWidgetItem *)pItem)->id("source_id") != -1)
    {
      pMenu->insertItem(tr("Edit Credit Memo..."), this, SLOT(sEditCreditMemo()), 0);
      if (!_privileges->check("MaintainCreditMemo"))
        pMenu->setItemEnabled(menuItem, FALSE);

      pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
      if (!_privileges->check("MaintainCreditMemo") && !_privileges->check("ViewCreditMemo"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    
   pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("New Incident..."), this, SLOT(sIncident()), 0);
    if (!_privileges->check("AddIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    pMenu->insertSeparator();
    
    menuItem = pMenu->insertItem(tr("Edit Incident..."), this, SLOT(sEditIncident()), 0);
    if (!_privileges->check("MaintainIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View Incident..."), this, SLOT(sViewIncident()), 0);
    if (!_privileges->check("ViewIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspAROpenItems::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
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

void dspAROpenItems::sEditCreditMemo()
{
  ParameterList params;
  params.append("cmhead_id", _aropen->id("source_id"));
  params.append("mode", "edit");
  creditMemo* newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sViewCreditMemo()
{
  ParameterList params;
  params.append("cmhead_id", _aropen->id("source_id"));
  params.append("mode", "view");
  creditMemo* newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sViewInvoice()
{
  ParameterList params;
  params.append("invoiceNumber", _aropen->id("source_number"));
  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAROpenItems::sViewInvoiceDetails()
{
  ParameterList params;
  params.append("invchead_id", _aropen->id("source_id"));
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

void dspAROpenItems::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItems::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _aropen->altId());
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
  return true;
}

void dspAROpenItems::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("AROpenItems", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
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


