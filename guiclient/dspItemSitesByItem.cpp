/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
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

#include <metasql.h>
#include "mqlutil.h"

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
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("itemSites", "detail");
  q = mql.toQuery(params);
  _itemsite->populate(q, true); 
}

bool dspItemSitesByItem::setParams(ParameterList &params)
{
  params.append("byItem");  

  params.append("regular", tr("Regular"));
  params.append("none", tr("None"));
  params.append("lot", tr("Lot #"));
  params.append("serial", tr("Serial #"));
  params.append("na", tr("N/A"));
  params.append("never", tr("Never"));
  
  if (_showInactive->isChecked())
    params.append("showInactive");  
  
  if (_item->isValid())
    params.append("item_id", _item->id());
  else
  {
    _item->setFocus();
    return false;
  }

  return true;
}
