/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "items.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>

#include "copyItem.h"
#include "item.h"
#include "storedProcErrorLookup.h"

items::items(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _statusGroupInt = new QButtonGroup(this);
  _statusGroupInt->addButton(_showAll);
  _statusGroupInt->addButton(_showPurchased);
  _statusGroupInt->addButton(_showManufactured);
  _statusGroupInt->addButton(_showSold);

  // signals and slots connections
  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_statusGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_item, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _item->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   , true, "item_number" );
  _item->addColumn(tr("Active"),      _ynColumn,   Qt::AlignCenter , true, "item_active" );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft   , true, "item_descrip" );
  _item->addColumn(tr("Class Code"),  _dateColumn, Qt::AlignCenter , true, "classcode_code");
  _item->addColumn(tr("Type"),        _itemColumn, Qt::AlignCenter , true, "item_type");
  _item->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter , true, "uom_name");
  _item->setDragString("itemid=");
  
  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  
  if (_privileges->check("MaintainItemMasters"))
  {
    connect(_item, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_item, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (_privileges->check("DeleteItemMasters"))
    connect(_item, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  sFillList();

  _searchFor->setFocus();
}

items::~items()
{
  // no need to delete child widgets, Qt does it all for us
}

void items::languageChange()
{
  retranslateUi(this);
}

void items::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Copy..."), this, SLOT(sCopy()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  QAction *tmpaction = pMenu->addAction(tr("Delete..."));
  connect(tmpaction, SIGNAL(triggered()), this, SLOT(sDelete()));
  tmpaction->setEnabled(_privileges->check("MaintainItemMasters"));
  tmpaction->setObjectName("items.popup.delete");
}

void items::sNew()
{
  item::newItem();
}

void items::sEdit()
{
  item::editItem(_item->id());
}

void items::sView()
{
  item::viewItem(_item->id());
}

void items::sDelete()
{
  q.prepare("SELECT deleteItem(:item_id) AS result;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteItem", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void items::sFillList( int pItemid, bool pLocal )
{
  QString sql( "SELECT item_id, item_number, item_active,"
               "       (item_descrip1 || ' ' || item_descrip2) as item_descrip, classcode_code,"
               "       CASE WHEN (item_type='P') THEN text(<? value(\"purchased\") ?>)"
               "            WHEN (item_type='M') THEN text(<? value(\"manufactured\") ?>)"
			   "            WHEN (item_type='J') THEN text(<? value(\"job\") ?>)"
               "            WHEN (item_type='F') THEN text(<? value(\"phantom\") ?>)"
               "            WHEN (item_type='B') THEN text(<? value(\"breeder\") ?>)"
               "            WHEN (item_type='C') THEN text(<? value(\"coProduct\") ?>)"
               "            WHEN (item_type='Y') THEN text(<? value(\"byProduct\") ?>)"
               "            WHEN (item_type='R') THEN text(<? value(\"reference\") ?>)"
               "            WHEN (item_type='S') THEN text(<? value(\"costing\") ?>)"
               "            WHEN (item_type='T') THEN text(<? value(\"tooling\") ?>)"
               "            WHEN (item_type='A') THEN text(<? value(\"assortment\") ?>)"
               "            WHEN (item_type='O') THEN text(<? value(\"outside\") ?>)"
               "            WHEN (item_type='L') THEN text(<? value(\"planning\") ?>)"
               "            WHEN (item_type='K') THEN text(<? value(\"kit\") ?>)"
               "            ELSE text(<? value(\"error\") ?>)"
               "       END AS item_type,"
               "       uom_name "
               "FROM item, classcode, uom "
               "WHERE ( (item_classcode_id=classcode_id)"
               " AND (item_inv_uom_id=uom_id)"
               "<? if exists(\"showPurchased\") ?>"
               " AND (item_type IN ('P', 'O'))"
               "<? elseif exists(\"showManufactured\") ?>"
               " AND (item_type IN ('M', 'F', 'B', 'J','K'))"
               "<? elseif exists(\"showSold\") ?>"
               " AND (item_sold)"
               "<? endif ?>"
               "<? if exists(\"onlyShowActive\") ?>"
               " AND (item_active)"
               "<? endif ?>"
               ") ORDER BY"
               "<? if exists(\"ListNumericItemNumbersFirst\") ?>"
               " toNumeric(item_number, 999999999999999),"
               "<? endif ?>"
               " item_number;" );

  ParameterList params;

  if(_showPurchased->isChecked())
    params.append("showPurchased");
  else if(_showManufactured->isChecked())
    params.append("showManufactured");
  else if(_showSold->isChecked())
    params.append("showSold");

  if (!_showInactive->isChecked())
    params.append("onlyShowActive");

  if (_preferences->boolean("ListNumericItemNumbersFirst"))
    params.append("ListNumericItemNumbersFirst");
  
  params.append("purchased", tr("Purchased"));
  params.append("manufactured", tr("Manufactured"));
  params.append("job", tr("Job"));
  params.append("phantom", tr("Phantom"));
  params.append("breeder", tr("Breeder"));
  params.append("coProduct", tr("Co-Product"));
  params.append("byProduct", tr("By-Product"));
  params.append("reference", tr("Reference"));
  params.append("costing", tr("Costing"));
  params.append("tooling", tr("Tooling"));
  params.append("outside", tr("Outside Process"));
  params.append("planning", tr("Planning"));
  params.append("assortment", tr("Assortment"));
  params.append("kit", tr("Kit"));
  params.append("error", tr("Error"));

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  if ((pItemid != -1) && (pLocal))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
}

void items::sFillList()
{
  sFillList(-1, FALSE);
}

void items::sSearch( const QString &pTarget )
{
  _item->clearSelection();
  int i;
  for (i = 0; i < _item->topLevelItemCount(); i++)
  {
    if (_item->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _item->topLevelItemCount())
  {
    _item->setCurrentItem(_item->topLevelItem(i));
    _item->scrollToItem(_item->topLevelItem(i));
  }
}

void items::sCopy()
{
  ParameterList params;
  params.append("item_id", _item->id());

  copyItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
