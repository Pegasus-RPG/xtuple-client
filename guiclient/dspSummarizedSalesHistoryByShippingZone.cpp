/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesHistoryByShippingZone.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspSummarizedSalesHistoryByShippingZone as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesHistoryByShippingZone::dspSummarizedSalesHistoryByShippingZone(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedShippingZone, SIGNAL(toggled(bool)), _shipZone, SLOT(setEnabled(bool)));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _shipZone->populate( "SELECT shipzone_id, (shipzone_name || '-' || shipzone_descrip) "
                       "FROM shipzone "
                       "ORDER BY shipzone_name;" );

  _sohist->addColumn(tr("Zone"),        _itemColumn,     Qt::AlignLeft,   true,  "shipzone_name"   );
  _sohist->addColumn(tr("Customer"),    200,             Qt::AlignLeft,   true,  "customer"   );
  _sohist->addColumn(tr("Item Number"), _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  _sohist->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "itemdescription"   );
  _sohist->addColumn(tr("Shipped"),     _qtyColumn,      Qt::AlignRight,  true,  "totalunits"  );
  _sohist->addColumn(tr("Total Sales"), _bigMoneyColumn, Qt::AlignRight,  true,  "totalsales"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesHistoryByShippingZone::~dspSummarizedSalesHistoryByShippingZone()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesHistoryByShippingZone::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesHistoryByShippingZone::sFillList()
{
  QString sql( "SELECT shipzone_id, cust_id, "
               "       shipzone_name, (cust_number || '-' || cust_name) AS customer,"
               "       item_number, itemdescription,"
               "       SUM(cohist_qtyshipped) AS totalunits,"
               "       SUM(baseextprice) AS totalsales,"
               "       'qty' AS totalunits_xtnumericrole,"
               "       'curr' AS totalsales_xtnumericrole,"
               "       0 AS totalunits_xttotalrole,"
               "       0 AS totalsales_xttotalrole "
               "FROM saleshistory "
               "WHERE ((cohist_shipdate BETWEEN :startDate and :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  if (_selectedShippingZone->isChecked())
    sql += " AND (shipzone_id=:shipzone_id)";

  sql += ") "
         "GROUP BY shipzone_id, cust_id, shipzone_name, cust_number, cust_name, item_number, itemdescription;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":shipzone_id", _shipZone->id());
  q.exec();
  _sohist->populate(q, TRUE);
}

void dspSummarizedSalesHistoryByShippingZone::sPrint()
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if(_selectedShippingZone->isChecked())
    params.append("shipzone_id", _shipZone->id());

  orReport report("SummarizedSalesHistoryByShippingZone", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

