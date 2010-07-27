/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "searchForItem.h"

#include <QVariant>
#include <QAction>
#include <QMenu>
#include <parameter.h>
#include "item.h"
#include "bom.h"

searchForItem::searchForItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_showInactive, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip1, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip2, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search, SIGNAL(lostFocus()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu *, QTreeWidgetItem *)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _item->addColumn(tr("Number"),      _itemColumn, Qt::AlignCenter, true, "item_number" );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft  , true, "description" );
  _item->addColumn(tr("Type"),        _itemColumn, Qt::AlignLeft  , true, "type" );

  if (_privileges->check("MaintainItemMasters"))
  {
    connect(_item, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_item, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _searchNumber->setChecked(true);
    _searchDescrip1->setChecked(true);
    _searchDescrip2->setChecked(true);
  }

  _search->setFocus();
}

searchForItem::~searchForItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void searchForItem::languageChange()
{
  retranslateUi(this);
}

void searchForItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  QAction *menuItem;

  bool hasBOM = false;
  q.prepare("SELECT (count(*) != 0) AS hasBOM"
            "  FROM bomhead"
            " WHERE (bomhead_item_id=:item_id); ");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if(q.first())
    hasBOM = q.value("hasBOM").toBool();

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainItemMasters"))
    menuItem->setEnabled(false);

  if (((XTreeWidgetItem *)pSelected)->altId() == 1)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("View BOM..."), this, SLOT(sViewBOM()));
    if (!hasBOM || !_privileges->check("ViewBOMs"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Edit BOM..."), this, SLOT(sEditBOM()));
    if (!hasBOM || !_privileges->check("MaintainBOMs"))
      menuItem->setEnabled(false);
  }
}

void searchForItem::sView()
{
  item::viewItem(_item->id());
}

void searchForItem::sEdit()
{
  item::editItem(_item->id());
}

void searchForItem::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());
  
  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());
  
  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sFillList()
{
  _item->clear();
  if (_search->text().trimmed().length() == 0)    
    return;

  QString sql( "SELECT item_id,"
               "       CASE WHEN item_type IN ('P','M','B','K') THEN 1"
               "       ELSE 0 "
               "       END AS alt_id, "
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       CASE WHEN (item_type='P') THEN :purchased"
               "            WHEN (item_type='M') THEN :manufactured" 
               "            WHEN (item_type='F') THEN :phantom"
               "            WHEN (item_type='B') THEN :breeder"
               "            WHEN (item_type='C') THEN :coProduct"
               "            WHEN (item_type='Y') THEN :byProduct"
               "            WHEN (item_type='R') THEN :reference"
               "            WHEN (item_type='S') THEN :costing"
               "            WHEN (item_type='T') THEN :tooling"
               "            WHEN (item_type='O') THEN :outside"
               "            WHEN (item_type='L') THEN :planning"
               "            WHEN (item_type='K') THEN :kit"
               "            ELSE :error"
               "       END AS type "
               "FROM item "
               "WHERE ( ( ((:useNumber) AND (item_number ~* :searchString))"
               "       OR ((:useDescrip1) AND (item_descrip1 ~* :searchString))"
               "       OR ((:useDescrip2) AND (item_descrip2 ~* :searchString)) )" );

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += " ) "
         "ORDER BY item_number;";

  XSqlQuery r;
  r.prepare(sql);
  r.bindValue(":purchased", tr("Purchased"));
  r.bindValue(":manufactured", tr("Manufactured"));
  r.bindValue(":phantom", tr("Phantom"));
  r.bindValue(":breeder", tr("Breeder"));
  r.bindValue(":coProduct", tr("Co-Product"));
  r.bindValue(":byProduct", tr("By-Product"));
  r.bindValue(":reference", tr("Reference"));
  r.bindValue(":costing", tr("Costing"));
  r.bindValue(":tooling", tr("Tooling"));
  r.bindValue(":outside", tr("Outside Process"));
  r.bindValue(":planning", tr("Planning"));
  r.bindValue(":kit", tr("Kit"));
  r.bindValue(":error", tr("Error"));
  r.bindValue(":useNumber", QVariant(_searchNumber->isChecked()));
  r.bindValue(":useDescrip1", QVariant(_searchDescrip1->isChecked()));
  r.bindValue(":useDescrip2", QVariant(_searchDescrip2->isChecked()));
  r.bindValue(":searchString", _search->text().toUpper());
  r.exec();
  _item->populate(r,true);
}

