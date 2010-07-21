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

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "mqlutil.h"
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
  if (_metrics->boolean("MultiWhs"))
    _usage->addColumn(tr("Transfers"), _qtyColumn,  Qt::AlignRight,  true,  "transfer"  );

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

void dspUsageStatisticsByItem::setParams(ParameterList & params)
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
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);
  _dates->appendValue(params);
}

void dspUsageStatisticsByItem::sPrint()
{
  ParameterList params;
  setParams(params);
  if (!params.count())
    return;

  params.append("print");

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

void dspUsageStatisticsByItem::sViewTransfer()
{
  viewTransactions("T");
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
  QAction *menuItem;

  menuItem = pMenu->addAction("View All Transactions...", this, SLOT(sViewAll()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));

  switch (pColumn)
  {
    case 1:
      menuItem = pMenu->addAction("View Receipt Transactions...", this, SLOT(sViewReceipt()));
      menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
      break;

    case 2:
      menuItem = pMenu->addAction("View Issue Transactions...", this, SLOT(sViewIssue()));
      menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
      break;

    case 3:
      menuItem = pMenu->addAction("View Sold Transactions...", this, SLOT(sViewSold()));
      menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
      break;

    case 4:
      menuItem = pMenu->addAction("View Scrap Transactions...", this, SLOT(sViewScrap()));
      menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
      break;

    case 5:
      menuItem = pMenu->addAction("View Adjustment Transactions...", this, SLOT(sViewAdjustment()));
      menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
      break;

    case 6:
      menuItem = pMenu->addAction("View Transfer Transactions...", this, SLOT(sViewTransfer()));
      menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
      break;
  }
}

void dspUsageStatisticsByItem::sFillList()
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

