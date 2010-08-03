/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemsByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>

#include "changePoitemQty.h"
#include "dspRunningAvailability.h"
#include "purchaseOrder.h"
#include "purchaseOrderItem.h"
#include "reschedulePoitem.h"

dspPoItemsByItem::dspPoItemsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight,  true,  "pohead_number"  );
  _poitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter, true,  "poitem_status" );
  _poitem->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft,   true,  "vend_name"   );
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter, true,  "poitem_duedate" );
  _poitem->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_ordered"  );
  _poitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "itemuom" );
  _poitem->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_received"  );
  _poitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_returned"  );
}

dspPoItemsByItem::~dspPoItemsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoItemsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspPoItemsByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT itemsrc_item_id "
               "FROM itemsrc "
               "WHERE (itemsrc_id=:itemsrc_id);" );
    q.bindValue(":itemsrc_id", param.toInt());
    q.exec();
    if (q.first())
      _item->setId(q.value("itemsrc_item_id").toInt());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspPoItemsByItem::setParams(ParameterList &params)
{
  params.append("byItem");

  params.append("item_id", _item->id() );

  _warehouse->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_closedItems->isChecked())
    params.append("closedItems");
  else if (_openItems->isChecked())
    params.append("openItems");

  params.append("closed",	tr("Closed"));
  params.append("unposted",	tr("Unposted"));
  params.append("partial",	tr("Partial"));
  params.append("received",	tr("Received"));
  params.append("open",		tr("Open"));
}

void dspPoItemsByItem::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                      tr( "Please enter a valid Item Number." ) );
    _item->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("POLineItemsByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoItemsByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
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

void dspPoItemsByItem::sRunningAvailability()
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

void dspPoItemsByItem::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByItem::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByItem::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("poitem_id", _poitem->altId());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByItem::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("poitem_id", _poitem->altId());

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByItem::sReschedule()
{
  ParameterList params;
  params.append("poitem_id", _poitem->altId());

  reschedulePoitem newdlg(this, "", TRUE);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspPoItemsByItem::sChangeQty()
{
  ParameterList params;
  params.append("poitem_id", _poitem->altId());

  changePoitemQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPoItemsByItem::sCloseItem()
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

void dspPoItemsByItem::sOpenItem()
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

void dspPoItemsByItem::sFillList()
{
  _poitem->clear();

  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("poItems", "detail");
  q = mql.toQuery(params);

  q.exec();
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
