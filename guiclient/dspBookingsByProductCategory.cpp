/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBookingsByProductCategory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

dspBookingsByProductCategory::dspBookingsByProductCategory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _productCategory->setType(ParameterGroup::ProductCategory);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->addColumn(tr("S/O #"),            _orderColumn,    Qt::AlignLeft,   true,  "coitem_linenumber"   );
  _soitem->addColumn(tr("Ord. Date"),        _dateColumn,     Qt::AlignCenter, true,  "cohead_orderdate" );
  _soitem->addColumn(tr("Cust. #"),          _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  _soitem->addColumn(tr("Customer/Item Number"),      _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  _soitem->addColumn(tr("Ordered"),          _qtyColumn,      Qt::AlignRight,  true,  "coitem_qtyord"  );
  _soitem->addColumn(tr("Unit Price"),       _priceColumn,    Qt::AlignRight,  true,  "coitem_price"  );
  _soitem->addColumn(tr("Ext. Price"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );
  _soitem->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _soitem->addColumn(tr("Base Unit Price"),  _priceColumn,    Qt::AlignRight,  true,  "baseunitprice" );
  _soitem->addColumn(tr("Base Ext. Price"),  _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
}

dspBookingsByProductCategory::~dspBookingsByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBookingsByProductCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBookingsByProductCategory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

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

  return NoError;
}

bool dspBookingsByProductCategory::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid() && isVisible())
  {
    QMessageBox::warning(this, tr("Enter Start Date"),
                         tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid() && isVisible())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _dates->appendValue(params);
  params.append("orderByOrderdate");

  return true;
}

void dspBookingsByProductCategory::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BookingsByProductCategory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBookingsByProductCategory::sFillList()
{
  MetaSQLQuery mql = mqlLoad("salesOrderItems", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _soitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
