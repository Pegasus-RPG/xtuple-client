/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByItemGroup.h"

#include <QMenu>
#include <QMessageBox>

#include <dbtools.h>
#include <openreports.h>
#include <parameter.h>

#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsByItemGroup::dspUsageStatisticsByItemGroup(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));

  _itemGroup->setType(ParameterGroup::ItemGroup);

  _usage->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _usage->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _usage->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _usage->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight,  true,  "received"  );
  _usage->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight,  true,  "issued"  );
  _usage->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight,  true,  "sold"  );
  _usage->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight,  true,  "scrap"  );
  _usage->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight,  true,  "adjust"  );
}

dspUsageStatisticsByItemGroup::~dspUsageStatisticsByItemGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUsageStatisticsByItemGroup::languageChange()
{
  retranslateUi(this);
}

void dspUsageStatisticsByItemGroup::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _itemGroup->appendValue(params);

  orReport report("UsageStatisticsByItemGroup", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUsageStatisticsByItemGroup::sViewAll()
{
  viewTransactions(NULL);
}

void dspUsageStatisticsByItemGroup::sViewReceipt()
{
  viewTransactions("R");
}

void dspUsageStatisticsByItemGroup::sViewIssue()
{
  viewTransactions("I");
}

void dspUsageStatisticsByItemGroup::sViewSold()
{
  viewTransactions("S");
}

void dspUsageStatisticsByItemGroup::sViewScrap()
{
  viewTransactions("SC");
}

void dspUsageStatisticsByItemGroup::sViewAdjustment()
{
  viewTransactions("A");
}

void dspUsageStatisticsByItemGroup::viewTransactions(QString pType)
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("itemsite_id", _usage->id());
  params.append("run");

  if (!pType.isNull())
    params.append("transtype", pType);

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatisticsByItemGroup::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  menuItem = pMenu->insertItem("View All Transactions...", this, SLOT(sViewAll()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  switch (pColumn)
  {
    case 3:
      menuItem = pMenu->insertItem("View Receipt Transactions...", this, SLOT(sViewReceipt()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 4:
      menuItem = pMenu->insertItem("View Issue Transactions...", this, SLOT(sViewIssue()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 5:
      menuItem = pMenu->insertItem("View Sold Transactions...", this, SLOT(sViewSold()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 6:
      menuItem = pMenu->insertItem("View Scrap Transactions...", this, SLOT(sViewScrap()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 7:
      menuItem = pMenu->insertItem("View Adjustment Transactions...", this, SLOT(sViewAdjustment()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;
  }
}

void dspUsageStatisticsByItemGroup::sFillList()
{
  _usage->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  QString sql( "SELECT itemsite_id, warehous_code,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
               "       summTransR(itemsite_id, :startDate, :endDate) AS received,"
               "       summTransI(itemsite_id, :startDate, :endDate) AS issued,"
               "       summTransS(itemsite_id, :startDate, :endDate) AS sold,"
               "       summTransC(itemsite_id, :startDate, :endDate) AS scrap,"
               "       summTransA(itemsite_id, :startDate, :endDate) AS adjust,"
               "       'qty' AS received_xtnumericrole,"
               "       'qty' AS issued_xtnumericrole,"
               "       'qty' AS sold_xtnumericrole,"
               "       'qty' AS scrap_xtnumericrole,"
               "       'qty' AS adjust_xtnumericrole "
               "FROM item, itemsite, warehous, itemgrp, itemgrpitem "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemgrpitem_itemgrp_id=itemgrp_id)"
               " AND (itemgrpitem_item_id=item_id)" );

  if (_itemGroup->isSelected())
    sql += " AND (itemgrp_id=:itemgrp_id)";
  else if (_itemGroup->isPattern())
    sql += " AND (itemgrp_name ~ :itemgrp_pattern)";

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  sql += " ) "
         "ORDER BY warehous_code, item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _itemGroup->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  _usage->populate(q);
}

