/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByClassCode.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "mqlutil.h"
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
  if (_metrics->boolean("MultiWhs"))
    _usage->addColumn(tr("Transfers"), _qtyColumn,  Qt::AlignRight,  true,  "transfer"  );

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

void dspUsageStatisticsByClassCode::setParams(ParameterList & params)
{
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

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  _warehouse->appendValue(params);
  _classCode->appendValue(params);
  _dates->appendValue(params);
}

void dspUsageStatisticsByClassCode::sPrint()
{
  ParameterList params;
  setParams(params);
  if (!params.count())
    return;

  params.append("print");

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

void dspUsageStatisticsByClassCode::sViewTransfer()
{
  viewTransactions("T");
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
  QAction *menuItem;

  menuItem = pMenu->addAction("View All Transactions...", this, SLOT(sViewAll()));
  if (!_privileges->check("ViewInventoryHistory"))
    menuItem->setEnabled(false);

  switch (pColumn)
  {
    case 3:
      menuItem = pMenu->addAction("View Receipt Transactions...", this, SLOT(sViewReceipt()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 4:
      menuItem = pMenu->addAction("View Issue Transactions...", this, SLOT(sViewIssue()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 5:
      menuItem = pMenu->addAction("View Sold Transactions...", this, SLOT(sViewSold()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 6:
      menuItem = pMenu->addAction("View Scrap Transactions...", this, SLOT(sViewScrap()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 7:
      menuItem = pMenu->addAction("View Adjustment Transactions...", this, SLOT(sViewAdjustment()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;

    case 8:
      menuItem = pMenu->addAction("View Transfer Transactions...", this, SLOT(sViewTransfer()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);
      break;
  }
}

void dspUsageStatisticsByClassCode::sFillList()
{
  _usage->clear();

  ParameterList params;
  setParams(params);
  if (!params.count())
    return;
  MetaSQLQuery mql = mqlLoad("usageStatistics", "detail");
  q = mql.toQuery(params);

  if (q.first())
  {
    _usage->populate(q, true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

