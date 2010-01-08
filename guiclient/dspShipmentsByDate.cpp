/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// TODO: add checkboxes to distinguish between sales and transfer orders
#include "dspShipmentsByDate.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "inputManager.h"
#include "mqlutil.h"
#include "printShippingForm.h"

dspShipmentsByDate::dspShipmentsByDate(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_ship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _ship->setRootIsDecorated(TRUE);
  _ship->addColumn(tr("Shipment #"),         _orderColumn, Qt::AlignLeft,   true,  "shiphead_number"  );
  _ship->addColumn(tr("Order Type"),	                 80, Qt::AlignLeft,   true,  "shiphead_order_type"  );
  _ship->addColumn(tr("Ship Date"),           _itemColumn, Qt::AlignCenter, true,  "shiphead_shipdate");
  _ship->addColumn(tr("#"),                    _seqColumn, Qt::AlignCenter, true,  "linenumber");
  _ship->addColumn(tr("S/O #/Item"),          _itemColumn, Qt::AlignLeft,   true,  "order_item"  );
  _ship->addColumn(tr("Customer/Description"),         -1, Qt::AlignLeft,   true,  "cust_desc"  );
  _ship->addColumn(tr("Site"),                 _whsColumn, Qt::AlignCenter, true,  "warehous_code");
  _ship->addColumn(tr("Ordered"),              _qtyColumn, Qt::AlignRight,  true,  "qtyord" );
  _ship->addColumn(tr("Shipped"),              _qtyColumn, Qt::AlignRight,  true,  "qtyshipped" );
  _ship->addColumn(tr("Tracking #"),           _qtyColumn, Qt::AlignRight,  true,  "shiphead_tracknum" );
  _ship->addColumn(tr("Freight at Shipping"),  _qtyColumn, Qt::AlignRight,  true,  "shiphead_freight" );
  _ship->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignRight,  true,  "freight_curr_abbr" );
}

dspShipmentsByDate::~dspShipmentsByDate()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspShipmentsByDate::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspShipmentsByDate::set(const ParameterList &pParams)
{ 
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

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

void dspShipmentsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Print Shipping Form..."), this, SLOT(sPrintShippingForm()), 0);
  if (!_privileges->check("PrintBillsOfLading"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspShipmentsByDate::setParams(ParameterList & params)
{
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  _dates->appendValue(params);
}

void dspShipmentsByDate::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("ShipmentsByDate", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspShipmentsByDate::sPrintShippingForm()
{
  ParameterList params;
  params.append("cosmisc_id", _ship->id());
  params.append("shiphead_id", _ship->id());

  printShippingForm newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspShipmentsByDate::sFillList()
{
  _ship->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("<p>You must enter a Start Date before running this query.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("<p>You must enter a End Date before running this query.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);
  MetaSQLQuery fillm = mqlLoad("shipments", "detail");
  q = fillm.toQuery(params);
  if (q.first())
  {
    _ship->populate(q, true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
