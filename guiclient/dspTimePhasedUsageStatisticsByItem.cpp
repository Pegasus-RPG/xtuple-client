/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedUsageStatisticsByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <datecluster.h>
#include <parameter.h>
#include <metasql.h>

#include "dspInventoryHistoryByItem.h"
#include "guiclient.h"
#include "submitReport.h"
#include "mqlutil.h"

dspTimePhasedUsageStatisticsByItem::dspTimePhasedUsageStatisticsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));

  _usage->addColumn(tr("Transaction Type"), 120,        Qt::AlignLeft   );
  _usage->addColumn(tr("Site"),             _whsColumn, Qt::AlignCenter );

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

dspTimePhasedUsageStatisticsByItem::~dspTimePhasedUsageStatisticsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTimePhasedUsageStatisticsByItem::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedUsageStatisticsByItem::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedStatisticsByItem", buildParameters());
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      return;
    }
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

void dspTimePhasedUsageStatisticsByItem::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedStatisticsByItem");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedStatisticsByItem\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedUsageStatisticsByItem::buildParameters()
{
  ParameterList params;

  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  params.append("calendar_id", _calendar->id());

  QList<QVariant> periodList;
  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());
  params.append("period_id_list", periodList);

  return params;
}

void dspTimePhasedUsageStatisticsByItem::sViewTransactions()
{
  if (_column > 1)
  {
    ParameterList params;
    params.append("itemsite_id", _usage->id());
    params.append("startDate",   _columnDates[_column - 2].startDate);
    params.append("endDate",     _columnDates[_column - 2].endDate);
    params.append("run");

    QString type = _usage->currentItem()->text(0);
    if (type == "Received")
      params.append("transtype", "R");
    else if (type == "Issued")
      params.append("transtype", "I");
    else if (type == "Sold")
      params.append("transtype", "S");
    else if (type == "Scrap")
      params.append("transtype", "SC");
    else if (type == "Adjustments")
      params.append("transtype", "A");

    dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedUsageStatisticsByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  menuItem = menu->addAction(tr("View Transactions..."), this, SLOT(sViewTransactions()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
}

void dspTimePhasedUsageStatisticsByItem::sCalculate()
{
  if (TRUE)
  {
    _columnDates.clear();
    _usage->clear();
    _usage->setColumnCount(2);

    int columns = 1;

    ParameterList params;
    if (! setParams(params))
      return;

    QList<XTreeWidgetItem*> selected = _periods->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
      columns++;

      _usage->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight);

      _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
    }

    MetaSQLQuery mql = mqlLoad("timePhasedUsageStatisticsByItem", "detail");
    q = mql.toQuery(params);
    if (q.first())
    {
      do
      {
        XTreeWidgetItem *received;
        XTreeWidgetItem *issued;
        XTreeWidgetItem *sold;
        XTreeWidgetItem *scrap;
        XTreeWidgetItem *adjustments;

        received    = new XTreeWidgetItem(_usage,           q.value("itemsite_id").toInt(), QVariant(tr("Received")),    q.value("warehous_code") );
        issued      = new XTreeWidgetItem(_usage, received, q.value("itemsite_id").toInt(), QVariant(tr("Issued")),      q.value("warehous_code") );
        sold        = new XTreeWidgetItem(_usage, issued,   q.value("itemsite_id").toInt(), QVariant(tr("Sold")),        q.value("warehous_code") );
        scrap       = new XTreeWidgetItem(_usage, sold,     q.value("itemsite_id").toInt(), QVariant(tr("Scrap")),       q.value("warehous_code") );
        adjustments = new XTreeWidgetItem(_usage, scrap,    q.value("itemsite_id").toInt(), QVariant(tr("Adjustments")), q.value("warehous_code") );

        QList<XTreeWidgetItem*> selected = _periods->selectedItems();
        for (int i = 0; i < selected.size(); i++)
        {
          PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];

          received->setText((i + 2), formatQty(q.value(QString("r_bucket_%1").arg(cursor->id())).toDouble()));
          issued->setText((i + 2), formatQty(q.value(QString("i_bucket_%1").arg(cursor->id())).toDouble()));
          sold->setText((i + 2), formatQty(q.value(QString("s_bucket_%1").arg(cursor->id())).toDouble()));
          scrap->setText((i + 2), formatQty(q.value(QString("c_bucket_%1").arg(cursor->id())).toDouble()));
          adjustments->setText((i + 2), formatQty(q.value(QString("a_bucket_%1").arg(cursor->id())).toDouble()));
        }
      }
      while (q.next());
    }
  }
}

bool dspTimePhasedUsageStatisticsByItem::setParams(ParameterList & params)
{
  params.append("period_list", _periods->periodList());

  params.append("item_id", _item->id());

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

   return true;
}
