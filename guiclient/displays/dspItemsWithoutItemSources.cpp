/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemsWithoutItemSources.h"

#include <QMenu>
#include <QVariant>

#include "guiclient.h"
#include "item.h"
#include "itemSource.h"

dspItemsWithoutItemSources::dspItemsWithoutItemSources(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemsWithoutItemSources", fl)
{
  setWindowTitle(tr("Items without Item Sources"));
  setListLabel(tr("Items without Item Sources"));
  setMetaSQLOptions("itemsWithoutItemSources", "detail");

  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft, true, "item_number");
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "descrip");
  list()->addColumn(tr("Type"),        _itemColumn,  Qt::AlignCenter,true, "type");

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspItemsWithoutItemSources::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
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
  params.append("item_id", list()->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspItemsWithoutItemSources::sEditItem()
{
  item::editItem(list()->id());
}

bool dspItemsWithoutItemSources::setParams(ParameterList &params)
{
  params.append("purchased", tr("Purchased"));
  params.append("outside", tr("Outside"));

  return true;
}

