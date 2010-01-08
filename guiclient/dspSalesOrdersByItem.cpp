/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrdersByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByItem::dspSalesOrdersByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _item->setType(ItemLineEdit::cSold);

  _so->addColumn(tr("Order #"),    _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  _so->addColumn(tr("Order Date"), _dateColumn,  Qt::AlignCenter, true,  "cohead_orderdate" );
  _so->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,   true,  "cust_name"   );
  _so->addColumn(tr("Ordered"),    _qtyColumn,   Qt::AlignRight,  true,  "coitem_qtyord"  );
  _so->addColumn(tr("Shipped"),    _qtyColumn,   Qt::AlignRight,  true,  "coitem_qtyshipped"  );
  _so->addColumn(tr("Returned"),   _qtyColumn,   Qt::AlignRight,  true,  "coitem_qtyreturned"  );
  _so->addColumn(tr("Balance"),    _qtyColumn,   Qt::AlignRight,  true,  "qtybalance"  );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

dspSalesOrdersByItem::~dspSalesOrdersByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesOrdersByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSalesOrdersByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  return NoError;
}

void dspSalesOrdersByItem::sPopulateMenu(QMenu *menuThis)
{
  if(_privileges->check("MaintainSalesOrders"))
    menuThis->insertItem(tr("Edit..."), this, SLOT(sEditOrder()), 0);
  menuThis->insertItem(tr("View..."), this, SLOT(sViewOrder()), 0);
  menuThis->insertSeparator();
  menuThis->insertItem(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()), 0);
  menuThis->insertItem(tr("Shipments.."), this, SLOT(sDspShipments()), 0);

  if (_privileges->check("MaintainReturns"))
  {
    menuThis->insertSeparator();
    menuThis->insertItem(tr("Create Return Authorization..."), this, SLOT(sCreateRA()));
  }
}

void dspSalesOrdersByItem::sEditOrder()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  salesOrder::editSalesOrder(_so->id(), false);
}

void dspSalesOrdersByItem::sViewOrder()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  salesOrder::viewSalesOrder(_so->id());
}

void dspSalesOrdersByItem::sCreateRA()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  ParameterList params;
  params.append("mode", "new");
  params.append("sohead_id", _so->id());

  returnAuthorization *newdlg = new returnAuthorization();
  if (newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
			  tr("The new Return Authorization could not be created"));
}

void dspSalesOrdersByItem::sDspShipmentStatus()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSalesOrdersByItem::sDspShipments()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("run");

  dspShipmentsBySalesOrder *newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspSalesOrdersByItem::sFillList()
{
  _so->clear();
  if ((_item->isValid()) && (_dates->allValid()))
  {
    MetaSQLQuery mql = mqlLoad("salesOrderItems", "detail");
    ParameterList params;
    _dates->appendValue(params);
    params.append("closed", tr("Closed"));
    params.append("item_id", _item->id());

    q = mql.toQuery(params);
    _so->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

bool dspSalesOrdersByItem::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("<p>You may not view or edit this Sales Order "
                                 "as it references a Site for which you have "
                                 "not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
