/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <parameter.h>
#include <openreports.h>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByItem::dspBacklogByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cSold);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->addColumn(tr("S/O #"),   _orderColumn, Qt::AlignLeft,  true, "cohead_number");
  _soitem->addColumn(tr("#"),         _seqColumn, Qt::AlignCenter,true, "coitem_linenumber");
  _soitem->addColumn(tr("Customer"),          -1, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Ordered"),  _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  _soitem->addColumn(tr("Scheduled"),_dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  _soitem->addColumn(tr("Qty. UOM"),  _qtyColumn, Qt::AlignRight, true, "uom_name");
  _soitem->addColumn(tr("Ordered"),   _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Shipped"),   _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  _soitem->addColumn(tr("Balance"),   _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    _soitem->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    _soitem->hideColumn("baseextpricebalance");

  _item->setFocus();
}

dspBacklogByItem::~dspBacklogByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBacklogByItem::languageChange()
{
  retranslateUi(this);
}

void dspBacklogByItem::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    _soitem->showColumn("baseextpricebalance");
  else
    _soitem->hideColumn("baseextpricebalance");

  sFillList();
}

bool dspBacklogByItem::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Item Number"),
                         tr("<p>You must enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("openOnly");
  params.append("orderByScheddate");

  if (_showPrices->isChecked())
    params.append("showPrices");

  return true;
}

void dspBacklogByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BacklogByItemNumber", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogByItem::sEditOrder()
{
  salesOrder::editSalesOrder(_soitem->id(), false);
}

void dspBacklogByItem::sViewOrder()
{
  salesOrder::viewSalesOrder(_soitem->id());
}

void dspBacklogByItem::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", _soitem->id());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
  if (!_privileges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);


  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
  if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  if (!_privileges->check("PrintPackingLists"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspBacklogByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("salesOrderItems", "detail");
  q = mql.toQuery(params);
  _soitem->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
