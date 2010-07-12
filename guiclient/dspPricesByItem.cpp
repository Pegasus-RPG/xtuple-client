/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByItem.h"

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "mqlutil.h"

#define CURR_COL	5
#define COST_COL	6
#define MARGIN_COL	7

dspPricesByItem::dspPricesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _item->setType(ItemLineEdit::cSold);

  _price->addColumn(tr("Schedule"),      _itemColumn, Qt::AlignLeft,   true,  "schedulename"  );
  _price->addColumn(tr("Source"),        _itemColumn, Qt::AlignLeft,   true,  "type"  );
  _price->addColumn(tr("Customer/Customer Type"), -1, Qt::AlignLeft,   true,  "typename"  );
  _price->addColumn(tr("Qty. Break"),     _qtyColumn, Qt::AlignRight,  true,  "f_qtybreak" );
  _price->addColumn(tr("Price"),        _priceColumn, Qt::AlignRight,  true,  "price" );
  _price->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,   true,  "currConcat"  );
  _price->addColumn(tr("Cost"),          _costColumn, Qt::AlignRight,  true,  "f_cost" );
  _price->addColumn(tr("Margin"),       _prcntColumn, Qt::AlignRight,  true,  "f_margin" );

  if (omfgThis->singleCurrency())
    _price->hideColumn(CURR_COL);
  sHandleCosts(_showCosts->isChecked());

  _item->setFocus();
}

dspPricesByItem::~dspPricesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPricesByItem::languageChange()
{
  retranslateUi(this);
}

void dspPricesByItem::sPrint()
{
  ParameterList params;

  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_showFuture->isChecked())
    params.append("showFuture");

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_useActualCosts->isChecked())
    params.append("actualCosts");
  else /*if(_useStandardCosts->isChecked())*/
    params.append("standardCosts");

  orReport report("PricesByItem", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPricesByItem::sHandleCosts(bool pShowCosts)
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

void dspPricesByItem::sFillList()
{
  _price->clear();

  if (_item->isValid())
  {
    ParameterList params;
	if ( ! setParams(params))
      return;
    MetaSQLQuery mql = mqlLoad("prices", "detail");
    q = mql.toQuery(params);
    _price->populate(q, true);
  }
}

bool dspPricesByItem::setParams(ParameterList & params)
{
  double cost = 0.0;

  if (_showCosts->isChecked())
  {
    if (_useStandardCosts->isChecked())
      q.prepare( "SELECT (stdCost(item_id) * iteminvpricerat(item_id)) AS cost "
                 "FROM item "
                 "WHERE (item_id=:item_id);");
    else if (_useActualCosts->isChecked())
      q.prepare( "SELECT (actCost(item_id) * iteminvpricerat(item_id)) AS cost "
                 "FROM item "
                 "WHERE (item_id=:item_id);");

    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
      cost = q.value("cost").toDouble();
    else
      return false;
  }

  params.append("na", tr("N/A"));
  params.append("customer", tr("Customer"));
  params.append("shipTo", tr("Cust. Ship-To"));
  params.append("shipToPattern", tr("Cust. Ship-To Pattern"));
  params.append("custType", tr("Cust. Type"));
  params.append("custTypePattern", tr("Cust. Type Pattern"));
  params.append("sale", tr("Sale"));
  params.append("listPrice", tr("List Price"));
  params.append("item_id", _item->id());
  params.append("cost", cost);
  if (_showCosts->isChecked())
    params.append("showCosts");
  if (!_showExpired->isChecked())
    params.append("showExpired");
  if (!_showFuture->isChecked())
    params.append("showFuture");
  params.append("byItem");

  return true;
}
