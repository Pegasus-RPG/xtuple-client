/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "searchForItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include "item.h"
#include "bom.h"
#include "boo.h"
#include "bbom.h"

/*
 *  Constructs a searchForItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
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

/*
 *  Destroys the object and frees any allocated resources
 */
searchForItem::~searchForItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void searchForItem::languageChange()
{
  retranslateUi(this);
}

void searchForItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  bool hasBOM = false;
  q.prepare("SELECT (count(*) != 0) AS hasBOM"
            "  FROM bomhead"
            " WHERE (bomhead_item_id=:item_id); ");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if(q.first())
    hasBOM = q.value("hasBOM").toBool();

  bool hasBOO = false;
  if (_metrics->value("Application") == "Manufacturing")
  {
    q.prepare("SELECT (count(*) != 0) AS hasBOO"
              "  FROM boohead"
              " WHERE (boohead_item_id=:item_id); ");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if(q.first())
      hasBOO = q.value("hasBOO").toBool();
  }
  
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if ((((XTreeWidgetItem *)pSelected)->text(2) == tr("Purchased")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Manufactured")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Job")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Breeder")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Kit")))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View BOM..."), this, SLOT(sViewBOM()), 0);
    if (!hasBOM || !_privileges->check("ViewBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit BOM..."), this, SLOT(sEditBOM()), 0);
    if (!hasBOM || !_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((((XTreeWidgetItem *)pSelected)->text(2) == tr("Purchased") ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Job")) ||
      (((XTreeWidgetItem *)pSelected)->text(2) == tr("Manufactured")))
      && _metrics->boolean("Routings"))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View BOO..."), this, SLOT(sViewBOO()), 0);
    if (!hasBOO || !_privileges->check("ViewBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit BOO..."), this, SLOT(sEditBOO()), 0);
    if (!hasBOO || !_privileges->check("MaintainBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if (((XTreeWidgetItem *)pSelected)->text(2) == tr("Breeder"))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View Breeder BOM..."), this, SLOT(sViewBBOM()), 0);
    if (!_privileges->check("ViewBBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit Breeder BOM..."), this, SLOT(sEditBBOM()), 0);
    if (!_privileges->check("MaintainBBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);
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

void searchForItem::sViewBOO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());
  
  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());
  
  boo *newdlg = new boo();
  newdlg->set(params);
  newdlg->show();
}

void searchForItem::sViewBBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());
  
  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sEditBBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());
  
  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void searchForItem::sFillList()
{
  _item->clear();
  if (_search->text().trimmed().length() == 0)    
    return;

  QString sql( "SELECT item_id,"
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
               "            WHEN (item_type='A') THEN :assortment"
               "            WHEN (item_type='O') THEN :outside"
               "            WHEN (item_type='J') THEN :job"
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

  q.prepare(sql);
  q.bindValue(":purchased", tr("Purchased"));
  q.bindValue(":manufactured", tr("Manufactured"));
  q.bindValue(":phantom", tr("Phantom"));
  q.bindValue(":breeder", tr("Breeder"));
  q.bindValue(":coProduct", tr("Co-Product"));
  q.bindValue(":byProduct", tr("By-Product"));
  q.bindValue(":reference", tr("Reference"));
  q.bindValue(":costing", tr("Costing"));
  q.bindValue(":tooling", tr("Tooling"));
  q.bindValue(":outside", tr("Outside Process"));
  q.bindValue(":assortment", tr("Assortment"));
  q.bindValue(":job", tr("Job"));
  q.bindValue(":planning", tr("Planning"));
  q.bindValue(":kit", tr("Kit"));
  q.bindValue(":error", tr("Error"));
  q.bindValue(":useNumber", QVariant(_searchNumber->isChecked()));
  q.bindValue(":useDescrip1", QVariant(_searchDescrip1->isChecked()));
  q.bindValue(":useDescrip2", QVariant(_searchDescrip2->isChecked()));
  q.bindValue(":searchString", _search->text().toUpper());
  q.exec();
  _item->populate(q);
}

