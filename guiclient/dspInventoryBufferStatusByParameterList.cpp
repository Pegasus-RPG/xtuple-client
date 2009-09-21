/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryBufferStatusByParameterList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "dspInventoryHistoryByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "workOrder.h"
#include "postMiscProduction.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "createCountTagsByItem.h"
#include "enterMiscCount.h"

dspInventoryBufferStatusByParameterList::dspInventoryBufferStatusByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _availability->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "item_number");
  _availability->addColumn(tr("Description"),  -1,          Qt::AlignLeft,  true, "itemdescrip");
  _availability->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  _availability->addColumn(tr("LT"),           _whsColumn,  Qt::AlignCenter,true, "itemsite_leadtime");
  _availability->addColumn(tr("Type"),         _qtyColumn,  Qt::AlignCenter,true, "bufrststype");
  _availability->addColumn(tr("Status"),       _qtyColumn,  Qt::AlignRight, true, "bufrsts_status");
  _availability->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight, true, "qoh");
  _availability->addColumn(tr("Allocated"),    _qtyColumn,  Qt::AlignRight, true, "allocated");
  _availability->addColumn(tr("Unallocated"),  _qtyColumn,  Qt::AlignRight, true, "unallocated");
  _availability->addColumn(tr("On Order"),     _qtyColumn,  Qt::AlignRight, true, "ordered");
  _availability->addColumn(tr("Reorder Lvl."), _qtyColumn,  Qt::AlignRight, true, "reorderlevel");
  _availability->addColumn(tr("OUT Level."),   _qtyColumn,  Qt::AlignRight, true, "outlevel");
  _availability->addColumn(tr("Available"),    _qtyColumn,  Qt::AlignRight, true, "available");

}

dspInventoryBufferStatusByParameterList::~dspInventoryBufferStatusByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryBufferStatusByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryBufferStatusByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ItemGroup);

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Inventory Buffer Status by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Inventory Buffer Status by Planner Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Inventory Buffer Status by Item Group"));
      break;

    default:
      break;
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInventoryBufferStatusByParameterList::sPrint()
{
  ParameterList params;
  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if (_parameter->isAll())
  {
    if (_parameter->type() == ParameterGroup::ItemGroup)
      params.append("itemgrp");
    else if(_parameter->type() == ParameterGroup::PlannerCode)
      params.append("plancode");
    else if (_parameter->type() == ParameterGroup::ClassCode)
      params.append("classcode");
  }

  if (_GreaterThanZero->isChecked())
    params.append("GreaterThanZero");
  else if (_EmergencyZone->isChecked())
    params.append("EmergencyZone");
  else if (_All->isChecked())
    params.append("All");

  orReport report("InventoryBufferStatusByParameterList", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);

}

