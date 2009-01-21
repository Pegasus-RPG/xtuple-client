/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInvalidBillsOfMaterials.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "item.h"
#include "itemSite.h"

dspInvalidBillsOfMaterials::dspInvalidBillsOfMaterials(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_exceptions, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query,  SIGNAL(clicked()),     this, SLOT(sFillList()));
  connect(_update, SIGNAL(toggled(bool)), this, SLOT(sHandleUpdate()));

  _exceptions->addColumn("componentItemid",                 0, Qt::AlignCenter,true, "pitem_id");
  _exceptions->addColumn("componentSiteId",                 0, Qt::AlignCenter,true, "citem_id");
  _exceptions->addColumn("warehousId",                      0, Qt::AlignCenter,true, "warehous_id");
  _exceptions->addColumn(tr("Site"),               _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _exceptions->addColumn(tr("Parent Item #"),     _itemColumn, Qt::AlignLeft,  true, "parentitem");
  _exceptions->addColumn(tr("Component Item #"),  _itemColumn, Qt::AlignLeft,  true, "componentitem");
  _exceptions->addColumn(tr("Component Item Description"), -1, Qt::AlignLeft,  true, "descrip");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _update->setChecked(true);
}

dspInvalidBillsOfMaterials::~dspInvalidBillsOfMaterials()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInvalidBillsOfMaterials::languageChange()
{
    retranslateUi(this);
}

void dspInvalidBillsOfMaterials::sEditItem()
{
  item::editItem(_exceptions->altId());
}

void dspInvalidBillsOfMaterials::sCreateItemSite()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id",     _exceptions->currentItem()->rawValue("citem_id").toInt());
  params.append("warehous_id", _exceptions->currentItem()->rawValue("warehous_id").toInt());
  
  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  
  sFillList();
}

void dspInvalidBillsOfMaterials::sEditItemSite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _exceptions->id());
  
  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInvalidBillsOfMaterials::sHandleUpdate()
{
  if (_update->isChecked())
  {
    connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(itemsitesUpdated()), this, SLOT(sFillList()));
  }
  else
  {
    disconnect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    disconnect(omfgThis, SIGNAL(itemsitesUpdated()), this, SLOT(sFillList()));
  }
}

void dspInvalidBillsOfMaterials::sFillList()
{
  QString sql("SELECT itemsite_id, parent.item_id AS pitem_id,"
              "        component.item_id AS citem_id,"
              "        warehous_id, warehous_code,"
              "       parent.item_number AS parentitem,"
              "       component.item_number AS componentitem,"
              "        (component.item_descrip1 || ' ' || component.item_descrip2) AS descrip "
               "FROM bomitem, itemsite, item AS parent, item AS component, warehous "
               "WHERE ( (bomitem_parent_item_id=parent.item_id)"
               " AND (bomitem_item_id=component.item_id)"
               " AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires - 1))"
               " AND (itemsite_item_id=parent.item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (parent.item_type='M')"
               " AND (itemsite_supply)"
               " AND (itemsite_active)"
               " AND (component.item_id NOT IN ( SELECT itemsite_item_id"
               "                                 FROM itemsite"
               "                                 WHERE ((itemsite_warehous_id=warehous_id)"
               "                                  AND (itemsite_active)) ) )" );

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY warehous_code, parentitem, componentitem;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();

  _exceptions->populate(q, TRUE);                               
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspInvalidBillsOfMaterials::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Parent Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Parent Item Site..."), this, SLOT(sEditItemSite()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Create Component Item Site..."), this, SLOT(sCreateItemSite()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

