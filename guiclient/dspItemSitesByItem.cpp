/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSitesByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "inputManager.h"
#include "itemSite.h"
#include "dspInventoryAvailabilityByItem.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "guiclient.h"

dspItemSitesByItem::dspItemSitesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setId(int)));

  _itemsite->addColumn(tr("Site"),          _whsColumn, Qt::AlignCenter, true, "warehous_code");
  _itemsite->addColumn(tr("QOH"),                   -1, Qt::AlignRight, true, "itemsite_qtyonhand");
  _itemsite->addColumn(tr("Loc. Cntrl."), _orderColumn, Qt::AlignCenter,true, "itemsite_loccntrl");
  _itemsite->addColumn(tr("Cntrl. Meth."), _itemColumn, Qt::AlignCenter,true, "controlmethod");
  _itemsite->addColumn(tr("Sold Ranking"), _itemColumn, Qt::AlignCenter,true, "soldranking");
  _itemsite->addColumn(tr("Last Cnt'd"),   _dateColumn, Qt::AlignCenter,true, "itemsite_datelastcount");
  _itemsite->addColumn(tr("Last Used"),    _dateColumn, Qt::AlignCenter,true, "itemsite_datelastused");
  _itemsite->setDragString("itemsiteid=");
  
  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillList()));

  _item->setFocus();
}

dspItemSitesByItem::~dspItemSitesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemSitesByItem::languageChange()
{
  retranslateUi(this);
}

void dspItemSitesByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

//  Ack - HackHackHack
  params.append("placeHolder", -1);

  if (_showInactive->isChecked())
    params.append("showInactive");

  orReport report("ItemSitesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSitesByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Item Site..."), this, SLOT(sViewItemsite()), 0);
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Item Site..."), this, SLOT(sEditItemsite()), 0);
  if (!_privileges->check("MaintainItemSites"))
    menu->setItemEnabled(menuItem, FALSE);

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("View Inventory Availability..."), this, SLOT(sViewInventoryAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    menu->setItemEnabled(menuItem, FALSE);

  if (((XTreeWidgetItem *)pSelected)->altId() == 1)
  {
    menuItem = menu->insertItem(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewLocationLotSerialDetail()), 0);
    if (!_privileges->check("ViewQOH"))
      menu->setItemEnabled(menuItem, FALSE);
  }

  menu->insertSeparator();

  menuItem = menu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspItemSitesByItem::sViewItemsite()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sEditItemsite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sViewInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byLeadTime");
  params.append("run");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByItem::sViewLocationLotSerialDetail()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sFillList()
{
  QString sql( "SELECT itemsite_id,"
               "       CASE WHEN ( (itemsite_loccntrl) OR"
               "                   (itemsite_controlmethod IN ('L', 'S')) ) THEN 1"
               "            ELSE 0"
               "       END,"
               "       warehous_code,"
               "       itemsite_qtyonhand,"
               "       itemsite_loccntrl,"
               "       CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "            WHEN itemsite_controlmethod='N' THEN :none"
               "            WHEN itemsite_controlmethod='L' THEN :lot"
               "            WHEN itemsite_controlmethod='S' THEN :serial"
               "       END AS controlmethod,"
               "       CASE WHEN (itemsite_sold) THEN itemsite_soldranking"
               "       END AS soldranking,"
               "       itemsite_datelastcount,"
               "       itemsite_datelastused,"
               "       'qty' AS itemsite_qtyonhand_xtnumericrole,"
               "       :na AS soldranking_xtnullrole,"
               "       :never AS itemsite_datelastused_xtnullrole,"
               "       :never AS itemsite_datelastused_xtnullrole "
               "FROM itemsite, warehous "
               "WHERE ( (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=:item_id)" );

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  sql += " )"
         "ORDER BY warehous_code;";

  q.prepare(sql);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lot", tr("Lot #"));
  q.bindValue(":serial", tr("Serial #"));
  q.bindValue(":na", tr("N/A"));
  q.bindValue(":never", tr("Never"));
  q.bindValue(":item_id", _item->id());
  q.exec();
  _itemsite->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
