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
 * The Original Code is xTuple ERP: PostBooks Edition 
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
 * Powered by xTuple ERP: PostBooks Edition
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

#include "dspAROpenItemsByCustomer.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "arOpenItem.h"
#include "dspInvoiceInformation.h"
#include "invoice.h"
#include "incident.h"
#include "mqlutil.h"

dspAROpenItemsByCustomer::dspAROpenItemsByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  QString baseBalanceTitle(tr("Balance"));
  if (! omfgThis->singleCurrency())
    baseBalanceTitle = tr("Balance\n(in %1)").arg(CurrDisplay::baseCurrAbbr());

  _aropen->addColumn(tr("Doc. Type"),     _itemColumn, Qt::AlignLeft,  true, "doctype");
  _aropen->addColumn(tr("Doc. #"),       _orderColumn, Qt::AlignLeft,  true, "aropen_docnumber");
  _aropen->addColumn(tr("Order/Incdt."),  _itemColumn, Qt::AlignLeft,  true, "aropen_ordernumber");
  _aropen->addColumn(tr("Doc. Date"),     _dateColumn, Qt::AlignCenter,true, "aropen_docdate");
  _aropen->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignCenter,true, "aropen_duedate");
  _aropen->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight, true, "aropen_amount");
  _aropen->addColumn(tr("Paid"),      _bigMoneyColumn, Qt::AlignRight, true, "paid");
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight, true, "balance");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  true, "currAbbr");
  _aropen->addColumn(baseBalanceTitle,_bigMoneyColumn, Qt::AlignRight, true, "base_balance");

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn("currAbbr");
    _aropen->hideColumn("base_balance");
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _cust->setFocus();
}

dspAROpenItemsByCustomer::~dspAROpenItemsByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspAROpenItemsByCustomer::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAROpenItemsByCustomer::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
    
  param = pParams.value("asofDate", &valid);
  if (valid)
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(FALSE);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}
 
void dspAROpenItemsByCustomer::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  if (((XTreeWidgetItem *)pItem)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    
    XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
    {
      if (item->text(0) == "Invoice")
      {
        pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
        pMenu->insertItem(tr("View Invoice Details..."), this, SLOT(sViewInvoiceDetails()), 0);
      }
    }
    
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("New Incident..."), this, SLOT(sIncident()), 0);
    if (!_privileges->check("AddIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Edit Incident..."), this, SLOT(sEditIncident()), 0);
    if (!_privileges->check("MaintainIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View Incident..."), this, SLOT(sViewIncident()), 0);
    if (!_privileges->check("ViewIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspAROpenItemsByCustomer::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sViewInvoice()
{
  q.prepare("SELECT aropen_docnumber FROM aropen WHERE (aropen_id=:aropen_id);");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("invoiceNumber", q.value("aropen_docnumber"));
    dspInvoiceInformation* newdlg = new dspInvoiceInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItemsByCustomer::sViewInvoiceDetails()
{
  q.prepare("SELECT invchead_id FROM aropen, invchead WHERE ((aropen_id=:aropen_id) AND (invchead_invcnumber=aropen_docnumber));");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first());
  {
    ParameterList params;
    params.append("invchead_id", q.value("invchead_id"));
    params.append("mode", "view");
    invoice* newdlg = new invoice();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItemsByCustomer::sIncident()
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
    incident newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void dspAROpenItemsByCustomer::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool dspAROpenItemsByCustomer::setParams(ParameterList &params)
{
  _dates->appendValue(params);
  params.append("asofDate",     _asOf->date());
  params.append("cust_id",      _cust->id());
  params.append("invoice",      tr("Invoice"));
  params.append("creditMemo",   tr("Credit Memo"));
  params.append("debitMemo",    tr("Debit Memo"));
  params.append("cashdeposit",  tr("Customer Deposit"));

  return true;
}

void dspAROpenItemsByCustomer::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("AROpenItemsByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAROpenItemsByCustomer::sFillList()
{
  MetaSQLQuery mql = mqlLoad("arOpenItemsByCustomer", "detail");
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
