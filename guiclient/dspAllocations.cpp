/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAllocations.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <format.h>

#include "mqlutil.h"
#include "salesOrder.h"
#include "transferOrder.h"
#include "workOrder.h"

dspAllocations::dspAllocations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_allocations, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setReadOnly(TRUE);
  _warehouse->setEnabled(FALSE);

  _allocations->addColumn(tr("Type"),         _docTypeColumn, Qt::AlignCenter,true, "type");
  _allocations->addColumn(tr("Order #"),      _orderColumn,   Qt::AlignLeft,  true, "order_number");
  _allocations->addColumn(tr("Parent Item"),  -1,             Qt::AlignLeft,  true, "item_number");
  _allocations->addColumn(tr("Total Qty."),   _qtyColumn,     Qt::AlignRight, true, "totalqty");
  _allocations->addColumn(tr("Relieved"),     _qtyColumn,     Qt::AlignRight, true, "relievedqty");
  _allocations->addColumn(tr("Balance"),      _qtyColumn,     Qt::AlignRight, true, "balanceqty");
  _allocations->addColumn(tr("Running Bal."), _qtyColumn,     Qt::AlignRight, true, "runningbal");
  _allocations->addColumn(tr("Required"),     _dateColumn,    Qt::AlignCenter,true, "duedate");

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

dspAllocations::~dspAllocations()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspAllocations::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAllocations::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDate", &valid);
  if (valid)
  {
    _byDate->setChecked(TRUE);
    _date->setDate(param.toDate());
  }

  param = pParams.value("byDays", &valid);
  if (valid)
  {
    _byDays->setChecked(TRUE);
    _days->setValue(param.toInt());
  }

  _byRange->setChecked(pParams.inList("byRange"));

  param = pParams.value("startDate", &valid);
  if (valid)
    _startDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endDate->setDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAllocations::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (QString(pSelected->text(0)) == "W/O")
  {
    menuItem = pMenu->insertItem(tr("View Work Order..."), this, SLOT(sViewWorkOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewWorkOrders"));
  }
  else if (QString(pSelected->text(0)) == "S/O")
  {
    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewCustomerOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewSalesOrders"));

    pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEditCustomerOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainSalesOrders"));
  }
  else if (QString(pSelected->text(0)) == "T/O")
  {
    menuItem = pMenu->insertItem(tr("View Transfer Order..."), this, SLOT(sViewTransferOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewTransferOrders"));

    pMenu->insertItem(tr("Edit Transfer Order..."), this, SLOT(sEditTransferOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainTransferOrders"));
  }
}

void dspAllocations::sViewWorkOrder()
{
  q.prepare( "SELECT womatl_wo_id "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _allocations->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("womatl_wo_id").toInt());
  
    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAllocations::sViewCustomerOrder()
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", _allocations->id());
  q.exec();
  if (q.first())
    salesOrder::viewSalesOrder(q.value("coitem_cohead_id").toInt());
}

void dspAllocations::sEditCustomerOrder()
{
  q.prepare( "SELECT coitem_cohead_id "
             "FROM coitem "
             "WHERE (coitem_id=:coitem_id);" );
  q.bindValue(":coitem_id", _allocations->id());
  q.exec();
  if (q.first())
    salesOrder::editSalesOrder(q.value("coitem_cohead_id").toInt(), false);
}

void dspAllocations::sViewTransferOrder()
{
  q.prepare( "SELECT toitem_tohead_id "
             "FROM toitem "
             "WHERE (toitem_id=:toitem_id);" );
  q.bindValue(":toitem_id", _allocations->id());
  q.exec();
  if (q.first())
    transferOrder::viewTransferOrder(q.value("toitem_tohead_id").toInt());
}

void dspAllocations::sEditTransferOrder()
{
  q.prepare( "SELECT toitem_tohead_id "
             "FROM toitem "
             "WHERE (toitem_id=:toitem_id);" );
  q.bindValue(":toitem_id", _allocations->id());
  q.exec();
  if (q.first())
    transferOrder::editTransferOrder(q.value("toitem_tohead_id").toInt(), false);
}

void dspAllocations::sFillList()
{
  _allocations->clear();

  if ( (_item->isValid()) &&
       ( (_leadTime->isChecked()) || (_byDays->isChecked()) ||
         ((_byDate->isChecked()) && (_date->isValid())) ||
         ((_byRange->isChecked()) && (_startDate->isValid()) && (_endDate->isValid())) ) )
  {
    MetaSQLQuery mql = mqlLoad("allocations", "detail");

    ParameterList params;
    params.append("warehous_id", _warehouse->id());
    params.append("item_id",	   _item->id());
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");

    if (_leadTime->isChecked())
      params.append("leadTime");
    else if (_byDays->isChecked())
      params.append("days", _days->value());
    else if (_byDate->isChecked())
      params.append("date", _date->date());
    else if (_byRange->isChecked())
    {
      params.append("startDate", _startDate->date());
      params.append("endDate",   _endDate->date());
    }

    q = mql.toQuery(params);
    _allocations->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
