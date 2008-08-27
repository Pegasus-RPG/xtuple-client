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

#include "dspARApplications.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>

#include <datecluster.h>
#include <metasql.h>
#include "mqlutil.h"
#include <parameter.h>
#include <openreports.h>

#include "arOpenItem.h"
#include "creditMemo.h"
#include "dspInvoiceInformation.h"

dspARApplications::dspARApplications(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_arapply,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
                  this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print,	SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query,	SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
    
  _arapply->addColumn(tr("Cust. #"),        _orderColumn, Qt::AlignCenter, true,  "cust_number" );
  _arapply->addColumn(tr("Customer"),                 -1, Qt::AlignLeft,   true,  "cust_name"   );
  _arapply->addColumn(tr("Date"),            _dateColumn, Qt::AlignCenter, true,  "arapply_postdate" );
  _arapply->addColumn("hidden source type",           10, Qt::AlignCenter, true,  "arapply_source_doctype" );
  _arapply->addColumn(tr("Source"),	         _itemColumn, Qt::AlignCenter, true,  "doctype" );
  _arapply->addColumn(tr("Doc #"),          _orderColumn, Qt::AlignCenter, true,  "source" );
  _arapply->addColumn("hidden target type",           10, Qt::AlignCenter, true,  "arapply_target_doctype" );
  _arapply->addColumn(tr("Apply-To"),        _itemColumn, Qt::AlignCenter, true,  "targetdoctype" );
  _arapply->addColumn(tr("Doc #"),          _orderColumn, Qt::AlignCenter, true,  "target" );
  _arapply->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight,  true,  "arapply_applied"  );
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  false, "base_applied"  );

  _arapply->hideColumn(3);
  _arapply->hideColumn(6);

  _allCustomers->setFocus();
}

dspARApplications::~dspARApplications()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspARApplications::languageChange()
{
  retranslateUi(this);
}

void dspARApplications::sPrint()
{
  if (!checkParams())
    return;

  ParameterList params;
  setParams(params);

  orReport report("ARApplications", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspARApplications::sViewCreditMemo()
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
	    "  AND (aropen_doctype IN ('C', 'R')) "
	    ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _arapply->currentItem()->text(5));
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
      params.append("docType", "creditMemo");
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
			     tr("<p>The Credit Memo #%1 could not be found.")
			     .arg(_arapply->currentItem()->text(5)));
    return;
  }
}

void dspARApplications::sViewDebitMemo()
{
  ParameterList params;

  params.append("mode", "view");
  q.prepare("SELECT aropen_id "
	    "FROM aropen "
	    "WHERE ((aropen_docnumber=:docnum) AND (aropen_doctype='D'));");
  q.bindValue(":docnum", _arapply->currentItem()->text(8));
  q.exec();
  if (q.first())
  {
    params.append("aropen_id", q.value("aropen_id"));
    params.append("docType", "debitMemo");
    arOpenItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspARApplications::sViewInvoice()
{
  ParameterList params;

  params.append("mode", "view");
  params.append("invoiceNumber", _arapply->currentItem()->text(8));
  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspARApplications::sPopulateMenu(QMenu* pMenu)
{
  int menuItem;

  if (_arapply->currentItem()->text(3) == "C")
  {
    menuItem = pMenu->insertItem(tr("View Source Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
	! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if (_arapply->currentItem()->text(6) == "D")
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Debit Memo..."), this, SLOT(sViewDebitMemo()), 0);
    if (! _privileges->check("MaintainARMemos") &&
	! _privileges->check("ViewARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_arapply->currentItem()->text(6) == "I")
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Invoice..."), this, SLOT(sViewInvoice()), 0);
    if (! _privileges->check("MaintainMiscInvoices") &&
	! _privileges->check("ViewMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspARApplications::sFillList()
{
  if (!checkParams())
    return;
    
  _arapply->clear();

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql = mqlLoad(":/ar/displays/arApplications.mql");
  q = mql.toQuery(params);
  if (q.first())
    _arapply->populate(q);
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspARApplications::checkParams()
{
  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose A/R Applications you wish to view.") );
    _cust->setFocus();
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  if ( (!_cashReceipts->isChecked()) && (!_creditMemos->isChecked()) )
  {
    QMessageBox::critical( this, tr("Select Document Type"),
                           tr("You must indicate which Document Type(s) you wish to view.") );
    _cashReceipts->setFocus();
    return false;
  }
  
  return true; 
}

void dspARApplications::setParams(ParameterList & params)
{
  if (_cashReceipts->isChecked())
    params.append("includeCashReceipts");

  if (_creditMemos->isChecked())
    params.append("includeCreditMemos");

  _dates->appendValue(params);
  params.append("creditMemo", tr("C/M"));
  params.append("debitMemo", tr("D/M"));
  params.append("cashdeposit", tr("Cash Deposit"));
  params.append("invoice", tr("Invoice"));
  params.append("cash", tr("C/R"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Cert. Check"));
  params.append("masterCard", tr("M/C"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("AmEx"));
  params.append("discoverCard", tr("Discover"));
  params.append("otherCreditCard", tr("Other C/C"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Trans."));
  params.append("other", tr("Other"));
	params.append("apcheck", tr("A/P Check"));

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());
}