/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByWarehouse.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "mqlutil.h"
#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsByWarehouse::dspUsageStatisticsByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _usage->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"  );
  _usage->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"  );
  _usage->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight,  true,  "received" );
  _usage->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight,  true,  "issued" );
  _usage->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight,  true,  "sold" );
  _usage->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight,  true,  "scrap" );
  _usage->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight,  true,  "adjust" );
  if (_metrics->boolean("MultiWhs"))
    _usage->addColumn(tr("Transfers"), _qtyColumn,  Qt::AlignRight,  true,  "transfer"  );

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
}

dspUsageStatisticsByWarehouse::~dspUsageStatisticsByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUsageStatisticsByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspUsageStatisticsByWarehouse::setParams(ParameterList & params)
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
  params.append("warehous_id", _warehouse->id());
  _dates->appendValue(params);
}

void dspUsageStatisticsByWarehouse::sPrint()
{
  ParameterList params;
  setParams(params);
  if (!params.count())
    return;

  params.append("print");

  orReport report("UsageStatisticsByWarehouse", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUsageStatisticsByWarehouse::sViewTransactions()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("itemsite_id", _usage->id());
  params.append("run");

  switch (_column)
  {
    case 2:
      params.append("transtype", "R");
      break;

    case 3:
      params.append("transtype", "I");
      break;

    case 4:
      params.append("transtype", "S");
      break;

    case 5:
      params.append("transtype", "SC");
      break;

    case 6:
      params.append("transtype", "A");
      break;

    case 7:
      params.append("transtype", "T");
      break;
  }

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatisticsByWarehouse::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;

  _column = pColumn;

  intMenuItem = menuThis->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void dspUsageStatisticsByWarehouse::sFillList()
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

