/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrdersByCustomerPO.h"

#include <QMenu>
#include <QMessageBox>

#include <metasql.h>
#include "mqlutil.h"

#include "dspSalesOrderStatus.h"
#include "dspShipmentsBySalesOrder.h"
#include "returnAuthorization.h"
#include "salesOrder.h"

dspSalesOrdersByCustomerPO::dspSalesOrdersByCustomerPO(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_poNumber, SIGNAL(lostFocus()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _so->addColumn(tr("Cust #"),      _itemColumn,  Qt::AlignLeft,   true,  "cust_number"   );
  _so->addColumn(tr("Customer"),    _itemColumn,  Qt::AlignLeft,   true,  "cust_name"   );
  _so->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  _so->addColumn(tr("Ordered"),     _dateColumn,  Qt::AlignRight,  true,  "cohead_orderdate"  );
  _so->addColumn(tr("Scheduled"),   _dateColumn,  Qt::AlignRight,  true,  "min_scheddate"  );
  _so->addColumn(tr("Status"),      _itemColumn,  Qt::AlignCenter, true,  "order_status" );
  _so->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "cohead_shiptoname"   );
  _so->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "cohead_custponumber"   );

  _poNumber->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

dspSalesOrdersByCustomerPO::~dspSalesOrdersByCustomerPO()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesOrdersByCustomerPO::languageChange()
{
  retranslateUi(this);
}

void dspSalesOrdersByCustomerPO::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Edit..."), this, SLOT(sEditOrder()), 0);
  menuThis->insertItem(tr("View..."), this, SLOT(sViewOrder()), 0);
  menuThis->insertSeparator();
  menuThis->insertItem(tr("Shipment Status..."), this, SLOT(sDspShipmentStatus()), 0);
  menuThis->insertItem(tr("Shipments..."), this, SLOT(sDspShipments()), 0);

  if (_privileges->check("MaintainReturns"))
  {
    menuThis->insertSeparator();
    menuThis->insertItem(tr("Create Return Authorization..."), this, SLOT(sCreateRA()));
  }
}

void dspSalesOrdersByCustomerPO::sEditOrder()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  salesOrder::editSalesOrder(_so->id(), false);
}

void dspSalesOrdersByCustomerPO::sViewOrder()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  salesOrder::viewSalesOrder(_so->id());
}

void dspSalesOrdersByCustomerPO::sCreateRA()
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

void dspSalesOrdersByCustomerPO::sDspShipmentStatus()
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

void dspSalesOrdersByCustomerPO::sDspShipments()
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

void dspSalesOrdersByCustomerPO::sFillList()
{
  _so->clear();
  if (_poNumber->text().trimmed().length() == 0)    
    return;

  if (_dates->allValid()) 
  {
    MetaSQLQuery mql = mqlLoad("salesOrders", "detail");
    ParameterList params;
    _dates->appendValue(params);
    params.append("noLines", tr("No Lines"));
    params.append("closed", tr("Closed"));
    params.append("open", tr("Open"));
    params.append("partial", tr("Partial"));
    params.append("poNumber", _poNumber->text());

    q = mql.toQuery(params);
    _so->populate(q);
  }
}

bool dspSalesOrdersByCustomerPO::checkSitePrivs(int orderid)
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
                                       tr("You may not view or edit this Sales Order as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

