/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspFrozenItemSites.h"

#include <QMenu>
#include <QSqlError>

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
  int menuItem;

  menuItem = pMenu->insertItem(tr("Thaw"), this, SLOT(sThaw()), 0);
  if (!_privileges->check("ThawInventory"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspFrozenItemSites::sThaw()
{
  q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemsite->id());
  q.exec();

  sFillList();
}

void dspFrozenItemSites::sFillList()
{
  QString sql( "SELECT itemsite_id, warehous_code, item_number,"
               " (item_descrip1 || ' ' || item_descrip2) AS descrip,"
               " COALESCE((SELECT invcnt_tagnumber"
               "           FROM invcnt"
               "           WHERE ((NOT invcnt_posted)"
               "           AND (invcnt_itemsite_id=itemsite_id)) LIMIT 1), '') AS cnttag "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_freeze)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += " ) "
         "ORDER BY warehous_code, item_number";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();
  _itemsite->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
