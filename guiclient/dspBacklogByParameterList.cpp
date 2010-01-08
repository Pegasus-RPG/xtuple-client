/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogByParameterList.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByParameterList::dspBacklogByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _soitem->addColumn(tr("S/O #/Line #"),_itemColumn, Qt::AlignLeft,  true, "coitem_linenumber");
  _soitem->addColumn(tr("Customer/Item Number"), -1, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Order"),       _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  _soitem->addColumn(tr("Ship/Sched."), _dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  _soitem->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "uom_name");
  _soitem->addColumn(tr("Ordered"),      _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Shipped"),      _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  _soitem->addColumn(tr("Balance"),      _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    _soitem->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    _soitem->hideColumn("baseextpricebalance");
}

dspBacklogByParameterList::~dspBacklogByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBacklogByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBacklogByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerType);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerType);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custtype", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CustomerType);

  param = pParams.value("custgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CustomerGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CustomerGroup);

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ProductCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ProductCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("prodcat", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ProductCategory);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  switch (_parameter->type())
  {
    case ParameterGroup::CustomerType:
      setWindowTitle(tr("Backlog by Customer Type"));
      break;

    case ParameterGroup::CustomerGroup:
      setWindowTitle(tr("Backlog by Customer Group"));
      break;

    case ParameterGroup::ProductCategory:
      setWindowTitle(tr("Backlog by Product Category"));
      break;

    default:
      break;
  }

  return NoError;
}

void dspBacklogByParameterList::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    _soitem->showColumn("baseextpricebalance");
  else
    _soitem->hideColumn("baseextpricebalance");
}

bool dspBacklogByParameterList::setParams(ParameterList &params)
{
  if (! _dates->allValid())
  {
    _dates->setFocus();
    return false;
  }

  params.append("openOnly");
  params.append("orderByScheddate");

  _warehouse->appendValue(params);
  _parameter->appendValue(params);
  _dates->appendValue(params);

  if (_showPrices->isChecked())
    params.append("showPrices");

  if (_parameter->isAll())
  {
    switch (_parameter->type())
    {
      case ParameterGroup::CustomerType:
        params.append("custtype");
        break;

      case ParameterGroup::CustomerGroup:
        params.append("custgrp");
        break;

      case ParameterGroup::ProductCategory:
        params.append("prodcat");
        break;

      default:
        break;
    }
  }

  return true;
}

void dspBacklogByParameterList::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BacklogByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogByParameterList::sEditOrder()
{
  salesOrder::editSalesOrder(_soitem->id(), false);
}

void dspBacklogByParameterList::sViewOrder()
{
  salesOrder::viewSalesOrder(_soitem->id());
}

void dspBacklogByParameterList::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByParameterList::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByParameterList::sPrintPackingList()
{
  QList<XTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("sohead_id", ((XTreeWidgetItem*)(selected[i]))->id());

    printPackingList newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspBacklogByParameterList::sAddToPackingListBatch()
{
  QList<XTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
    q.bindValue(":sohead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void dspBacklogByParameterList::sPopulateMenu(QMenu *pMenu)
{
  QList<XTreeWidgetItem*> selected = _soitem->selectedItems();

  int menuItem;

  if (selected.size() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (_soitem->altId() != -1)
    {
      pMenu->insertSeparator();

      menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
      if (!_privileges->check("MaintainSalesOrders"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
      if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
        pMenu->setItemEnabled(menuItem, FALSE);
    }
  }

  if (_soitem->id() > 0)
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
    if (!_privileges->check("PrintPackingLists"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Add to Packing List Batch..."), this, SLOT(sAddToPackingListBatch()), 0);
    if (!_privileges->check("MaintainPackingListBatch"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspBacklogByParameterList::sFillList()
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
