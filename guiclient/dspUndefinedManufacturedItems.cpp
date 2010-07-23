/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUndefinedManufacturedItems.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include <parameter.h>
#include <metasql.h>

#include "bom.h"
#include "item.h"
#include "mqlutil.h"

dspUndefinedManufacturedItems::dspUndefinedManufacturedItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _item->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _item->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter, true,  "item_type" );
  _item->addColumn(tr("Active"),      _orderColumn, Qt::AlignCenter, true,  "item_active" );
  _item->addColumn(tr("Exception"),   _itemColumn,  Qt::AlignCenter, true,  "exception" );

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _bom->setChecked(true);
  }

  if (!_privileges->check("ViewBOMs"))
  {
    _bom->setChecked(FALSE);
    _bom->setEnabled(FALSE);
  }

}

dspUndefinedManufacturedItems::~dspUndefinedManufacturedItems()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUndefinedManufacturedItems::languageChange()
{
  retranslateUi(this);
}

void dspUndefinedManufacturedItems::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Item..."), this, SLOT(sEditItem()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  if (((XTreeWidgetItem *)_item->currentItem())->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Create BOM..."), this, SLOT(sCreateBOM()));
    menuItem->setEnabled(_privileges->check("MaintainBOMs"));
  }
}

void dspUndefinedManufacturedItems::sCreateBOM()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUndefinedManufacturedItems::sEditItem()
{
  item::editItem(_item->id());
}

void dspUndefinedManufacturedItems::sFillList()
{
  sFillList(-1, TRUE);
}

void dspUndefinedManufacturedItems::sFillList(int pItemid, bool pLocal)
{
  if (_bom->isChecked())
  {
    ParameterList params;
    MetaSQLQuery mql = mqlLoad("undefinedManufacturedItems", "detail");
    params.append("noBom", tr("No BOM"));
    if (!_showInactive->isChecked())
      params.append("notshowInactive");
    q = mql.toQuery(params);

    if ((pItemid != -1) && (pLocal))
      _item->populate(q, pItemid, TRUE);
    else
      _item->populate(q, TRUE);
  }
  else
    _item->clear();
}
