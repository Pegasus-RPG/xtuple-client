/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPlannedRevenueExpensesByPlannerCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>

dspPlannedRevenueExpensesByPlannerCode::dspPlannedRevenueExpensesByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _costGroupInt = new QButtonGroup(this);
  _costGroupInt->addButton(_useStandardCost);
  _costGroupInt->addButton(_useActualCost);

  QButtonGroup* _salesPriceGroupInt = new QButtonGroup(this);
  _salesPriceGroupInt->addButton(_useListPrice);
  _salesPriceGroupInt->addButton(_useAveragePrice);

  connect(_planord, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _planord->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,  true, "ordernum");
  _planord->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter,true, "ordtype");
  _planord->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter,true, "warehous_code");
  _planord->addColumn(tr("From Site"),   _whsColumn,   Qt::AlignCenter,true, "supply_warehous_code");
  _planord->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _planord->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "item_descrip");
  _planord->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "planord_duedate");
  _planord->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight, true, "planord_qty");
  _planord->addColumn(tr("Firm"),        _ynColumn,    Qt::AlignCenter,true, "planord_firm");
  _planord->addColumn(tr("Cost"),        _moneyColumn, Qt::AlignRight, true, "plocost");
  _planord->addColumn(tr("Revenue"),     _moneyColumn, Qt::AlignRight, true, "plorevenue");
  _planord->addColumn(tr("Gr. Profit"),  _moneyColumn, Qt::AlignRight, true, "profit");

  _startDate->setAllowNullDate(true);
  _startDate->setNullString(tr("Earliest"));
  _startDate->setNullDate(omfgThis->startOfTime());
  _endDate->setAllowNullDate(true);
  _endDate->setNullString(tr("Latest"));
  _endDate->setNullDate(omfgThis->endOfTime());

  _startEvalDate->setAllowNullDate(true);
  _startEvalDate->setNullString(tr("Earliest"));
  _startEvalDate->setNullDate(omfgThis->startOfTime());
  _endEvalDate->setAllowNullDate(true);
  _endEvalDate->setNullString(tr("Latest"));
  _endEvalDate->setNullDate(omfgThis->endOfTime());
}

dspPlannedRevenueExpensesByPlannerCode::~dspPlannedRevenueExpensesByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPlannedRevenueExpensesByPlannerCode::languageChange()
{
  retranslateUi(this);
}

bool dspPlannedRevenueExpensesByPlannerCode::setParams(ParameterList &params)
{
  params.append("soldOnly");
  params.append("startDate", _startDate->date());
  params.append("endDate", _endDate->date());
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_useStandardCost->isChecked())
    params.append("useStandardCost");

  if (_useActualCost->isChecked())
    params.append("useActualCost");

  if (_useListPrice->isChecked())
    params.append("useListPrice");

  if (_useAveragePrice->isChecked())
  {
    params.append("useAveragePrice");
    params.append("startEvalDate", _startEvalDate->date());
    params.append("endEvalDate", _endEvalDate->date());
  }

  return true;
}

void dspPlannedRevenueExpensesByPlannerCode::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("PlannedRevenueExpensesByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPlannedRevenueExpensesByPlannerCode::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspPlannedRevenueExpensesByPlannerCode::sFillList()
{
  _planord->clear();

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql = mqlLoad("schedule", "plannedorders");
  q = mql.toQuery(params);
  _planord->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // set color if profit is negative
//  XTreeWidgetItem *last = _planord->topLevelItem(_planord->topLevelItemCount() - 1);
//  if (last && last->data(9, Qt::UserRole).toMap().value("raw").toDouble() < 0)
//    last->setTextColor(9, "red");
}
