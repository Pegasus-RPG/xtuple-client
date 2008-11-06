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

#include "dspInvoiceRegister.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "arOpenItem.h"
#include "creditMemo.h"
#include "dspInvoiceInformation.h"

dspInvoiceRegister::dspInvoiceRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _gltrans->addColumn(tr("Date"),     _dateColumn, Qt::AlignCenter,true, "transdate");
  _gltrans->addColumn(tr("Source"),  _orderColumn, Qt::AlignCenter,true, "gltrans_source");
  _gltrans->addColumn(tr("Doc Type"),_orderColumn, Qt::AlignLeft,  true, "doctype");
  _gltrans->addColumn(tr("Doc. #"),  _orderColumn, Qt::AlignCenter,true, "gltrans_docnumber");
  _gltrans->addColumn(tr("Reference"),         -1, Qt::AlignLeft,  true, "notes");
  _gltrans->addColumn(tr("Account"),  _itemColumn, Qt::AlignLeft,  true, "account");
  _gltrans->addColumn(tr("Debit"),   _moneyColumn, Qt::AlignRight, true, "debit");
  _gltrans->addColumn(tr("Credit"),  _moneyColumn, Qt::AlignRight, true, "credit");
}

dspInvoiceRegister::~dspInvoiceRegister()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInvoiceRegister::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInvoiceRegister::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _selectedAccount->setChecked(TRUE);
    _account->setId(param.toInt());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInvoiceRegister::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_gltrans->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
    if (! _privileges->check("MaintainMiscInvoices") &&
        ! _privileges->check("ViewMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  else if (_gltrans->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
        ! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_gltrans->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("View Debit Memo..."), this, SLOT(sViewCreditMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
        ! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_gltrans->altId() == 4)
  {
    menuItem = pMenu->insertItem(tr("View Customer Deposit..."), this, SLOT(sViewCreditMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
        ! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspInvoiceRegister::sViewCreditMemo()
{
  ParameterList params;
  params.append("mode", "view");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT 2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND  (aropen_doctype=:doctype)"
            ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _gltrans->currentItem()->text(3));
  if(_gltrans->altId()==1)
    q.bindValue(":doctype", "I");
  else if(_gltrans->altId()==2)
    q.bindValue(":doctype", "C");
  else if(_gltrans->altId()==3)
    q.bindValue(":doctype", "D");
  else if(_gltrans->altId()==4)
    q.bindValue(":doctype", "R");
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    if (_gltrans->altId() == 2)
      QMessageBox::information(this, tr("Credit Memo Not Found"),
                               tr("<p>The Credit Memo #%1 could not be found.")
                               .arg(_gltrans->currentItem()->text(3)));
    else if (_gltrans->altId() == 3)
      QMessageBox::information(this, tr("Debit Memo Not Found"),
                               tr("<p>The Debit Memo #%1 could not be found.")
                               .arg(_gltrans->currentItem()->text(3)));
    else
      QMessageBox::information(this, tr("Document Not Found"),
                               tr("<p>The Document #%1 could not be found.")
                               .arg(_gltrans->currentItem()->text(3)));
    return;
  }
}

void dspInvoiceRegister::sViewInvoice()
{
  ParameterList params;
  params.append("invoiceNumber", _gltrans->currentItem()->text(3));

  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspInvoiceRegister::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range.") );
    return false;
  }

  _dates->appendValue(params);

  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());

  params.append("invoice",      tr("Invoice"));
  params.append("creditmemo",   tr("Credit Memo"));
  params.append("debitmemo",    tr("Debit Memo"));
  params.append("cashdeposit",  tr("Customer Deposit"));

  return true;
}

void dspInvoiceRegister::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("InvoiceRegister", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInvoiceRegister::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(
     "SELECT DISTINCT"
     "       -1 AS gltrans_id, -1 AS altId,"
     "       gltrans_date,  '' AS gltrans_source,"
     "       '' AS doctype, '' AS gltrans_docnumber,"
     "       '' AS notes,   '' AS account,"
     "       CAST(NULL AS NUMERIC) AS debit,"
     "       CAST(NULL AS NUMERIC) AS credit,"
     "       'curr' AS debit_xtnumericrole,"
     "       'curr' AS credit_xtnumericrole,"
     "       0 AS xtindentrole,"
     "       gltrans_date AS transdate,"  // qtdisplay role isn't working right?
     "       1 AS debit_xttotalrole,"
     "       1 AS credit_xttotalrole "
     "FROM gltrans "
     "WHERE ((gltrans_doctype IN ('IN', 'CM', 'DM', 'CD'))"
     " AND (gltrans_source = 'A/R')"
     " AND (gltrans_date BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>) "
     "<? if exists(\"accnt_id\") ?>"
     " AND (gltrans_accnt_id=<? value(\"accnt_id\") ?>)"
     "<? endif ?>"
     ") "
     "UNION "
     "SELECT gltrans_id,"
     "       CASE WHEN(gltrans_doctype='IN') THEN 1"
     "            WHEN(gltrans_doctype='CM') THEN 2"
     "            WHEN(gltrans_doctype='DM') THEN 3"
     "            WHEN(gltrans_doctype='CD') THEN 4"
     "            ELSE -1"
     "       END AS altId,"
     "       gltrans_date, gltrans_source,"
     "       CASE WHEN(gltrans_doctype='IN') THEN <? value(\"invoice\") ?>"
     "            WHEN(gltrans_doctype='CM') THEN <? value(\"creditmemo\") ?>"
     "            WHEN(gltrans_doctype='DM') THEN <? value(\"debitmemo\") ?>"
     "            WHEN(gltrans_doctype='CD') THEN <? value(\"cashdeposit\") ?>"
     "            ELSE gltrans_doctype"
     "       END AS doctype,"
     "       gltrans_docnumber,"
     "       CASE WHEN(gltrans_doctype='IN') THEN"
     "                (SELECT invchead_shipto_name"
     "                   FROM aropen LEFT OUTER JOIN"
     "                        invchead"
     "                          ON (invchead_id=aropen_cobmisc_id"
     "                          AND invchead_cust_id=aropen_cust_id)"
     "                  WHERE ((aropen_docnumber=gltrans_docnumber)"
     "                    AND  (aropen_doctype='I')))"
     "            ELSE firstLine(gltrans_notes)"
     "       END AS f_notes,"
     "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS f_accnt,"
     "       CASE WHEN (gltrans_amount < 0) THEN ABS(gltrans_amount)"
     "            ELSE 0"
     "       END AS debit,"
     "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
     "            ELSE 0"
     "       END AS credit,"
     "       'curr' AS debit_xtnumericrole,"
     "       'curr' AS credit_xtnumericrole,"
     "       1 AS xtindentrole,"
     "       NULL AS transdate,"          // qtdisplay role isn't working right?
     "       0 AS debit_xttotalrole,"
     "       0 AS credit_xttotalrole "
     "FROM gltrans, accnt "
     "WHERE ((gltrans_accnt_id=accnt_id)"
     " AND (gltrans_doctype IN ('IN', 'CM', 'DM', 'CD'))"
     " AND (gltrans_source = 'A/R')"
     " AND (gltrans_date BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
     "<? if exists(\"accnt_id\") ?>"
     " AND (gltrans_accnt_id=<? value(\"accnt_id\") ?>)"
     "<? endif ?>"
     ") "
     "ORDER BY gltrans_date, gltrans_docnumber, xtindentrole;");

  q = mql.toQuery(params);
  _gltrans->populate(q, true);
  _gltrans->expandAll();

  // calculate subtotals for debit and credit columns and add rows for them
  for (int i = 0; i < _gltrans->topLevelItemCount(); i++)
  {
    double debitsum = 0.0;
    double creditsum = 0.0;
    XTreeWidgetItem *item = 0;
    for (int j = 0; j < _gltrans->topLevelItem(i)->childCount(); j++)
    {
      item = _gltrans->topLevelItem(i)->child(j);
      qDebug("in loop @ %d %p", j, item);
      if (item)
      {
        debitsum += item->rawValue("debit").toDouble();
        creditsum += item->rawValue("credit").toDouble();
      }
    }
    if (item)
    {
      qDebug("adding subtotal %p", item);
      item = new XTreeWidgetItem(_gltrans->topLevelItem(i), -1, -1, tr("Subtotal"));
      item->setData(_gltrans->column("debit"),  Qt::EditRole, formatMoney(debitsum));
      item->setData(_gltrans->column("credit"), Qt::EditRole, formatMoney(creditsum));
    }
  }
}
