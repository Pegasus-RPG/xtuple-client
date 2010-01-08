/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "inputManager.h"
#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsByItem::dspUsageStatisticsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _usage->addColumn(tr("Site"),        -1,         Qt::AlignCenter, true,  "warehous_code" );
  _usage->addColumn(tr("Received"),    _qtyColumn, Qt::AlignRight,  true,  "received"  );
  _usage->addColumn(tr("Issued"),      _qtyColumn, Qt::AlignRight,  true,  "issued"  );
  _usage->addColumn(tr("Sold"),        _qtyColumn, Qt::AlignRight,  true,  "sold"  );
  _usage->addColumn(tr("Scrap"),       _qtyColumn, Qt::AlignRight,  true,  "scrap"  );
  _usage->addColumn(tr("Adjustments"), _qtyColumn, Qt::AlignRight,  true,  "adjust"  );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
}

dspUsageStatisticsByItem::~dspUsageStatisticsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUsageStatisticsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspUsageStatisticsByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspUsageStatisticsByItem::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Invalid Dates"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  params.append("item_id", _item->id());

  _warehouse->appendValue(params);

  orReport report("UsageStatisticsByItem", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspUsageStatisticsByItem::sViewAll()
{
  viewTransactions(NULL);
}

void dspUsageStatisticsByItem::sViewReceipt()
{
  viewTransactions("R");
}

void dspUsageStatisticsByItem::sViewIssue()
{
  viewTransactions("I");
}

void dspUsageStatisticsByItem::sViewSold()
{
  viewTransactions("S");
}

void dspUsageStatisticsByItem::sViewScrap()
{
  viewTransactions("SC");
}

void dspUsageStatisticsByItem::sViewAdjustment()
{
  viewTransactions("A");
}

void dspUsageStatisticsByItem::viewTransactions(QString pType)
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

void dspUsageStatisticsByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;

  intMenuItem = pMenu->insertItem("View All Transactions...", this, SLOT(sViewAll()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    pMenu->setItemEnabled(intMenuItem, FALSE);

  switch (pColumn)
  {
    case 1:
      intMenuItem = pMenu->insertItem("View Receipt Transactions...", this, SLOT(sViewReceipt()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(intMenuItem, FALSE);
      break;

    case 2:
      intMenuItem = pMenu->insertItem("View Issue Transactions...", this, SLOT(sViewIssue()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(intMenuItem, FALSE);
      break;

    case 3:
      intMenuItem = pMenu->insertItem("View Sold Transactions...", this, SLOT(sViewSold()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(intMenuItem, FALSE);
      break;

    case 4:
      intMenuItem = pMenu->insertItem("View Scrap Transactions...", this, SLOT(sViewScrap()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(intMenuItem, FALSE);
      break;

    case 5:
      intMenuItem = pMenu->insertItem("View Adjustment Transactions...", this, SLOT(sViewAdjustment()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(intMenuItem, FALSE);
      break;
  }
}

void dspUsageStatisticsByItem::sFillList()
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
               "FROM itemsite, warehous "
               "WHERE ((itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=:item_id) " );

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":item_id", _item->id());
  q.exec();
  _usage->populate(q);
}

