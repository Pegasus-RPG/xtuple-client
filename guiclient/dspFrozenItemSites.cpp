/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspFrozenItemSites.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

dspFrozenItemSites::dspFrozenItemSites(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _itemsite->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  _itemsite->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _itemsite->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "descrip");
  _itemsite->addColumn(tr("Count Tag #"), _qtyColumn,  Qt::AlignRight, true, "cnttag");

  sFillList();
}

dspFrozenItemSites::~dspFrozenItemSites()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspFrozenItemSites::languageChange()
{
  retranslateUi(this);
}

void dspFrozenItemSites::sPrint()
{
  ParameterList params;

  _warehouse->appendValue(params);

  orReport report("FrozenItemSites", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspFrozenItemSites::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Thaw"), this, SLOT(sThaw()));
  menuItem->setEnabled(_privileges->check("ThawInventory"));
}

void dspFrozenItemSites::sThaw()
{
  q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemsite->id());
  q.exec();

  sFillList();
}

bool dspFrozenItemSites::setParams(ParameterList &params)
{
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id()); 
  return true;
}


void dspFrozenItemSites::sFillList()
{
  MetaSQLQuery mql = mqlLoad("frozenItemSites", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _itemsite->populate(q);
}
