/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedRoughCutByWorkCenter.h"

#include <QMessageBox>
#include <QVariant>

#include <datecluster.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"

dspTimePhasedRoughCutByWorkCenter::dspTimePhasedRoughCutByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _roughCut->addColumn("", 80, Qt::AlignRight);
}

dspTimePhasedRoughCutByWorkCenter::~dspTimePhasedRoughCutByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspTimePhasedRoughCutByWorkCenter::languageChange()
{
    retranslateUi(this);
}

void dspTimePhasedRoughCutByWorkCenter::sPrint()
{
  // TODO: why is this so different from sFillList()?
  if (_periods->isPeriodSelected())
  {
    ParameterList params;
    _warehouse->appendValue(params);

    if(_selectedWorkCenter->isChecked())
      params.append("wrkcnt_id", _workCenters->id());

    QList<XTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);
      
    orReport report("TimePhasedRoughCutByWorkCenter", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
  else
    QMessageBox::critical(this, tr("Incomplete criteria"),
			  tr("<p>The criteria you specified are not complete. "
			     "Please make sure all fields are correctly filled out "
			     "before running the report." ) );

}

void dspTimePhasedRoughCutByWorkCenter::sFillList()
{
  _columnDates.clear();
  _roughCut->clear();
  _roughCut->setColumnCount(1);

  QString sql("SELECT");
  int     columns = 1;
  bool    show    = FALSE;
  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++, columns++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    if (show)
      sql += ",";
    else
      show = TRUE;

    sql += QString( " SUM(plannedSetupTime(wrkcnt_id, %1)) AS setup%2,"
		    " SUM(plannedSetupTime(wrkcnt_id, %3) * wrkcnt_setuprate / 60.0) AS setupcost%4,"
		    " SUM(plannedRunTime(wrkcnt_id, %5)) AS run%6,"
		    " SUM(plannedRunTime(wrkcnt_id, %7) * wrkcnt_runrate / 60.0) AS runcost%8" )
	   .arg(cursor->id())
	   .arg(columns)
	   .arg(cursor->id())
	   .arg(columns)
	   .arg(cursor->id())
	   .arg(columns)
	   .arg(cursor->id())
	   .arg(columns);

    _roughCut->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM wrkcnt ";

  if (_warehouse->isSelected())
    sql += "WHERE (wrkcnt_warehous_id=:warehous_id)";

  if (_selectedWorkCenter->isChecked())
  {
    if (_warehouse->isSelected())
      sql += " AND (wrkcnt_id=:wrkcnt_id);";
    else
      sql += "WHERE (wrkcnt_id=:wrkcnt_id);";
  }

  if (show)
  {
    q.prepare(sql);
    _warehouse->bindValue(q);
    q.bindValue(":wrkcnt_id", _workCenters->id());
    q.exec();
    if (q.first())
    {
      XTreeWidgetItem *setup     = new XTreeWidgetItem(_roughCut, 0, QVariant(tr("Setup Time")), formatCost(q.value("setup1").toDouble()));
      XTreeWidgetItem *setupCost = new XTreeWidgetItem(_roughCut, setup, 0, QVariant(tr("Setup Cost")), formatCost(q.value("setupcost1").toDouble()));
      XTreeWidgetItem *run       = new XTreeWidgetItem(_roughCut, setupCost,  0, QVariant(tr("Run Time")), formatCost(q.value("run1").toDouble()));
      XTreeWidgetItem *runCost   = new XTreeWidgetItem(_roughCut, run, 0, QVariant(tr("Run Cost")), formatCost(q.value("runcost1").toDouble()));
                       
      for (int column = 2; column < columns; column++)
      {
        setup->setText(column, formatCost(q.value(QString("setup%1").arg(column)).toDouble()));
        setupCost->setText(column, formatCost(q.value(QString("setupcost%1").arg(column)).toDouble()));
        run->setText(column, formatCost(q.value(QString("run%1").arg(column)).toDouble()));
        runCost->setText(column, formatCost(q.value(QString("runcost%1").arg(column)).toDouble()));
      }
    }
  }
}
