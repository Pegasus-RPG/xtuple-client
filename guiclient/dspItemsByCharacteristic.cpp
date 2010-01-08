/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemsByCharacteristic.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "bom.h"
#include "item.h"
#include "mqlutil.h"
#include "guiclient.h"

dspItemsByCharacteristic::dspItemsByCharacteristic(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_items) "
                   "ORDER BY char_name;" );

  _item->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  _item->addColumn(tr("Description"),    -1,          Qt::AlignLeft,  true, "descrip");
  _item->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignCenter,true, "char_name");
  _item->addColumn(tr("Value"),          _itemColumn, Qt::AlignLeft,  true, "charass_value");
  _item->addColumn(tr("Type"),           _itemColumn, Qt::AlignCenter,true, "type");
  _item->addColumn(tr("UOM"),            _uomColumn,  Qt::AlignCenter,true, "uom_name");

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

dspItemsByCharacteristic::~dspItemsByCharacteristic()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemsByCharacteristic::languageChange()
{
  retranslateUi(this);
}

bool dspItemsByCharacteristic::setParams(ParameterList &params)
{
  params.append("char_id",      _char->id());
  params.append("value",        _value->text().trimmed());
  params.append("purchased",    tr("Purchased"));
  params.append("manufactured", tr("Manufactured"));
  params.append("job",          tr("Job"));
  params.append("phantom",      tr("Phantom"));
  params.append("breeder",      tr("Breeder"));
  params.append("coProduct",    tr("Co-Product"));
  params.append("byProduct",    tr("By-Product"));
  params.append("reference",    tr("Reference"));
  params.append("costing",      tr("Costing"));
  params.append("tooling",      tr("Tooling"));
  params.append("outside",      tr("Outside Process"));
  params.append("kit",          tr("Kit"));
  params.append("error",        tr("Error"));
  params.append("byCharacteristic");

  if(_showInactive->isChecked())
    params.append("showInactive");

  return true;
}

void dspItemsByCharacteristic::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ItemsByCharacteristic", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemsByCharacteristic::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  XTreeWidgetItem * xselected = static_cast<XTreeWidgetItem*>(selected);
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item Master..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Item Master..."), this, SLOT(sView()), 0);

  if (xselected && (xselected->rawValue("type").toString() == "M"))
  {
    menuItem = pMenu->insertItem(tr("Edit Bill of Material..."), this, SLOT(sEditBOM()), 0);
    if (!_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Bill of Material..."), this, SLOT(sViewBOM()), 0);
    if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspItemsByCharacteristic::sEdit()
{
  item::editItem(_item->id());
}

void dspItemsByCharacteristic::sView()
{
  item::viewItem(_item->id());
}

void dspItemsByCharacteristic::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sFillList()
{
  sFillList(-1, TRUE);
}

void dspItemsByCharacteristic::sFillList(int pItemid, bool pLocal)
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("products", "items");
  q = mql.toQuery(params);
  if ((pItemid != -1) && (pLocal))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _item->setDragString("itemid=");
}
