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

#include "dspFreightPricesByCustomerType.h"

#include <parameter.h>
#include <openreports.h>

dspFreightPricesByCustomerType::dspFreightPricesByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _custtype->setType(XComboBox::CustomerTypes);

  _price->addColumn(tr("Schedule"),      _itemColumn,     Qt::AlignLeft,   true,  "ipshead_name"  );
  _price->addColumn(tr("Source"),        _itemColumn,     Qt::AlignLeft,   true,  "source"  );
  _price->addColumn(tr("Qty. Break"),    _qtyColumn,      Qt::AlignRight,  true,  "ipsfreight_qtybreak" );
  _price->addColumn(tr("Price"),         _priceColumn,    Qt::AlignRight,  true,  "ipsfreight_price" );
  _price->addColumn(tr("Method"),        _itemColumn,     Qt::AlignLeft,   true,  "method"  );
  _price->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,   true,  "currConcat");
  _price->addColumn(tr("From"),          _itemColumn,     Qt::AlignLeft,   true,  "warehous_code"  );
  _price->addColumn(tr("To"),            _itemColumn,     Qt::AlignLeft,   true,  "shipzone_name"  );
  _price->addColumn(tr("Freight Class"), _itemColumn,     Qt::AlignLeft,   true,  "freightclass_code"  );
  _price->addColumn(tr("Ship Via"),      _itemColumn,     Qt::AlignLeft,   true,  "ipsfreight_shipvia"  );

}

dspFreightPricesByCustomerType::~dspFreightPricesByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspFreightPricesByCustomerType::languageChange()
{
  retranslateUi(this);
}

void dspFreightPricesByCustomerType::sPrint()
{
  ParameterList params;

  params.append("custtype_id", _custtype->id());

  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_showFuture->isChecked())
    params.append("showFuture");

  orReport report("FreightPricesByCustomerType", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspFreightPricesByCustomerType::sFillList()
{
  _price->clear();

  if (_custtype->isValid())
  {
    QString sql = "SELECT itemid, sourcetype, ipshead_name, source, ipsfreight_qtybreak, ipsfreight_price,"
                  "       CASE WHEN (ipsfreight_type = 'F') THEN :flatrate"
                  "            ELSE :peruom"
                  "       END AS method,"
                  "       currConcat(ipshead_curr_id) AS currConcat,"
                  "       warehous_code, shipzone_name, freightclass_code, ipsfreight_shipvia,"
                  "       'qty' AS ipsfreight_qtybreak_xtnumericrole,"
                  "       :na AS ipsfreight_qtybreak_xtnullrole,"
                  "       'salesprice' AS ipsfreight_price_xtnumericrole,"
                  "       :any AS warehous_code_xtnullrole,"
                  "       :any AS shipzone_name_xtnullrole,"
                  "       :any AS freightclass_code_xtnullrole,"
                  "       :any AS ipsfreight_shipvia_xtnullrole ";

    sql += "FROM ( SELECT ipsfreight_id AS itemid, 1 AS sourcetype,"
           "              ipshead_name, :custType AS source,"
           "              ipsfreight_qtybreak, ipsfreight_price,"
           "              ipsfreight_type, ipshead_curr_id,"
           "              warehous_code, shipzone_name, freightclass_code, ipsfreight_shipvia "
           "FROM ipsass JOIN ipshead ON (ipshead_id=ipsass_ipshead_id)"
           "            JOIN ipsfreight ON (ipsfreight_ipshead_id=ipshead_id)"
           "            LEFT OUTER JOIN whsinfo ON (warehous_id=ipsfreight_warehous_id)"
           "            LEFT OUTER JOIN shipzone ON (shipzone_id=ipsfreight_shipzone_id)"
           "            LEFT OUTER JOIN freightclass ON (freightclass_id=ipsfreight_freightclass_id) "
           "WHERE ( (ipsass_custtype_id=:custtype_id)";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsfreight_id AS itemid, 2 AS sourcetype,"
           "             ipshead_name, :custTypePattern AS source,"
           "             ipsfreight_qtybreak, ipsfreight_price,"
           "             ipsfreight_type, ipshead_curr_id,"
           "             warehous_code, shipzone_name, freightclass_code, ipsfreight_shipvia "
           "FROM custtype JOIN ipsass ON ((coalesce(length(ipsass_custtype_pattern), 0) > 0) AND"
           "                              (custtype_code ~ ipsass_custtype_pattern))"
           "              JOIN ipshead ON (ipshead_id=ipsass_ipshead_id)"
           "              JOIN ipsfreight ON (ipsfreight_ipshead_id=ipshead_id)"
           "              LEFT OUTER JOIN whsinfo ON (warehous_id=ipsfreight_warehous_id)"
           "              LEFT OUTER JOIN shipzone ON (shipzone_id=ipsfreight_shipzone_id)"
           "              LEFT OUTER JOIN freightclass ON (freightclass_id=ipsfreight_freightclass_id) "
           "WHERE ( (custtype_id=:custtype_id) ";
                  
    if (!_showExpired->isChecked())
      sql += " AND (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE)";

    sql += ") "
           "UNION SELECT ipsfreight_id AS itemid, 3 AS sourcetype,"
           "             ipshead_name, (:sale || '-' || sale_name) AS source,"
           "             ipsfreight_qtybreak, ipsfreight_price,"
           "             ipsfreight_type, ipshead_curr_id,"
           "             warehous_code, shipzone_name, freightclass_code, ipsfreight_shipvia "
           "FROM sale JOIN ipshead ON (ipshead_id=sale_ipshead_id)"
           "          JOIN ipsfreight ON (ipsfreight_ipshead_id=ipshead_id)"
           "          LEFT OUTER JOIN whsinfo ON (warehous_id=ipsfreight_warehous_id)"
           "          LEFT OUTER JOIN shipzone ON (shipzone_id=ipsfreight_shipzone_id)"
           "          LEFT OUTER JOIN freightclass ON (freightclass_id=ipsfreight_freightclass_id) "
           "WHERE ((TRUE)";

    if (!_showExpired->isChecked())
      sql += " AND (sale_enddate > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (sale_startdate <= CURRENT_DATE)";

    sql += ") ) AS data "
           "ORDER BY ipsfreight_qtybreak, ipsfreight_price;";

    q.prepare(sql);
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":any", tr("Any"));
    q.bindValue(":flatrate", tr("Flat Rate"));
    q.bindValue(":peruom", tr("Per UOM"));
    q.bindValue(":custType", tr("Cust. Type"));
    q.bindValue(":custTypePattern", tr("Cust. Type Pattern"));
    q.bindValue(":sale", tr("Sale"));
    q.bindValue(":custtype_id", _custtype->id());
    q.exec();
    _price->populate(q, true);
  }
}
