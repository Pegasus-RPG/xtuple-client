/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByItem.h"

#include <openreports.h>
#include <parameter.h>

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
    }

    QString sql = "SELECT itemid, sourcetype, schedulename, type, typename,"
                  "       CASE WHEN (qtybreak <> -1) THEN qtybreak END AS f_qtybreak,"
                  "       price,"
                  "       currConcat(curr_id) AS currConcat,"
                  "       'qty' AS f_qtybreak_xtnumericrole,"
                  "       :na AS f_qtybreak_xtnullrole,"
                  "       'salesprice' AS price_xtnumericrole ";

    if (_showCosts->isChecked())
    {
      sql +=      "     , CASE WHEN (:cost <> 0) THEN :cost END AS f_cost,"
                  "       CASE WHEN ((price <> 0) AND (:cost <> 0)) THEN ((price - :cost) / price) END AS f_margin,"
                  "       'cost' AS f_cost_xtnumericrole,"
                  "       :costna AS f_cost_xtnullrole,"
                  "       'percent' AS f_margin_xtnumericrole,"
                  "       :na AS f_margin_xtnullrole,"
                  "       CASE WHEN (:cost > price) THEN 'error' END AS f_margin_qtforegroundrole ";
    }
    
      sql +=     "FROM ( "
                 "SELECT ipsprice_id AS itemid, 1 AS sourcetype,"
                 "       ipshead_name AS schedulename, :customer AS type,"
                 "       cust_name AS typename,"
                 "       ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, ipshead_curr_id AS curr_id "
                 "FROM ipsass, ipshead, ipsprice, cust, item "
                 "WHERE ( (ipsass_ipshead_id=ipshead_id)"
                 " AND (ipsprice_ipshead_id=ipshead_id)"
                 " AND (ipsass_cust_id=cust_id)"
                 " AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
                 " AND (ipsprice_item_id=item_id)"
                 " AND (item_id=:item_id)";

    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 2 AS sourcetype,"
           "             ipshead_name AS schedulename, :custType AS type,"
           "             (custtype_code || '-' || custtype_descrip) AS typename,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, ipshead_curr_id AS curr_id "
           "FROM ipsass, ipshead, ipsprice, custtype, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsass_custtype_id=custtype_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 3 AS sourcetype,"
           "             ipshead_name AS schedulename, :custTypePattern AS type,"
           "             (custtype_code || '-' || custtype_descrip) AS typename,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, ipshead_curr_id AS curr_id "
           "FROM ipsass, ipshead, ipsprice, custtype, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
           " AND (custtype_code ~ ipsass_custtype_pattern)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 4 AS sourcetype,"
           "             ipshead_name AS schedulename, :sale AS type,"
           "             sale_name AS typename,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, ipshead_curr_id AS curr_id "
           "FROM sale, ipshead, ipsprice, item "
           "WHERE ( (sale_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (sale_enddate > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (sale_startdate <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 5 AS sourcetype,"
           "             ipshead_name AS schedulename, :shipTo AS type,"
           "             (cust_name || '-' || shipto_num) AS typename,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, ipshead_curr_id AS curr_id "
           "FROM ipsass, ipshead, ipsprice, cust, shipto, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsass_shipto_id=shipto_id)"
           " AND (shipto_cust_id=cust_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 6 AS sourcetype,"
           "             ipshead_name AS schedulename, :shipToPattern AS type,"
           "             (cust_name || '-' || shipto_num) AS typename,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, ipshead_curr_id AS curr_id "
           "FROM ipsass, ipshead, ipsprice, cust, shipto, item "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (COALESCE(LENGTH(ipsass_shipto_pattern),0) > 0)"
           " AND (shipto_num ~ ipsass_shipto_pattern)"
           " AND (ipsass_cust_id=cust_id)"
           " AND (shipto_cust_id=cust_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_id=:item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT item_id AS itemid, 0 AS sourcetype,"
           "             :listPrice AS schedulename, :na AS type,"
           "             '' AS typename,"
           "             -1 AS qtybreak, item_listprice AS price, baseCurrId() AS curr_id "
           "FROM item "
           "WHERE ( (NOT item_exclusive)"
           " AND (item_id=:item_id) ) ) AS data "
           "ORDER BY price;";

    q.prepare(sql);
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":customer", tr("Customer"));
    q.bindValue(":shipTo", tr("Cust. Ship-To"));
    q.bindValue(":shipToPattern", tr("Cust. Ship-To Pattern"));
    q.bindValue(":custType", tr("Cust. Type"));
    q.bindValue(":custTypePattern", tr("Cust. Type Pattern"));
    q.bindValue(":sale", tr("Sale"));
    q.bindValue(":listPrice", tr("List Price"));
    q.bindValue(":item_id", _item->id());
    q.bindValue(":cost", cost);
    q.exec();
    _price->populate(q, true);
  }
}
