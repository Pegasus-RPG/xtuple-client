/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByClassCode.h"

#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsByClassCode::dspUsageStatisticsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));

  _classCode->setType(ParameterGroup::ClassCode);

  _usage->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _usage->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _usage->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _usage->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight,  true,  "received"  );
  _usage->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight,  true,  "issued"  );
  _usage->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight,  true,  "sold"  );
  _usage->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight,  true,  "scrap"  );
  _usage->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight,  true,  "adjust"  );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);

}

dspUsageStatisticsByClassCode::~dspUsageStatisticsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUsageStatisticsByClassCode::languageChange()
{
  retranslateUi(this);
}

void dspUsageStatisticsByClassCode::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("print");
  _warehouse->appendValue(params);
  _classCode->appendValue(params);

  orReport report("UsageStatisticsByClassCode", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspUsageStatisticsByClassCode::sViewAll()
{
  viewTransactions(QString::null);
}

void dspUsageStatisticsByClassCode::sViewReceipt()
{
  viewTransactions("R");
}

void dspUsageStatisticsByClassCode::sViewIssue()
{
  viewTransactions("I");
}

void dspUsageStatisticsByClassCode::sViewSold()
{
  viewTransactions("S");
}

void dspUsageStatisticsByClassCode::sViewScrap()
{
  viewTransactions("SC");
}

void dspUsageStatisticsByClassCode::sViewAdjustment()
{
  viewTransactions("A");
}

void dspUsageStatisticsByClassCode::viewTransactions(QString pType)
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

void dspUsageStatisticsByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
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

void dspUsageStatisticsByClassCode::sFillList()
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
               "FROM item, itemsite, warehous "
               "WHERE ((itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY warehous_code, item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  _usage->populate(q);
}

