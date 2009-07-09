/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedPlannedREByPlannerCode.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"

dspTimePhasedPlannedREByPlannerCode::dspTimePhasedPlannedREByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCost);
  _costsGroupInt->addButton(_useActualCost);

  QButtonGroup* _salesPriceGroupInt = new QButtonGroup(this);
  _salesPriceGroupInt->addButton(_useListPrice);
  _salesPriceGroupInt->addButton(_useAveragePrice);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _plannedRE->addColumn("", 80, Qt::AlignRight);
}

dspTimePhasedPlannedREByPlannerCode::~dspTimePhasedPlannedREByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTimePhasedPlannedREByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedPlannedREByPlannerCode::sPrint()
{
  if(_useAveragePrice->isChecked() && !(_startEvalDate->isValid() && _endEvalDate->isValid()))
  {
    QMessageBox::information(this, tr("Average Price Requires Dates"),
                                   tr("The Average Price option requires that you specify a valid\n"
                                      "date range to evaluate the average price."));
    return;
  }

  // TODO: Why is this so different from sFillList()?
  ParameterList params;

  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_useStandardCost->isChecked())
    params.append("standardCost");
  else if (_useActualCost->isChecked())
    params.append("actualCost");

  if (_useListPrice->isChecked())
    params.append("listPrice");
  else if (_useAveragePrice->isChecked())
  {
    params.append("averagePrice");
    params.append("startEvalDate", _startEvalDate->date());
    params.append("endEvalDate", _endEvalDate->date());
  }

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  if(selected.isEmpty())
  {
    QMessageBox::information(this, tr("No Periods Selected"),
                                   tr("You must select at least one Period."));
    return;
  }

  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());
  params.append("period_id_list", periodList);

  orReport report("TimePhasedPlannedRevenueExpensesByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspTimePhasedPlannedREByPlannerCode::sFillList()
{
  if(_useAveragePrice->isChecked() && !(_startEvalDate->isValid() && _endEvalDate->isValid()))
  {
    QMessageBox::information(this, tr("Average Price Requires Dates"),
                                   tr("The Average Price option requires that you specify a valid\n"
                                      "date range to evaluate the average price."));
    return;
  }

  _plannedRE->clear();
  _plannedRE->setColumnCount(1);

  QString       sql("SELECT ");

  bool show    = FALSE;
  int  columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  if(selected.isEmpty())
  {
    QMessageBox::information(this, tr("No Periods Selected"),
                                   tr("You must select at least one Period."));
    return;
  }

  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    if (show)
      sql += ",";
    else
      show = TRUE;

    sql += QString(" SUM(plannedCost(plancode_id, warehous_id, '%1', %2)) AS cost%3,")
	   .arg((_useStandardCost->isChecked()) ? 'S' : 'A')
	   .arg(cursor->id())
	   .arg(columns);

    if (_useListPrice->isChecked())
      sql += QString(" SUM(plannedRevenue(plancode_id, warehous_id, 'L', %1)) AS revenue%2 ")
	     .arg(cursor->id())
	     .arg(columns++);
    else
      sql += QString(" SUM(plannedRevenue(plancode_id, warehous_id, 'A', %1, date('%2'), date('%3'))) AS revenue%4 ")
	     .arg(cursor->id())
	     .arg(_startEvalDate->date().toString())    // NOT locale format
	     .arg(_endEvalDate->date().toString())      // NOT locale format
	     .arg(columns++);

    _plannedRE->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  if (show)
  {
    sql += " FROM plancode, warehous";

    if (_warehouse->isSelected())
      sql += " WHERE ( (warehous_id=:warehous_id)";

    if (_plannerCode->isSelected())
    {
      if (_warehouse->isSelected())
        sql += " AND (plancode_id=:plancode_id)";
      else
        sql += " WHERE ( (plancode_id=:plancode_id)";
    }
    else if (_plannerCode->isPattern())
    {
      if (_warehouse->isSelected())
        sql += " AND (plancode_code ~ :plancode_pattern)";
      else
        sql += " WHERE ( (plancode_code ~ :plancode_pattern)";
    }

    if ( (_warehouse->isSelected()) || (!_plannerCode->isAll()) )
      sql += ");";
    else
      sql += ";";

    q.prepare(sql);
    _warehouse->bindValue(q);
    _plannerCode->bindValue(q);
    q.exec();
    if (q.first())
    {
      XTreeWidgetItem *cost    = new XTreeWidgetItem( _plannedRE, 0, QVariant(tr("Cost")),
                                                  formatMoney(q.value("cost1").toDouble()) );

      XTreeWidgetItem *revenue = new XTreeWidgetItem( _plannedRE, cost, 0, QVariant(tr("Revenue")),
                                                  formatMoney(q.value("revenue1").toDouble()) );

      XTreeWidgetItem *profit  = new XTreeWidgetItem( _plannedRE, revenue,  0, QVariant(tr("Gross Profit")),
                                                  formatMoney(q.value("revenue1").toDouble() - q.value("cost1").toDouble() ) );
                       
      for (int bucketCounter = 1; bucketCounter < columns; bucketCounter++)
      {
        cost->setText(bucketCounter, formatMoney(q.value(QString("cost%1").arg(bucketCounter)).toDouble()));
        revenue->setText(bucketCounter, formatMoney(q.value(QString("revenue%1").arg(bucketCounter)).toDouble()));

        profit->setText( bucketCounter,
                         formatMoney( q.value(QString("revenue%1").arg(bucketCounter)).toDouble() -
                                      q.value(QString("cost%1").arg(bucketCounter)).toDouble() ) );
      }
    }
  }
}
