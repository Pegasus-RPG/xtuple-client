/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByCustomer.h"

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "mqlutil.h"

#define CURR_COL	7
#define COST_COL	8
#define MARGIN_COL	9

dspPricesByCustomer::dspPricesByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _price->addColumn(tr("Schedule"),    _itemColumn,     Qt::AlignLeft,   true,  "schedulename"  );
  _price->addColumn(tr("Source"),      _itemColumn,     Qt::AlignLeft,   true,  "type"  );
  _price->addColumn(tr("Item Number"), _itemColumn,     Qt::AlignLeft,   true,  "itemnumber"  );
  _price->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "itemdescrip"  );
  _price->addColumn(tr("Price UOM"),   _uomColumn,      Qt::AlignCenter, true,  "priceuom");
  _price->addColumn(tr("Qty. Break"),  _qtyColumn,      Qt::AlignRight,  true,  "f_qtybreak" );
  _price->addColumn(tr("Price"),       _priceColumn,    Qt::AlignRight,  true,  "price" );
  _price->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currConcat");
  _price->addColumn(tr("Ext. Cost"),   _costColumn,     Qt::AlignRight,  true,  "f_cost" );
  _price->addColumn(tr("Mar. %"),      _prcntColumn,    Qt::AlignRight,  true,  "f_margin" );

  if (omfgThis->singleCurrency())
    _price->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());
}

dspPricesByCustomer::~dspPricesByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPricesByCustomer::languageChange()
{
  retranslateUi(this);
}

void dspPricesByCustomer::sPrint()
{
  ParameterList params;

  params.append("cust_id", _cust->id());

  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_showFuture->isChecked())
    params.append("showFuture");

  if(_showCosts->isChecked())
    params.append("showCosts");

  if(_useActualCosts->isChecked())
      params.append("actualCosts");
  else /*if(_useStandardCosts->isChecked())*/
      params.append("standardCosts");

  orReport report("PricesByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPricesByCustomer::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
  {
    _price->showColumn(COST_COL);
    _price->showColumn(MARGIN_COL);
  }
  else
  {
    _price->hideColumn(COST_COL);
    _price->hideColumn(MARGIN_COL);
  }
  _costsGroup->setEnabled(_showCosts->isChecked());
}

void dspPricesByCustomer::sFillList()
{
  _price->clear();

  if (_cust->isValid())
  {
    ParameterList params;
    if (! setParams(params))
      return;

    MetaSQLQuery mql = mqlLoad("prices", "detail");
    q = mql.toQuery(params);
    _price->populate(q, true);
  }
}

bool dspPricesByCustomer::setParams(ParameterList & params)
{
  params.append("na", tr("N/A"));
  params.append("costna", tr("?????"));
  params.append("customer", tr("Customer"));
  params.append("custType", tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));
  params.append("listPrice", tr("List Price"));
  params.append("cust_id", _cust->id());
  params.append("byCustomer");

  if (_showCosts->isChecked())
  {
    params.append("showCosts");
    if (_useStandardCosts->isChecked())
      params.append("useStandardCosts");
    else if (_useActualCosts->isChecked())
      params.append("useActualCosts");
  }

  if (!_showExpired->isChecked())
    params.append("showExpired");

  if (!_showFuture->isChecked())
    params.append("showFuture");

  return true;
}
