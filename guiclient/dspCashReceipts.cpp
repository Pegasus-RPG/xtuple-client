/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCashReceipts.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QErrorMessage>
#include <QWorkspace>
#include <QSqlError>

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
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  true, "base_applied"  );

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
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CashReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCashReceipts::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  _arapply->clear();

  if (_legacyDisplayType->isChecked())
  {
    MetaSQLQuery mql = mqlLoad("cashReceipts", "detail");
    q = mql.toQuery(params);
  }
  else
  {
    QErrorMessage errorMessageDialog(this);
//    errorMessageDialog.setWindowModality(Qt::WindowModal); 
    errorMessageDialog.showMessage(
      tr("This feature was introduced in version 3.3.\n"
         "Cash Receipts prior to this version will not be displayed."));
    MetaSQLQuery mql = mqlLoad("cashReceipts", "detailnew");
    q = mql.toQuery(params);
  }
  
  if (q.first())
    _arapply->populate(q);
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspCashReceipts::setParams(ParameterList &pParams)
{
  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose Cash Receipts you wish to view.") );
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
  
  _dates->appendValue(pParams);
  pParams.append("creditMemo", tr("C/M"));
  pParams.append("debitMemo", tr("D/M"));
  pParams.append("cashdeposit", tr("Cash Deposit"));
  pParams.append("invoice", tr("Invoice"));
  pParams.append("cash", tr("C/R"));
  pParams.append("check", tr("Check"));
  pParams.append("certifiedCheck", tr("Cert. Check"));
  pParams.append("masterCard", tr("M/C"));
  pParams.append("visa", tr("Visa"));
  pParams.append("americanExpress", tr("AmEx"));
  pParams.append("discoverCard", tr("Discover"));
  pParams.append("otherCreditCard", tr("Other C/C"));
  pParams.append("cash", tr("Cash"));
  pParams.append("wireTransfer", tr("Wire Trans."));
  pParams.append("other", tr("Other"));
  pParams.append("unapplied", tr("Cash Deposit"));
  pParams.append("unposted", tr("Unposted"));
  if (_selectedCustomer->isChecked())
    pParams.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    pParams.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    pParams.append("custtype_pattern", _customerType->text());
    
  return true;
}
