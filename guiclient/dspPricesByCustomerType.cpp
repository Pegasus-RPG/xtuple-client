/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByCustomerType.h"

#include <parameter.h>
#include <openreports.h>
#include <metasql.h>

#include "mqlutil.h"

#define CURR_COL	7
#define COST_COL	8
#define MARGIN_COL	9

dspPricesByCustomerType::dspPricesByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _custtype->setType(XComboBox::CustomerTypes);

  _price->addColumn(tr("Schedule"),    _itemColumn,     Qt::AlignLeft,   true,  "schedulename"  );
  _price->addColumn(tr("Source"),      _itemColumn,     Qt::AlignLeft,   true,  "type"  );
  _price->addColumn(tr("Item Number"), _itemColumn,     Qt::AlignLeft,   true,  "itemnumber"  );
  _price->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "itemdescrip"  );
  _price->addColumn(tr("Price UOM"),   _uomColumn,      Qt::AlignCenter, true,  "priceuom" );
  _price->addColumn(tr("Qty. Break"),  _qtyColumn,      Qt::AlignRight,  true,  "f_qtybreak" );
  _price->addColumn(tr("Price"),       _priceColumn,    Qt::AlignRight,  true,  "price" );
  _price->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currConcat" );
  _price->addColumn(tr("Ext. Cost"),   _costColumn,     Qt::AlignRight,  true,  "f_cost" );
  _price->addColumn(tr("Mar. %"),      _prcntColumn,    Qt::AlignRight,  true,  "f_margin" );

  if (omfgThis->singleCurrency())
    _price->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());
}

dspPricesByCustomerType::~dspPricesByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPricesByCustomerType::languageChange()
{
  retranslateUi(this);
}

void dspPricesByCustomerType::sPrint()
{
  ParameterList params;

  params.append("custtype_id", _custtype->id());

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

  orReport report("PricesByCustomerType", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPricesByCustomerType::sHandleCosts(bool pShowCosts)
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
  _costsGroup->setEnabled(pShowCosts);
}

void dspPricesByCustomerType::sFillList()
{
  _price->clear();

  if (_custtype->isValid())
  {
    ParameterList params;

    if (! setParams(params))
      return;

    MetaSQLQuery mql = mqlLoad("prices", "detail");
    q = mql.toQuery(params);
    _price->populate(q, true);
  }
}

bool dspPricesByCustomerType::setParams(ParameterList & params)
{
  params.append("na", tr("N/A"));
  params.append("costna",  tr("?????"));
  params.append("custType",  tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));
  params.append("listPrice", tr("List Price"));
  params.append("custtype_id", _custtype->id());

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

  params.append("byCustomerType");

  return true;
}
