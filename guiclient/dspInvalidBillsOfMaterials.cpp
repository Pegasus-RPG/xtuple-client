/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInvalidBillsOfMaterials.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include <metasql.h>
#include "mqlutil.h"

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
  MetaSQLQuery mql = mqlLoad("invalidBillsofMaterials", "detail");
  ParameterList params;

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  q = mql.toQuery(params);

  _exceptions->populate(q, TRUE);                               
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspInvalidBillsOfMaterials::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Parent Item..."), this, SLOT(sEditItem()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  menuItem = pMenu->addAction(tr("Edit Parent Item Site..."), this, SLOT(sEditItemSite()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));

  menuItem = pMenu->addAction(tr("Create Component Item Site..."), this, SLOT(sCreateItemSite()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));
}

