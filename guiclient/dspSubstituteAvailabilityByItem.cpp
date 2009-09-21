/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSubstituteAvailabilityByItem.h"

#include <QMenu>
#include <QVariant>

#include <openreports.h>

#include "dspAllocations.h"
#include "dspOrders.h"

dspSubstituteAvailabilityByItem::dspSubstituteAvailabilityByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _availability->addColumn(tr("Site"),          _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _availability->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _availability->addColumn(tr("Description"),   -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _availability->addColumn(tr("LT"),            _whsColumn,  Qt::AlignCenter, true,  "leadtime" );
  _availability->addColumn(tr("QOH"),           _qtyColumn,  Qt::AlignRight,  true,  "qtyonhand"  );
  _availability->addColumn(tr("Allocated"),     _qtyColumn,  Qt::AlignRight,  true,  "allocated"  );
  _availability->addColumn(tr("On Order"),      _qtyColumn,  Qt::AlignRight,  true,  "ordered"  );
  _availability->addColumn(tr("Reorder Lvl."),  _qtyColumn,  Qt::AlignRight,  true,  "reorderlevel"  );
  _availability->addColumn(tr("Available"),     _qtyColumn,  Qt::AlignRight,  true,  "available"  );
}

dspSubstituteAvailabilityByItem::~dspSubstituteAvailabilityByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSubstituteAvailabilityByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSubstituteAvailabilityByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);
  }

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
   _byDays->setChecked(TRUE);
   _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
   _byDate->setChecked(TRUE);
   _date->setDate(param.toDate());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspSubstituteAvailabilityByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  
  _warehouse->appendValue(params);

  if (_leadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->value());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  orReport report("SubstituteAvailabilityByRootItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSubstituteAvailabilityByItem::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSubstituteAvailabilityByItem::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", TRUE);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSubstituteAvailabilityByItem::sPopulateMenu(QMenu *menu)
{
  menu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  menu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
}

void dspSubstituteAvailabilityByItem::sFillList()
{
  if ((_leadTime->isChecked()) || (_byDays->isChecked()) ||  ((_byDate->isChecked()) && (_date->isValid())))
  {
    _availability->clear();

    QString sql = "SELECT s_itemsite_id, warehous_code, item_number, itemdescrip,"
                  "       qtyonhand, reorderlevel, leadtime, itemsub_rank,"
                  "       allocated, ordered, available,"
                  "      'qty' AS qtyonhand_xtnumericrole,"
                  "      'qty' AS allocated_xtnumericrole,"
                  "      'qty' AS ordered_xtnumericrole,"
                  "      'qty' AS reorderlevel_xtnumericrole,"
                  "      'qty' AS available_xtnumericrole,"
                  "      CASE WHEN (reorderlevel >= available) THEN 'error' END AS available_qtforegroundrole "
                  "FROM ( "
                  "SELECT sub.itemsite_id AS s_itemsite_id,"
                  " warehous_code, item_number,"
                  " (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,";

    if (_normalize->isChecked())
      sql += " (sub.itemsite_qtyonhand * itemsub_uomratio) AS qtyonhand,"
             " (CASE WHEN(sub.itemsite_useparams) THEN sub.itemsite_reorderlevel ELSE 0.0 END * itemsub_uomratio) AS reorderlevel,"
             " sub.itemsite_leadtime AS leadtime, itemsub_rank,";
    else
      sql += " (sub.itemsite_qtyonhand) AS qtyonhand,"
             " (CASE WHEN(sub.itemsite_useparams) THEN sub.itemsite_reorderlevel ELSE 0.0 END) AS reorderlevel,"
             " sub.itemsite_leadtime AS leadtime, itemsub_rank,";

    if (_leadTime->isChecked())
    {
      if (_normalize->isChecked())
        sql += " (qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio) AS allocated,"
               " (qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio) AS ordered, "
               " ((sub.itemsite_qtyonhand * itemsub_uomratio) + (qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio) - (qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime) * itemsub_uomratio)) AS available ";
      else
        sql += " (qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime)) AS allocated,"
               " (qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime)) AS ordered, "
               " (sub.itemsite_qtyonhand + qtyOrdered(sub.itemsite_id, sub.itemsite_leadtime) - qtyAllocated(sub.itemsite_id, sub.itemsite_leadtime)) AS available ";
    }
    else if (_byDays->isChecked())
    {
      if (_normalize->isChecked())
        sql += " (qtyAllocated(sub.itemsite_id, :days) * itemsub_uomratio) AS allocated,"
               " (qtyOrdered(sub.itemsite_id, :days) * itemsub_uomratio) AS ordered, "
               " ((sub.itemsite_qtyonhand * itemsub_uomratio) + (qtyOrdered(sub.itemsite_id, :days) * itemsub_uomratio) - (qtyAllocated(sub.itemsite_id, :days) * itemsub_uomratio)) AS available ";
      else
        sql += " (qtyAllocated(sub.itemsite_id, :days)) AS allocated,"
               " (qtyOrdered(sub.itemsite_id, :days)) AS ordered, "
               " (sub.itemsite_qtyonhand + qtyOrdered(sub.itemsite_id, :days) - qtyAllocated(sub.itemsite_id, :days)) AS available ";
    }
    else if (_byDate->isChecked())
    {
      if (_normalize->isChecked())
        sql += " (qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio) AS allocated,"
               " (qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio) AS ordered, "
               " ((sub.itemsite_qtyonhand * itemsub_uomratio) + (qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio) - (qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE)) * itemsub_uomratio)) AS available ";
      else
        sql += " (qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE))) AS allocated,"
               " (qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE))) AS ordered, "
               " (sub.itemsite_qtyonhand + qtyOrdered(sub.itemsite_id, (:date - CURRENT_DATE)) - qtyAllocated(sub.itemsite_id, (:date - CURRENT_DATE))) AS available ";
    }

    sql += "FROM item, itemsite AS sub, itemsite AS root, warehous, itemsub "
           "WHERE ( (sub.itemsite_item_id=item_id)"
           " AND (root.itemsite_item_id=itemsub_parent_item_id)"
           " AND (sub.itemsite_item_id=itemsub_sub_item_id)"
           " AND (root.itemsite_warehous_id=sub.itemsite_warehous_id)"
           " AND (sub.itemsite_warehous_id=warehous_id)"
           " AND (root.itemsite_item_id=:item_id)";

    if (_warehouse->isSelected())
      sql += " AND (root.itemsite_warehous_id=:warehous_id)";

    sql += ") ) AS data "
           "ORDER BY itemsub_rank";

    q.prepare(sql);
    q.bindValue(":days", _days->value());
    q.bindValue(":date", _date->date());
    q.bindValue(":item_id", _item->id());
    _warehouse->bindValue(q);
    q.exec();
    _availability->populate(q);
  }
}
