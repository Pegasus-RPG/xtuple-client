/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBookings.h"

#include <QMessageBox>
#include <QVariant>

#include "xcombobox.h"
#include "xtreewidget.h"
#include "parameterwidget.h"

dspBookings::dspBookings(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspBookings", fl)
{
  setWindowTitle(tr("Bookings"));
  setReportName("Bookings");
  setMetaSQLOptions("bookings", "detail");
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate(), true  );
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate(), true);
  parameterWidget()->append(tr("Customer"),   "cust_id",   ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Group"), "custgrp_id", XComboBox::CustomerGroups);
  parameterWidget()->append(tr("Customer Group Pattern"), "custgrp_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Customer Ship-to"),   "shipto_id",   ParameterWidget::Shipto);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesReps);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

  list()->addColumn(tr("Order #"),          _orderColumn,    Qt::AlignLeft,   true,  "cohead_number"  );
  list()->addColumn(tr("Line #"),           _seqColumn,      Qt::AlignLeft,   true,  "f_linenumber"  );
  list()->addColumn(tr("Ord. Date"),        _dateColumn,     Qt::AlignCenter, true,  "cohead_orderdate");
  list()->addColumn(tr("Cust. #"),          _itemColumn,     Qt::AlignLeft,   true,  "cust_number"  );
  list()->addColumn(tr("Customer"),         -1,              Qt::AlignLeft,   false,  "cust_name"  );
  list()->addColumn(tr("Item Number"),      _itemColumn,     Qt::AlignLeft,   true,  "item_number"  );
  list()->addColumn(tr("Description"),      -1,              Qt::AlignLeft,   true,  "itemdescription"  );
  list()->addColumn(tr("Ordered"),          _qtyColumn,      Qt::AlignRight,  true,  "coitem_qtyord" );
  list()->addColumn(tr("Unit Price"),       _priceColumn,    Qt::AlignRight,  false,  "coitem_price" );
  list()->addColumn(tr("Ext. Price"),       _bigMoneyColumn, Qt::AlignRight,  false,  "extprice" );
  list()->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignCenter, false,  "currAbbr" );
  list()->addColumn(tr("Base Unit Price"),  _priceColumn,    Qt::AlignRight,  true,  "baseunitprice" );
  list()->addColumn(tr("Base Ext. Price"),  _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
}

enum SetResponse dspBookings::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  /*
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    _productCategory->setId(param.toInt());

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    _productCategory->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }
*/
  return NoError;
}


