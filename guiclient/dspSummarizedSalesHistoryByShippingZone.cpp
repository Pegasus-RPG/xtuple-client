/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
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
#include <metasql.h>

#include "mqlutil.h"

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
  _sohist->clear();
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("summarizedSalesHistory", "detail");  
  q = mql.toQuery(params);
  _sohist->populate(q, TRUE);
}

bool dspSummarizedSalesHistoryByShippingZone::setParams(ParameterList & params)
{
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  if (_productCategory->isSelected())
    params.append("prodcat_id" , _productCategory->id());
  else if (_productCategory->isPattern())
  {
    QString pattern = _productCategory->pattern();
    if (pattern.length() == 0)
      return false;
    params.append("prodcat_pattern" , _productCategory->pattern());
  }

  if (_selectedShippingZone->isChecked())
    params.append("shipzone_id", _shipZone->id());

  if (! _dates->allValid())
  {
    QMessageBox::warning(this, tr("Enter Dates"),
                         tr("Enter valid date(s)."));
    _dates->setFocus();
    return false;
  }

  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());

  params.append("byShippingZone");

  return true;
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

