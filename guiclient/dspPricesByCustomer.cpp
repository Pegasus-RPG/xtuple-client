/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPricesByCustomer.h"

#include <openreports.h>
#include <parameter.h>

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
    QString sql = "SELECT itemid, sourcetype, schedulename, type, itemnumber, priceuom, itemdescrip,"
                  "       CASE WHEN (qtybreak <> -1) THEN qtybreak END AS f_qtybreak,"
                  "       price, "
                  "       currConcat(curr_id) AS currConcat,"
                  "       'qty' AS f_qtybreak_xtnumericrole,"
                  "       :na AS f_qtybreak_xtnullrole,"
                  "       'salesprice' AS price_xtnumericrole ";

    if (_showCosts->isChecked())
    {
      sql +=      "     , CASE WHEN (cost IS NOT NULL) THEN cost END AS f_cost,"
                  "       CASE WHEN ((price <> 0) AND (cost <>0)) THEN ((price - cost) / price) END AS f_margin,"
                  "       'cost' AS f_cost_xtnumericrole,"
                  "       :costna AS f_cost_xtnullrole,"
                  "       'percent' AS f_margin_xtnumericrole,"
                  "       :na AS f_margin_xtnullrole,"
                  "       CASE WHEN (cost > price) THEN 'error' END AS f_margin_qtforegroundrole ";
    }
    
    sql += "FROM ( SELECT ipsprice_id AS itemid, 1 AS sourcetype,"
           "              ipshead_name AS schedulename, :customer AS type,"
           "              item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "              (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "              ipsprice_qtybreak AS qtybreak, "
	   "		  ipsprice_price AS price, ipshead_curr_id AS curr_id, "
	   "		  ipshead_updated AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM ipsass, ipshead, ipsprice, item, uom "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (ipsass_cust_id=:cust_id)"
           " AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0) ";

    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 2 AS sourcetype,"
           "             ipshead_name AS schedulename, :custType AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, "
	   "		 ipshead_curr_id AS curr_id, "
	   "		 ipshead_updated AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM ipsass, ipshead, ipsprice, item, uom, cust "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (ipsass_custtype_id=cust_custtype_id)"
           " AND (cust_id=:cust_id) ";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 3 AS sourcetype,"
           "             ipshead_name AS schedulename, :custTypePattern AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, "
	   "		 ipshead_curr_id AS curr_id, "
	   "		 ipshead_updated AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM ipsass, ipshead, ipsprice, item, uom, cust, custtype "
           "WHERE ( (ipsass_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (ipsprice_item_id=item_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (cust_custtype_id=custtype_id)"
           " AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
           " AND (custtype_code ~ ipsass_custtype_pattern)"
           " AND (cust_id=:cust_id) ";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsprice_id AS itemid, 4 AS sourcetype,"
           "             ipshead_name AS schedulename, (:sale || '-' || sale_name) AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             ipsprice_qtybreak AS qtybreak, ipsprice_price AS price, "
	   "		 ipshead_curr_id AS curr_id, "
	   "		 ipshead_updated AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM sale, ipshead, ipsprice, item, uom "
           "WHERE ((sale_ipshead_id=ipshead_id)"
           " AND (ipsprice_ipshead_id=ipshead_id)"
           " AND (item_price_uom_id=uom_id)"
           " AND (ipsprice_item_id=item_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (sale_enddate > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (sale_startdate <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT item_id AS itemid, 0 AS sourcetype,"
           "             '' AS schedulename, :listPrice AS type,"
           "             item_number AS itemnumber, uom_name AS priceuom, iteminvpricerat(item_id) AS invpricerat,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
           "             -1 AS qtybreak, item_listprice AS price, "
	   "		 baseCurrId() AS curr_id, "
	   "		 CURRENT_DATE AS effective ";

    if (_showCosts->isChecked())
    {
      if (_useStandardCosts->isChecked())
        sql += ", (stdcost(item_id) * iteminvpricerat(item_id)) AS cost ";
      else if (_useActualCosts->isChecked())
        sql += ", (actcost(item_id) * iteminvpricerat(item_id)) AS cost ";
    }

    sql += "FROM item JOIN uom ON (item_price_uom_id=uom_id)"
           "WHERE ( (item_sold)"
           " AND (NOT item_exclusive) ) ) AS data "
           "ORDER BY itemnumber, price;";

    q.prepare(sql);
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":costna", tr("?????"));
    q.bindValue(":customer", tr("Customer"));
    q.bindValue(":custType", tr("Cust. Type"));
    q.bindValue(":custTypePattern", tr("Cust. Type Pattern"));
    q.bindValue(":sale", tr("Sale"));
    q.bindValue(":listPrice", tr("List Price"));
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    _price->populate(q, true);
  }
}
