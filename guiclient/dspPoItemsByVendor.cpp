/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemsByVendor.h"

#include <QMenu>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>
#include <QMessageBox>

#include <metasql.h>
#include <openreports.h>

#include "purchaseOrder.h"
#include "purchaseOrderItem.h"
#include "reschedulePoitem.h"
#include "changePoitemQty.h"
#include "dspRunningAvailability.h"

#define POITEM_STATUS_COL 11

dspPoItemsByVendor::dspPoItemsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_vendor, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_next, SIGNAL(clicked()), this, SLOT(sSearchNext()));

  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight,  true,  "pohead_number"  );
  _poitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter, true,  "warehousecode"  );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter, true,  "poitemstatus" );
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter, true,  "poitem_duedate" );
  _poitem->addColumn(tr("Item #"),      _itemColumn,  Qt::AlignLeft,   true,  "itemnumber"   );
  _poitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _poitem->addColumn(tr("Vend. Item #"), _itemColumn, Qt::AlignLeft,   true,  "poitem_vend_item_number");
  _poitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "itemuom" );
  _poitem->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_ordered"  );
  _poitem->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_received"  );
  _poitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_returned"  );
  _poitem->addColumn(tr("Item Status"), 10,           Qt::AlignCenter, false, "poitem_status" );

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
  params.append("vend_id", _vendor->id());

  _warehouse->appendValue(params);

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
  int menuItem;

  if (pSelected->text(POITEM_STATUS_COL) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privileges->check("MaintainPurchaseOrders")) && (!_privileges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  if (pSelected->text(POITEM_STATUS_COL) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
  if ((!_privileges->check("MaintainPurchaseOrders")) && (!_privileges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (pSelected->text(POITEM_STATUS_COL) != "C")
  {
    menuItem = pMenu->insertItem(tr("Reschedule..."), this, SLOT(sReschedule()), 0);
    if (!_privileges->check("ReschedulePurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change Qty..."), this, SLOT(sChangeQty()), 0);
    if (!_privileges->check("ChangePurchaseOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }

  if (pSelected->text(POITEM_STATUS_COL) == "O")
  {
    menuItem = pMenu->insertItem(tr("Close Item..."), this, SLOT(sCloseItem()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (pSelected->text(POITEM_STATUS_COL) == "C")
  {
    menuItem = pMenu->insertItem(tr("Open Item..."), this, SLOT(sOpenItem()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspPoItemsByVendor::sRunningAvailability()
{
  q.prepare("SELECT poitem_itemsite_id "
            "FROM poitem "
            "WHERE (poitem_id=:poitemid); ");
  q.bindValue(":poitemid", _poitem->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("poitem_itemsite_id").toInt());
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
  q.prepare( "UPDATE poitem "
             "SET poitem_status='C' "
             "WHERE (poitem_id=:poitem_id);" );
  q.bindValue(":poitem_id", _poitem->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByVendor::sOpenItem()
{
  q.prepare( "UPDATE poitem "
             "SET poitem_status='O' "
             "WHERE (poitem_id=:poitem_id);" );
  q.bindValue(":poitem_id", _poitem->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByVendor::sFillList()
{
  _poitem->clear();

  QString sql( "SELECT pohead_id, poitem_id, pohead_number,"
               "       poitem_status,"
               "       CASE WHEN(poitem_status='C') THEN <? value(\"closed\") ?>"
               "            WHEN(poitem_status='U') THEN <? value(\"unposted\") ?>"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered>(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"partial\") ?>"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered<=(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"received\") ?>"
               "            WHEN(poitem_status='O') THEN <? value(\"open\") ?>"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       CASE WHEN (itemsite_id IS NULL) THEN ( SELECT warehous_code"
               "                                              FROM warehous"
               "                                              WHERE (pohead_warehous_id=warehous_id) )"
               "            ELSE ( SELECT warehous_code"
               "                   FROM warehous"
               "                   WHERE (itemsite_warehous_id=warehous_id) )"
               "       END AS warehousecode,"
               "       COALESCE(item_number, <? value(\"nonInv\") ?>) AS itemnumber,"
               "       COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) AS itemdescrip,"
               "       poitem_vend_item_number,"
               "       COALESCE(uom_name, poitem_vend_uom) AS itemuom,"
               "       poitem_duedate, poitem_qty_ordered, poitem_qty_received, poitem_qty_returned,"
               "       CASE WHEN (poitem_duedate < CURRENT_DATE) THEN 'error' END AS poitem_duedate_qtforegroundrole,"
               "       'qty' AS poitem_qty_ordered_xtnumericrole,"
               "       'qty' AS poitem_qty_received_xtnumericrole,"
               "       'qty' AS poitem_qty_returned_xtnumericrole "
               "FROM pohead,"
               "     poitem LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id))"
               "     ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ((poitem_pohead_id=pohead_id)"
               "<? if exists(\"warehous_id\") ?>"
               " AND (((itemsite_id IS NULL) AND"
               "       (pohead_warehous_id=<? value(\"warehous_id\") ?>) ) OR"
               "      ((itemsite_id IS NOT NULL) AND"
               "       (itemsite_warehous_id=<? value(\"warehous_id\") ?>) ) )"
               "<? endif ?>"
               "<? if exists(\"agentUsername\") ?>"
               " AND (pohead_agent_username=<? value(\"agentUsername\") ?>)"
               "<? endif ?>"
               "<? if exists(\"poNumber\") ?>"
               " AND (pohead_number=<? value(\"poNumber\") ?>)"
               "<? endif ?>"
               "<? if exists(\"openItems\") ?>"
               " AND (poitem_status='O')"
               "<? endif ?>"
               "<? if exists(\"closedItems\") ?>"
               " AND (poitem_status='C')"
               "<? endif ?>"
               " AND (pohead_vend_id=<? value(\"vend_id\") ?>) ) "
               "ORDER BY poitem_duedate, pohead_number, poitem_linenumber;" );
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    _poitem->populate(q, true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemsByVendor::sPopulatePo()
{
  _poNumber->clear();

  if ( _vendor->isValid())
  {
    q.prepare( "SELECT pohead_id, pohead_number "
               "FROM pohead "
               "WHERE ( (pohead_vend_id=:vend_id) ) "
               "ORDER BY pohead_number;" );
    q.bindValue(":vend_id", _vendor->id());
    q.exec();
    _poNumber->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
    if ( _poitem->topLevelItem(i)->text(4).startsWith(pTarget, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(5).contains(pTarget, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(6).startsWith(pTarget, Qt::CaseInsensitive) )
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
    if ( _poitem->topLevelItem(i)->text(4).startsWith(target, Qt::CaseInsensitive) ||
         _poitem->topLevelItem(i)->text(5).contains(target, Qt::CaseInsensitive)  ||
         _poitem->topLevelItem(i)->text(6).startsWith(target, Qt::CaseInsensitive) )
      break;
  }

  if (i < _poitem->topLevelItemCount())
  {
    _poitem->setCurrentItem(_poitem->topLevelItem(i));
    _poitem->scrollToItem(_poitem->topLevelItem(i));
  }
}


