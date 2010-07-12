/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCapacityUOMsByParameter.h"

#include <QMenu>
#include <QVariant>

#include "item.h"

dspCapacityUOMsByParameter::dspCapacityUOMsByParameter(QWidget* parent, const char* name, Qt::WFlags fl)
    : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Capacity UOMs"));
  setMetaSQLOptions("capacityUOMs", "detail");
}

void dspCapacityUOMsByParameter::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspCapacityUOMsByParameter::setParams(ParameterList &params)
{
  _parameter->appendValue(params);

  return true;
}

void dspCapacityUOMsByParameter::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspCapacityUOMsByParameter::sEditItem()
{
  item::editItem(list()->id());
}

