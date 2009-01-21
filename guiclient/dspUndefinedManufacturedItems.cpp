/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUndefinedManufacturedItems.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <parameter.h>
#include "boo.h"
#include "bom.h"
#include "item.h"

/*
 *  Constructs a dspUndefinedManufacturedItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUndefinedManufacturedItems::dspUndefinedManufacturedItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _boo->setChecked(true);
    _bom->setChecked(true);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUndefinedManufacturedItems::~dspUndefinedManufacturedItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUndefinedManufacturedItems::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspUndefinedManufacturedItems::init()
{
//  statusBar()->hide();
  
  if (!_privileges->check("ViewBOMs"))
  {
    _bom->setChecked(FALSE);
    _bom->setEnabled(FALSE);
  }

  if (!_privileges->check("ViewBOOs"))
  {
    _boo->setChecked(FALSE);
    _boo->setEnabled(FALSE);
  }

  _item->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _item->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _item->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter, true,  "item_type" );
  _item->addColumn(tr("Active"),      _orderColumn, Qt::AlignCenter, true,  "item_active" );
  _item->addColumn(tr("Exception"),   _itemColumn,  Qt::AlignCenter, true,  "exception" );

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

void dspUndefinedManufacturedItems::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (((XTreeWidgetItem *)_item->currentItem())->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Create BOO..."), this, SLOT(sCreateBOO()), 0);
    if (!_privileges->check("MaintainBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (((XTreeWidgetItem *)_item->currentItem())->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Create BOM..."), this, SLOT(sCreateBOM()), 0);
    if (!_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspUndefinedManufacturedItems::sCreateBOO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
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
  if ((_boo->isChecked()) || (_bom->isChecked()))
  {
    QString sql;

    if (_boo->isChecked())
    {
      sql = "SELECT item_id, 1, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
            "       item_type, item_active, :noBoo AS exception "
            "FROM item "
            "WHERE ( (item_type='M')"
            " AND (item_id NOT IN (SELECT DISTINCT booitem_item_id FROM booitem) )";

      if (!_showInactive->isChecked())
        sql += " AND (item_active) ";

      sql += ") ";
    }

    if (_bom->isChecked())
    {
      if (_boo->isChecked())
        sql += "UNION ";

      sql += "SELECT item_id, 2, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
             "       item_type, item_active, :noBom AS exception "
             "FROM item "
             "WHERE ( (item_type IN ('M', 'F'))"
             " AND (item_id NOT IN (SELECT DISTINCT bomitem_parent_item_id FROM bomitem) )";

      if (!_showInactive->isChecked())
        sql += " AND (item_active) ";

      sql += ") ";
    }

    sql += "ORDER BY item_number;";

    q.prepare(sql);
    q.bindValue(":noBoo", tr("No BOO"));
    q.bindValue(":noBom", tr("No BOM"));
    q.exec();

    if ((pItemid != -1) && (pLocal))
      _item->populate(q, pItemid, TRUE);
    else
      _item->populate(q, TRUE);
  }
  else
    _item->clear();
}

