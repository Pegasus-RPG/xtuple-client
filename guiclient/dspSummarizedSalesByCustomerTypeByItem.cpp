/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByCustomerTypeByItem.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspSummarizedSalesByCustomerTypeByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesByCustomerTypeByItem::dspSummarizedSalesByCustomerTypeByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _customerType->setType(ParameterGroup::CustomerType);

  _sohist->addColumn(tr("Item Number"), _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  _sohist->addColumn(tr("Description"), -1 ,             Qt::AlignLeft,   true,  "itemdescription"   );
  _sohist->addColumn(tr("Site"),        _whsColumn,      Qt::AlignCenter, true,  "warehous_code" );
  _sohist->addColumn(tr("Min. Price"),  _priceColumn,    Qt::AlignRight,  true,  "minprice"  );
  _sohist->addColumn(tr("Max. Price"),  _priceColumn,    Qt::AlignRight,  true,  "maxprice"  );
  _sohist->addColumn(tr("Avg. Price"),  _priceColumn,    Qt::AlignRight,  true,  "avgprice"  );
  _sohist->addColumn(tr("Total Units"), _qtyColumn,      Qt::AlignRight,  true,  "totalunits"  );
  _sohist->addColumn(tr("Total Sales"), _bigMoneyColumn, Qt::AlignRight,  true,  "totalsales"  );
  _sohist->setDragString("itemsiteid=");
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesByCustomerTypeByItem::~dspSummarizedSalesByCustomerTypeByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesByCustomerTypeByItem::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesByCustomerTypeByItem::sPrint()
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
  _customerType->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  orReport report("SummarizedSalesHistoryByCustomerTypeByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedSalesByCustomerTypeByItem::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cohist_itemsite_id, item_number, itemdescription, warehous_code,"
               "       minprice, maxprice, avgprice, totalunits, totalsales,"
               "       'salesprice' AS minprice_xtnumericrole,"
               "       'salesprice' AS maxprice_xtnumericrole,"
               "       'salesprice' AS avgprice_xtnumericrole,"
               "       'qty' AS totalunits_xtnumericrole,"
               "       'curr' AS totalsales_xtnumericrole,"
               "       0 AS totalunits_xttotalrole,"
               "       0 AS totalsales_xttotalrole "
               "FROM ( SELECT cohist_itemsite_id, item_number, itemdescription,"
               "              warehous_code, MIN(baseunitprice) AS minprice, MAX(baseunitprice) AS maxprice,"
               "              AVG(baseunitprice) AS avgprice, SUM(cohist_qtyshipped) AS totalunits,"
               "              SUM(baseextprice) AS totalsales "
               "       FROM saleshistory "
               "       WHERE ( (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_customerType->isSelected())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";

  sql += ") "
         "GROUP BY cohist_itemsite_id, item_number, itemdescription, warehous_code ) AS data "
         "ORDER BY item_number, warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerType->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  _sohist->populate(q);
}

bool dspSummarizedSalesByCustomerTypeByItem::checkParameters()
{
  if (!_dates->startDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  if (!_dates->endDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Data.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  return TRUE;
}

