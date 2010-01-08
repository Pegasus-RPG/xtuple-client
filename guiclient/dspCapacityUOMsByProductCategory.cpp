/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCapacityUOMsByProductCategory.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "item.h"
#include "mqlutil.h"

dspCapacityUOMsByProductCategory::dspCapacityUOMsByProductCategory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _item->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  _item->addColumn(tr("Description"),    -1,          Qt::AlignLeft,  true, "descrip");
  _item->addColumn(tr("Inv. UOM"),       _uomColumn,  Qt::AlignCenter,true, "uom_name");
  _item->addColumn(tr("Cap. UOM"),       _uomColumn,  Qt::AlignCenter,true, "capuom");
  _item->addColumn(tr("Cap./Inv. Rat."), _qtyColumn,  Qt::AlignRight, true, "capinvrat");
  _item->addColumn(tr("Alt. UOM"),       _uomColumn,  Qt::AlignCenter,true, "altcapuom");
  _item->addColumn(tr("Alt/Inv Ratio"),  _qtyColumn,  Qt::AlignRight, true, "altcapinvrat");
}

dspCapacityUOMsByProductCategory::~dspCapacityUOMsByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCapacityUOMsByProductCategory::languageChange()
{
  retranslateUi(this);
}

bool dspCapacityUOMsByProductCategory::setParams(ParameterList &params)
{
  _productCategory->appendValue(params);
  return true;
}

void dspCapacityUOMsByProductCategory::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("CapacityUOMsByProductCategory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCapacityUOMsByProductCategory::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspCapacityUOMsByProductCategory::sEditItem()
{
  item::editItem(_item->id());
}

void dspCapacityUOMsByProductCategory::sFillList()
{
  sFillList(-1, FALSE);
}

void dspCapacityUOMsByProductCategory::sFillList(int pItemid, bool pLocalUpdate)
{
  MetaSQLQuery mql = mqlLoad("capacityUOMs", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  if ((pItemid != -1) && (pLocalUpdate))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
