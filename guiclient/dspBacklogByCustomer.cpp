/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBacklogByCustomer.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByCustomer::dspBacklogByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));

  _cust->setFocus();

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _soitem->addColumn(tr("S/O #/Line #"), _itemColumn, Qt::AlignLeft,  true, "coitem_linenumber");
  _soitem->addColumn(tr("Customer/Item Number"),  -1, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Order"),        _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  _soitem->addColumn(tr("Ship/Sched."),  _dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  _soitem->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "uom_name");
  _soitem->addColumn(tr("Ordered"),       _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Shipped"),       _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  _soitem->addColumn(tr("Balance"),       _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    _soitem->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    _soitem->hideColumn("baseextpricebalance");

  _cust->setFocus();
}

dspBacklogByCustomer::~dspBacklogByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBacklogByCustomer::languageChange()
{
  retranslateUi(this);
}

void dspBacklogByCustomer::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    _soitem->showColumn("baseextpricebalance");
  else
    _soitem->hideColumn("baseextpricebalance");

  sFillList();
}

bool dspBacklogByCustomer::setParams(ParameterList &params)
{
  if (!_cust->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Customer Number"),
                         tr("<p>You must enter a valid Customer Number for "
                            "this report.") );
    _cust->setFocus();
    return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  params.append("cust_id", _cust->id());
  params.append("openOnly");
  params.append("orderByScheddate");

  if (_showPrices->isChecked())
    params.append("showPrices");

  return true;
}

void dspBacklogByCustomer::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BacklogByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogByCustomer::sEditOrder()
{
  salesOrder::editSalesOrder(_soitem->id(), false);
}

void dspBacklogByCustomer::sViewOrder()
{
  salesOrder::viewSalesOrder(_soitem->id());
}

void dspBacklogByCustomer::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByCustomer::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _soitem->altId());
      
  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByCustomer::sPrintPackingList()
{
  QList<XTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(selected[i]);
    if (cursor->altId() == -1)
    {
      ParameterList params;
      params.append("sohead_id", cursor->id());

      printPackingList newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
  }
}

void dspBacklogByCustomer::sAddToPackingListBatch()
{
  QList<XTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(selected[i]);
    if (cursor->altId() == -1)
    {
      q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
      q.bindValue(":sohead_id", cursor->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
}

void dspBacklogByCustomer::sPopulateMenu(QMenu *pMenu)
{
  bool hasParents     = FALSE;
  bool hasChildren    = FALSE;
  QList<XTreeWidgetItem*> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(selected[i]);
    if ( (cursor->altId() == -1) && (!hasParents) )
      hasParents = TRUE;

    if ( (cursor->altId() != -1) && (!hasChildren) )
      hasChildren = TRUE;
  }

  int menuItem;

  if (selected.size() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (hasChildren)
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

  if (hasParents)
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

void dspBacklogByCustomer::sFillList()
{
  MetaSQLQuery mql = mqlLoad("salesOrderItems", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _soitem->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
