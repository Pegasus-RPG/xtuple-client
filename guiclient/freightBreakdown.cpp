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

#include "freightBreakdown.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>

freightBreakdown::freightBreakdown(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,         SIGNAL(clicked()),              this, SLOT(sSave()));

  _freight->addColumn(tr("Schedule"),           _itemColumn,     Qt::AlignLeft,   true, "freightdata_schedule");
  _freight->addColumn(tr("From"),               _itemColumn,     Qt::AlignLeft,   true, "freightdata_from");
  _freight->addColumn(tr("To"),                 _itemColumn,     Qt::AlignLeft,   true, "freightdata_to");
  _freight->addColumn(tr("Ship Via"),           _itemColumn,     Qt::AlignLeft,   true, "freightdata_shipvia");
  _freight->addColumn(tr("Freight Class"),      _itemColumn,     Qt::AlignLeft,   true, "freightdata_freightclass");
  _freight->addColumn(tr("Total Weight"),       _qtyColumn,      Qt::AlignRight,  true, "freightdata_weight");
  _freight->addColumn(tr("UOM"),                _uomColumn,      Qt::AlignCenter, true, "freightdata_uom");
  _freight->addColumn(tr("Price"),              _moneyColumn,    Qt::AlignRight,  true, "freightdata_price");
  _freight->addColumn(tr("Type"),               _itemColumn,     Qt::AlignLeft,   true, "freightdata_type");
  _freight->addColumn(tr("Total"),              _moneyColumn,    Qt::AlignRight,  true, "freightdata_total");
  _freight->addColumn(tr("Currency"),           _currencyColumn, Qt::AlignCenter, true, "freightdata_currency");
}

freightBreakdown::~freightBreakdown()
{
  // no need to delete child widgets, Qt does it all for us
}

void freightBreakdown::languageChange()
{
  retranslateUi(this);
}

SetResponse freightBreakdown::set(const ParameterList& pParams)
{
  QVariant param;
  bool	   valid;

  ParameterList params;
  param = pParams.value("order_id", &valid);
  if (valid)
  {
    _orderid = param.toInt();
    params.append("order_id", _orderid);
  }

  param = pParams.value("document_number", &valid);
  if (valid)
    _document->setText(param.toString());

  param = pParams.value("order_type", &valid);
  if (valid)
  {
    _ordertype = param.toString();
    params.append("order_type", _ordertype);
    if (_ordertype == "SO")
      _header->setText(tr("Freight Breakdown for Sales Order:"));
    else
      _header->setText(tr("Freight Breakdown for Quote:"));
  }

  param = pParams.value("calcfreight", &valid);
  if (valid)
  {
    _calcfreight = param.toBool();
    if (_calcfreight == true)
      _calculated->setChecked(true);
    else
      _manual->setChecked(true);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if(param.toString() == "view")
      _mode = cView;
    else
      _mode = cEdit;
  }

  QString sql =	"SELECT *,"
                "       'weight' AS freightdata_weight_xtnumericrole,"
                "       'salesprice' AS freightdata_price_xtnumericrole,"
                "       'curr' AS freightdata_total_xtnumericrole,"
                "       0 AS freightdata_total_xttotalrole "
                "FROM freightDetail(<? value(\"order_type\") ?>, <? value(\"order_id\") ?>);";

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _freight->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return UndefinedError;
  }

  return NoError;
}

void freightBreakdown::sSave()
{
  accept();
}

