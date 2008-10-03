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

#include "dspCashReceipts.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>

#include <metasql.h>
#include "mqlutil.h"

#include <datecluster.h>
#include <openreports.h>

/*
 *  Constructs a dspCashReceipts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCashReceipts::dspCashReceipts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_selectedCustomer, SIGNAL(toggled(bool)), _cust, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _customerTypes->setType(XComboBox::CustomerTypes);
  
  _arapply->addColumn(tr("Cust. #"),     _orderColumn,    Qt::AlignCenter, true,  "cust_number" );
  _arapply->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _arapply->addColumn(tr("Date"),        _dateColumn,     Qt::AlignCenter, true,  "postdate" );
  _arapply->addColumn(tr("Source"),      _itemColumn,     Qt::AlignCenter, true,  "source" );
  _arapply->addColumn(tr("Apply-To"),    _itemColumn,     Qt::AlignCenter, true,  "target" );
  _arapply->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight,  true,  "applied"  );
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  false, "base_applied"  );

  _allCustomers->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCashReceipts::~dspCashReceipts()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCashReceipts::languageChange()
{
  retranslateUi(this);
}

void dspCashReceipts::sPrint()
{
  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose A/R Applications you wish to view.") );
    _cust->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  orReport report("CashReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCashReceipts::sFillList()
{
  _arapply->clear();

  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose Cash Receipts you wish to view.") );
    _cust->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  MetaSQLQuery mql = mqlLoad("cashReceipts", "detail");
  ParameterList params;
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
  params.append("unapplied", tr("Cash Deposit"));
  q = mql.toQuery(params);
  if (q.first())
    _arapply->populate(q);
}

