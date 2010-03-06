/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspEarnedCommissions.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <parameter.h>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include "guiclient.h"

/*
 *  Constructs a dspEarnedCommissions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspEarnedCommissions::dspEarnedCommissions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_selectedSalesrep, SIGNAL(toggled(bool)), _salesrep, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _salesrep->setType(XComboBox::SalesReps);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _commission->addColumn(tr("Sales Rep."),      100,             Qt::AlignLeft,   true,  "salesrep_name"   );
  _commission->addColumn(tr("S/O #"),           _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _commission->addColumn(tr("Cust. #"),         _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  _commission->addColumn(tr("Ship-To"),         -1,              Qt::AlignLeft,   true,  "cohist_shiptoname"   );
  _commission->addColumn(tr("Invc. Date"),      _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _commission->addColumn(tr("Item Number"),     _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  _commission->addColumn(tr("Qty."),            _qtyColumn,      Qt::AlignRight,  true,  "cohist_qtyshipped"  );
  _commission->addColumn(tr("Ext. Price"),      _moneyColumn,    Qt::AlignRight,  true,  "extprice"  );
  _commission->addColumn(tr("Commission"),      _moneyColumn,    Qt::AlignRight,  true,  "cohist_commission"  );
  _commission->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _commission->addColumn(tr("Base Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice"  );
  _commission->addColumn(tr("Base Commission"), _bigMoneyColumn, Qt::AlignRight,  true,  "basecommission"  );
  _commission->addColumn(tr("Paid"),            _ynColumn,       Qt::AlignCenter, true,  "cohist_commissionpaid" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspEarnedCommissions::~dspEarnedCommissions()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspEarnedCommissions::languageChange()
{
  retranslateUi(this);
}

void dspEarnedCommissions::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start and End Date"),
                          tr("You must enter a valid Start and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());
  if (_includeMisc->isChecked())
    params.append("includeMisc");
  params.append("includeFormatted");

  orReport report("EarnedCommissions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspEarnedCommissions::sFillList()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start and End Date"),
                          tr("You must enter a valid Start and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  _commission->clear();

  MetaSQLQuery mql = mqlLoad("salesHistory", "detail");
  ParameterList params;
  _dates->appendValue(params);
  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());
  if (_includeMisc->isChecked())
    params.append("includeMisc");
  params.append("orderBySalesRepInvcdate");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _commission->populate(q);
}
