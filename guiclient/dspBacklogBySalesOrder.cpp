/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogBySalesOrder.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <parameter.h>
#include <openreports.h>
#include "inputManager.h"
#include "salesOrderList.h"
#include "dspRunningAvailability.h"

dspBacklogBySalesOrder::dspBacklogBySalesOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_salesOrder, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_salesOrder, SIGNAL(requestList()), this, SLOT(sSalesOrderList()));

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _salesOrder, SLOT(setId(int)));

  _soitem->addColumn(tr("#"),           _seqColumn,  Qt::AlignCenter,true, "coitem_linenumber");
  _soitem->addColumn(tr("Item"),        _itemColumn, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "itemdescription");
  _soitem->addColumn(tr("Site"),        _whsColumn,  Qt::AlignLeft,  true, "warehous_code");
  _soitem->addColumn(tr("Ordered"),     _qtyColumn,  Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Shipped"),     _qtyColumn,  Qt::AlignRight, true, "coitem_qtyshipped");
  _soitem->addColumn(tr("Balance"),     _qtyColumn,  Qt::AlignRight, true, "qtybalance");
  _soitem->addColumn(tr("At Shipping"), _qtyColumn,  Qt::AlignRight, true, "qtyatshipping");
  _soitem->addColumn(tr("Available"),   _qtyColumn,  Qt::AlignRight, true, "qtyavailable");
}

dspBacklogBySalesOrder::~dspBacklogBySalesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBacklogBySalesOrder::languageChange()
{
  retranslateUi(this);
}

void dspBacklogBySalesOrder::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);
}

void dspBacklogBySalesOrder::sPrint()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());

  orReport report("BacklogBySalesOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogBySalesOrder::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _salesOrder->id());
  params.append("soType", cSoOpen);

  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _salesOrder->setId(newdlg.exec());
}

void dspBacklogBySalesOrder::sRunningAvailability()
{
  q.prepare( "SELECT coitem_itemsite_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", _soitem->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("coitem_itemsite_id").toInt());
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspBacklogBySalesOrder::sFillList()
{
  if (_salesOrder->isValid())
  {
    q.prepare( "SELECT cohead_number,"
               "       cohead_orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", _salesOrder->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("cohead_orderdate").toDate());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }

    MetaSQLQuery mql = mqlLoad("salesOrderItems", "detail");
    ParameterList params;
    params.append("cohead_id", _salesOrder->id());
    q = mql.toQuery(params);
    _soitem->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
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
