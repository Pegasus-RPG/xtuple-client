/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrderStatus.h"

#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "inputManager.h"
#include "salesOrderList.h"
#include "mqlutil.h"

dspSalesOrderStatus::dspSalesOrderStatus(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _so->setAllowedTypes(OrderLineEdit::Sales);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(newId(int,QString)), this, SLOT(sFillList(int)));

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));

  _soitem->addColumn(tr("#"),                   _seqColumn,  Qt::AlignCenter, true,  "coitem_linenumber" );
  _soitem->addColumn(tr("Item"),                _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _soitem->addColumn(tr("Description"),         -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _soitem->addColumn(tr("Site"),                _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _soitem->addColumn(tr("Ordered"),             _qtyColumn,  Qt::AlignRight,  true,  "coitem_qtyord"  );
  _soitem->addColumn(tr("Shipped"),             _qtyColumn,  Qt::AlignRight,  true,  "coitem_qtyshipped"  );
  _soitem->addColumn(tr("Returned"),            _qtyColumn,  Qt::AlignRight,  true,  "coitem_qtyreturned"  );
  _soitem->addColumn(tr("Invoiced"),            _qtyColumn,  Qt::AlignRight,  true,  "invoiced"  );
  _soitem->addColumn(tr("Balance"),             _qtyColumn,  Qt::AlignRight,  true,  "balance"  );
  _soitem->addColumn(tr("Close Date"),          _dateColumn, Qt::AlignLeft,   true,  "closedate"  );
  _soitem->addColumn(tr("Close User"),          _itemColumn, Qt::AlignLeft,   true,  "closeuser"  );
  _soitem->addColumn(tr("Child Ord. #/Status"), _itemColumn, Qt::AlignCenter, true,  "childinfo" );

  _so->setFocus();
}

dspSalesOrderStatus::~dspSalesOrderStatus()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesOrderStatus::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSalesOrderStatus::set(const ParameterList &pParams)
{ 
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _so->setId(param.toInt());

  return NoError;
}

void dspSalesOrderStatus::sPrint()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  orReport report("SalesOrderStatus", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSalesOrderStatus::sFillList(int pSoheadid)
{
  if (pSoheadid != -1)
  {
    q.prepare( "SELECT cohead_number,"
               "       cohead_orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("cohead_orderdate").toDate());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MAX(lastupdated) AS lastupdated "
               "  FROM (SELECT cohead_lastupdated AS lastupdated "
               "          FROM cohead "
               "         WHERE (cohead_id=:sohead_id) "
               "         UNION "
               "        SELECT coitem_lastupdated AS lastupdated "
               "          FROM coitem "
               "         WHERE (coitem_cohead_id=:sohead_id) ) AS data; " );
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
      _lastUpdated->setDate(q.value("lastupdated").toDate());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    ParameterList params;
    params.append("sohead_id", pSoheadid);

    MetaSQLQuery mql = mqlLoad("salesOrderStatus", "detail");
    q = mql.toQuery(params);
    _soitem->populate(q);
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    _soitem->clear();
  }
}
