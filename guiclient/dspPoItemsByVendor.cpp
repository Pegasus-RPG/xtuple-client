/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemsByVendor.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "purchaseOrder.h"
#include "purchaseOrderItem.h"
#include "reschedulePoitem.h"
#include "changePoitemQty.h"
#include "dspRunningAvailability.h"
#include "mqlutil.h"

dspPoItemsByVendor::dspPoItemsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_vendor, SIGNAL(updated()), this, SLOT(sPopulatePo()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_next, SIGNAL(clicked()), this, SLOT(sSearchNext()));

  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight,  true,  "pohead_number"  );
  _poitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code"  );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter, true,  "poitem_status" );
  _poitem->addColumn(tr("Vendor"),      _itemColumn,  Qt::AlignLeft,   true,  "vend_name"   );
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter, true,  "poitem_duedate" );
  _poitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "itemnumber"   );
  _poitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _poitem->addColumn(tr("Vend. Item #"), _itemColumn, Qt::AlignLeft,   true,  "poitem_vend_item_number");
  _poitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "itemuom" );
  _poitem->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_ordered"  );
  _poitem->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_received"  );
  _poitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_returned"  );

  _vendor->setFocus();
}

dspPoItemsByVendor::~dspPoItemsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoItemsByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoItemsByVendor::setParams(ParameterList & params)
{
  params.append("byVendor");

  _warehouse->appendValue(params);
  _vendor->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_selectedPO->isChecked())
    params.append("poNumber", _poNumber->currentText());

  if (_closedItems->isChecked())
    params.append("closedItems");
  else if (_openItems->isChecked())
    params.append("openItems");

  params.append("nonInv",	tr("Non Inventory"));
  params.append("closed",	tr("Closed"));
  params.append("unposted",	tr("Unposted"));
  params.append("partial",	tr("Partial"));
  params.append("received",	tr("Received"));
  params.append("open",		tr("Open"));
}

void dspPoItemsByVendor::sPrint()
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Vendor Number"),
                          tr( "Please enter a valid Vendor Number." ) );
    _vendor->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("POLineItemsByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);

}

void dspPoItemsByVendor::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  QAction *menuItem;
  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(pSelected);

  if (item && item->rawValue("poitem_status") == "U")
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                       _privileges->check("ViewPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));

  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  if (item && item->rawValue("poitem_status") == "U")
  {
    menuItem = pMenu->addAction(tr("Edit Item..."), this, SLOT(sEditItem()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

  menuItem = pMenu->addAction(tr("View Item..."), this, SLOT(sViewItem()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                       _privileges->check("ViewPurchaseOrders"));

  if (item && item->rawValue("poitem_status") != "C")
  {
    menuItem = pMenu->addAction(tr("Reschedule..."), this, SLOT(sReschedule()));
    menuItem->setEnabled(_privileges->check("ReschedulePurchaseOrders"));

    menuItem = pMenu->addAction(tr("Change Qty..."), this, SLOT(sChangeQty()));
    menuItem->setEnabled(_privileges->check("ChangePurchaseOrderQty"));

    pMenu->addSeparator();
  }

  if (item && item->rawValue("poitem_status") == "O")
  {
    menuItem = pMenu->addAction(tr("Close Item..."), this, SLOT(sCloseItem()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }
  else if (item && item->rawValue("poitem_status") == "C")
  {
    menuItem = pMenu->addAction(tr("Open Item..."), this, SLOT(sOpenItem()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }
}

void dspPoItemsByVendor::sRunningAvailability()
{
  XSqlQuery raq;
  raq.prepare("SELECT poitem_itemsite_id "
              "FROM poitem "
              "WHERE (poitem_id=:poitemid); ");
  raq.bindValue(":poitemid", _poitem->altId());
  raq.exec();
  if (raq.first())
  {
    ParameterList params;
    params.append("itemsite_id", raq.value("poitem_itemsite_id").toInt());
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (raq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, raq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemsByVendor::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByVendor::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByVendor::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("poitem_id", _poitem->altId());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByVendor::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("poitem_id", _poitem->altId());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByVendor::sReschedule()
{
  ParameterList params;
  params.append("poitem_id", _poitem->altId());

  reschedulePoitem newdlg(this, "", TRUE);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspPoItemsByVendor::sChangeQty()
{
  ParameterList params;
  params.append("poitem_id", _poitem->altId());

  changePoitemQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPoItemsByVendor::sCloseItem()
{
  XSqlQuery closeq;
  closeq.prepare("UPDATE poitem "
                 "SET poitem_status='C' "
                 "WHERE (poitem_id=:poitem_id);" );
  closeq.bindValue(":poitem_id", _poitem->altId());
  closeq.exec();
  if (closeq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, closeq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByVendor::sOpenItem()
{
  XSqlQuery openq;
  openq.prepare( "UPDATE poitem "
                 "SET poitem_status='O' "
                 "WHERE (poitem_id=:poitem_id);" );
  openq.bindValue(":poitem_id", _poitem->altId());
  openq.exec();
  if (openq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, openq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByVendor::sFillList()
{
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("poItems", "detail");
  XSqlQuery itemq = mql.toQuery(params);
  _poitem->populate(itemq, true);
  if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemsByVendor::sPopulatePo()
{
  _poNumber->clear();

  if (_vendor->isSelectedVend())
  {
    XSqlQuery poq;
    poq.prepare( "SELECT pohead_id, pohead_number "
               "FROM pohead "
               "WHERE ( (pohead_vend_id=:vend_id) ) "
               "ORDER BY pohead_number;" );
    poq.bindValue(":vend_id", _vendor->vendId());
    poq.exec();
    _poNumber->populate(poq);
    if (poq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, poq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void dspPoItemsByVendor::sSearch( const QString &pTarget )
{
  _poitem->clearSelection();
  int i;
  for (i = 0; i < _poitem->topLevelItemCount(); i++)
  {
    if ( _poitem->topLevelItem(i)->text(_poitem->column("itemnumber")).startsWith(pTarget, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(_poitem->column("itemdescrip")).contains(pTarget, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(_poitem->column("poitem_vend_item_number")).startsWith(pTarget, Qt::CaseInsensitive) )
      break;
  }

  if (i < _poitem->topLevelItemCount())
  {
    _poitem->setCurrentItem(_poitem->topLevelItem(i));
    _poitem->scrollToItem(_poitem->topLevelItem(i));
  }
}

void dspPoItemsByVendor::sSearchNext()
{
  QString target = _searchFor->text();
  int i;
  int currentIndex = _poitem->indexOfTopLevelItem(_poitem->currentItem()) + 1;
  if(currentIndex < 0 || currentIndex > _poitem->topLevelItemCount())
    currentIndex = 0;
  for (i = currentIndex; i < _poitem->topLevelItemCount(); i++)
  {
    if ( _poitem->topLevelItem(i)->text(_poitem->column("itemnumber")).startsWith(target, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(_poitem->column("itemdescrip")).contains(target, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(_poitem->column("poitem_vend_item_number")).startsWith(target, Qt::CaseInsensitive) )
      break;
  }

  if (i < _poitem->topLevelItemCount())
  {
    _poitem->setCurrentItem(_poitem->topLevelItem(i));
    _poitem->scrollToItem(_poitem->topLevelItem(i));
  }
}
