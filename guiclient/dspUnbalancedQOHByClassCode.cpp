/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUnbalancedQOHByClassCode.h"

#include <QMenu>
#include <QSqlError>

#include "createCountTagsByItem.h"
#include "dspInventoryAvailabilityByItem.h"
#include "itemSite.h"
#include "storedProcErrorLookup.h"

dspUnbalancedQOHByClassCode::dspUnbalancedQOHByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);

  _itemsite->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _itemsite->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _itemsite->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _itemsite->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  _itemsite->addColumn(tr("QOH"),         _qtyColumn,   Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  _itemsite->addColumn(tr("QOH Detail."), _qtyColumn,   Qt::AlignRight,  true,  "detailedqoh"  );
  _itemsite->addColumn(tr("NN QOH"),      _qtyColumn,   Qt::AlignRight,  true,  "itemsite_nnqoh"  );
  _itemsite->addColumn(tr("NN Detail."),  _qtyColumn,   Qt::AlignRight,  true,  "detailednnqoh"  );
}

dspUnbalancedQOHByClassCode::~dspUnbalancedQOHByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUnbalancedQOHByClassCode::languageChange()
{
  retranslateUi(this);
}

void dspUnbalancedQOHByClassCode::sBalance()
{
  q.prepare("SELECT balanceItemsite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemsite->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("balanceItemsite", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspUnbalancedQOHByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byLeadTime");
  params.append("run");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUnbalancedQOHByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Balance Item Site..."), this, SLOT(sBalance()), 0);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Item Site..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Item Site..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Inventory Availability..."), this, SLOT(sInventoryAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspUnbalancedQOHByClassCode::sFillList()
{
  QString sql( "SELECT itemsite_id, warehous_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
               "       itemsite_qtyonhand,"
               "       detailedQOH(itemsite_id, FALSE) AS detailedqoh,"
               "       itemsite_nnqoh,"
               "       detailedNNQOH(itemsite_id, FALSE) AS detailednnqoh,"
               "       'qty' AS itemsite_qtyonhand_xtnumericrole,"
               "       'qty' AS detailedqoh_xtnumericrole,"
               "       'qty' AS itemsite_nnqoh_xtnumericrole,"
               "       'qty' AS detailednnqoh_xtnumericrole "
               "FROM warehous, item, itemsite, uom "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND ( (itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S')) )"
               " AND ( (itemsite_qtyonhand <> detailedQOH(itemsite_id, FALSE))"
               "    OR (itemsite_nnqoh <> detailedNNQOH(itemsite_id, FALSE)) )" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lot", tr("Lot #"));
  q.bindValue(":serial", tr("Serial #"));
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  q.exec();
  _itemsite->populate(q);
}
