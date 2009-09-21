/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspOrders.h"

#include <QMenu>
#include <QVariant>

#include <metasql.h>

#include "changePoitemQty.h"
#include "changeWoQty.h"
#include "mqlutil.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "reschedulePoitem.h"
#include "rescheduleWo.h"

dspOrders::dspOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_orders, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setReadOnly(TRUE);
  _warehouse->setEnabled(FALSE);

  _orders->addColumn(tr("Type"),         _docTypeColumn, Qt::AlignCenter, true,  "order_type" );
  _orders->addColumn(tr("Order #"),      -1,             Qt::AlignLeft,   true,  "order_number"   );
  _orders->addColumn(tr("Total"),        _qtyColumn,     Qt::AlignRight,  true,  "totalqty"  );
  _orders->addColumn(tr("Received"),     _qtyColumn,     Qt::AlignRight,  true,  "relievedqty"  );
  _orders->addColumn(tr("Balance"),      _qtyColumn,     Qt::AlignRight,  true,  "balanceqty"  );
  _orders->addColumn(tr("Running Bal."), _qtyColumn,     Qt::AlignRight,  true,  "runningbalanceqty"  );
  _orders->addColumn(tr("Required"),     _dateColumn,    Qt::AlignCenter, true,  "duedate" );

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

dspOrders::~dspOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspOrders::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspOrders::set(const ParameterList &pParams)
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

void dspOrders::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_orders->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Reschedule P/O Item..."), this, SLOT(sReschedulePoitem()), 0);
    if (!_privileges->check("ReschedulePurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change P/O Item Quantity..."), this, SLOT(sChangePoitemQty()), 0);
    if (!_privileges->check("ChangePurchaseOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_orders->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()), 0);
    if (!_privileges->check("ReprioritizeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()), 0);
    if (!_privileges->check("RescheduleWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()), 0);
    if (!_privileges->check("ChangeWorkOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Print Traveler..."), this, SLOT(sPrintTraveler()), 0);
    if (!_privileges->check("PrintWorkOrderPaperWork"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspOrders::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  reprioritizeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  rescheduleWo newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", _orders->id());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspOrders::sReschedulePoitem()
{
  ParameterList params;
  params.append("poitem_id", _orders->id());

  reschedulePoitem newdlg(this, "", TRUE);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspOrders::sChangePoitemQty()
{
  ParameterList params;
  params.append("poitem_id", _orders->id());

  changePoitemQty newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sFillList()
{
  _orders->clear();

  if ( (_item->isValid()) &&
       ( (_leadTime->isChecked()) || (_byDays->isChecked()) ||
         ((_byDate->isChecked()) && (_date->isValid())) ||
         (_byRange->isChecked() && _startDate->isValid() && _endDate->isValid()) ) )
  {
    MetaSQLQuery mql = mqlLoad("orders", "detail");
    ParameterList params;
    params.append("warehous_id", _warehouse->id());
    params.append("item_id",     _item->id());
    params.append("itemType",    _item->itemType());
    if (_leadTime->isChecked())
      params.append("useLeadTime");
    else if (_byDays->isChecked())
      params.append("days",      _days->value());
    else if (_byDate->isChecked())
      params.append("date",      _date->date());
    else if (_byRange->isChecked())
    {
      params.append("startDate", _startDate->date());
      params.append("endDate",   _endDate->date());
    }

    q = mql.toQuery(params);
    _orders->populate(q, true);
  }
}
