/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemsWithoutItemSources.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "item.h"
#include "itemSource.h"

dspItemsWithoutItemSources::dspItemsWithoutItemSources(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));

  _item->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft, true, "item_number");
  _item->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "descrip");
  _item->addColumn(tr("Type"),        _itemColumn,  Qt::AlignCenter,true, "type");
}

dspItemsWithoutItemSources::~dspItemsWithoutItemSources()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspItemsWithoutItemSources::languageChange()
{
    retranslateUi(this);
}

void dspItemsWithoutItemSources::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Create Item Source..."), this, SLOT(sCreateItemSource()), 0);
  if (!_privileges->check("MaintainItemSources"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspItemsWithoutItemSources::sCreateItemSource()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspItemsWithoutItemSources::sEditItem()
{
  item::editItem(_item->id());
}

void dspItemsWithoutItemSources::sFillList()
{
  q.prepare( "SELECT item_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
             "       CASE WHEN (item_type = 'P') THEN :purchased"
             "            WHEN (item_type = 'O') THEN :outside"
             "       END AS type "
             "FROM item "
             "WHERE ( (item_type IN ('P', 'O'))"
             " AND (item_active)"
             " AND (item_id NOT IN (SELECT DISTINCT itemsrc_item_id"
             "                      FROM itemsrc WHERE (itemsrc_active))) ) "
             "ORDER BY item_number;" );
  q.bindValue(":purchased", tr("Purchased"));
  q.bindValue(":outside", tr("Outside"));
  q.exec();
  _item->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