void dspInventoryBufferStatusByParameterList::sPopulateMenu(QMenu *menu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  if (selected->text(5).remove(',').toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  if (selected->text(7).remove(',').toDouble() == 0.0)
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Running Availability..."), this, SLOT(sRunningAvailability()), 0);

  menu->insertSeparator();

  if (((XTreeWidgetItem *)selected)->altId() == 1)
  {
    menuItem = menu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
    if (!_privileges->check("MaintainPurchaseRequests"))
      menu->setItemEnabled(menuItem, FALSE);

    menuItem = menu->insertItem(tr("Create P/O..."), this, SLOT(sCreatePO()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      menu->setItemEnabled(menuItem, FALSE);

    menu->insertSeparator();
  }
  else if (((XTreeWidgetItem *)selected)->altId() == 2)
  {
    menuItem = menu->insertItem(tr("Create W/O..."), this, SLOT(sCreateWO()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      menu->setItemEnabled(menuItem, FALSE);

    menuItem = menu->insertItem(tr("Post Misc. Production..."), this, SLOT(sPostMiscProduction()), 0);
    if (!_privileges->check("PostMiscProduction"))
      menu->setItemEnabled(menuItem, FALSE);

    menu->insertSeparator();
  }
    
  menu->insertItem(tr("View Substitute Availability..."), this, SLOT(sViewSubstituteAvailability()), 0);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()), 0);
  if (!_privileges->check("EnterMiscCounts"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspInventoryBufferStatusByParameterList::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byLeadTime");
  params.append("run");

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byLeadTime");
  params.append("run");


  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sPostMiscProduction()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());

  postMiscProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sViewSubstituteAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("run");
  params.append("byLeadTime", TRUE);

  dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryBufferStatusByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryBufferStatusByParameterList::sFillList()
{
  _availability->clear();

  QString sql( "SELECT itemsite_id, itemtype, item_number,"
              "        (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
               "       warehous_id, warehous_code, itemsite_leadtime,"
               "       CASE WHEN (bufrsts_type='T') THEN :time"
               "            ELSE :stock"
               "       END AS bufrststype,"
               "       bufrsts_status,"
               "       qoh, allocated,"
               "       noNeg(qoh - allocated) AS unallocated,"
               "       ordered, reorderlevel, outlevel,"
               "       (qoh - allocated + ordered) AS available,"
               "       'qty' AS qoh_xtnumericrole,"
               "       'qty' AS allocated_xtnumericrole,"
               "       'qty' AS unallocated_xtnumericrole,"
               "       'qty' AS ordered_xtnumericrole,"
               "       'qty' AS reorderlevel_xtnumericrole,"
               "       'qty' AS outlevel_xtnumericrole,"
               "       'qty' AS available_xtnumericrole,"
               "       CASE WHEN (bufrsts_status > 65) THEN 'error'"
               "       END AS bufrsts_status_qtforegroundrole "
               "  FROM ( SELECT itemsite_id,"
               "                CASE WHEN (item_type IN ('P', 'O')) THEN 1"
               "                     WHEN (item_type IN ('M')) THEN 2"
               "                     ELSE 0"
               "                END AS itemtype,"
               "                item_number, item_descrip1, item_descrip2,"
               "                warehous_id, warehous_code, itemsite_leadtime,"
               "                bufrsts_status, bufrsts_type,"
               "                itemsite_qtyonhand AS qoh,"
               "                itemsite_reorderlevel AS reorderlevel,"
               "                itemsite_ordertoqty AS outlevel," 
               "                qtyAllocated(itemsite_id, endoftime()) AS allocated,"
               "                qtyOrdered(itemsite_id, endoftime()) AS ordered "
               "           FROM item, itemsite, warehous, bufrsts "
               "          WHERE ( (itemsite_active)"
               "            AND   (itemsite_item_id=item_id)"
               "            AND   (itemsite_warehous_id=warehous_id)"
               "            AND   (bufrsts_target_type='I')"
               "            AND   (bufrsts_target_id=itemsite_id)"
               "            AND   (bufrsts_date=current_date)");

  if  (_GreaterThanZero->isChecked())
    sql += " AND (bufrsts_status > 0) ";

  else if (_EmergencyZone->isChecked())
    sql += " AND (bufrsts_status > 65)";
  
  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (_parameter->isSelected())
  {
    if (_parameter->type() == ParameterGroup::ClassCode)
      sql += " AND (item_classcode_id=:classcode_id)";
    else if (_parameter->type() == ParameterGroup::ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem WHERE (itemgrpitem_itemgrp_id=:itemgrp_id)))";
    else if (_parameter->type() == ParameterGroup::PlannerCode)
      sql += " AND (itemsite_plancode_id=:plancode_id)";
  }
  else if (_parameter->isPattern())
  {
    if (_parameter->type() == ParameterGroup::ClassCode)
      sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    else if (_parameter->type() == ParameterGroup::ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem, itemgrp WHERE ( (itemgrpitem_itemgrp_id=itemgrp_id) AND (itemgrp_name ~ :itemgrp_pattern) ) ))";
    else if (_parameter->type() == ParameterGroup::PlannerCode)
      sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
  }
  else if (_parameter->type() == ParameterGroup::ItemGroup)
    sql += " AND (item_id IN (SELECT DISTINCT itemgrpitem_item_id FROM itemgrpitem))";

  sql += ") ) as data ";

  sql += "ORDER BY bufrsts_status DESC, item_number, warehous_code DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _parameter->bindValue(q);
  q.bindValue(":stock", tr("Stock"));
  q.bindValue(":time", tr("Time"));
  q.exec();
  _availability->populate(q, true);
}
