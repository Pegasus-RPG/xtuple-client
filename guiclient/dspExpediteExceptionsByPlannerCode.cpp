/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspExpediteExceptionsByPlannerCode.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>
#include "mqlutil.h"

#include <parameter.h>
#include <openreports.h>

dspExpediteExceptionsByPlannerCode::dspExpediteExceptionsByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_exception, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _exception->addColumn(tr("Order Type/#"),   _itemColumn, Qt::AlignCenter,true, "order_number");
  _exception->addColumn(tr("To Site"),        _whsColumn,  Qt::AlignCenter,true, "to_warehous");
  _exception->addColumn(tr("From Site"),      _whsColumn,  Qt::AlignCenter,true, "from_warehous");
  _exception->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  _exception->addColumn(tr("Descriptions"),   -1,          Qt::AlignLeft,  true, "item_descrip");
  _exception->addColumn(tr("Start/Due Date"), _itemColumn, Qt::AlignLeft,  true, "keydate");
  _exception->addColumn(tr("Exception"),      120,         Qt::AlignLeft,  true, "exception");
}

dspExpediteExceptionsByPlannerCode::~dspExpediteExceptionsByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspExpediteExceptionsByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void dspExpediteExceptionsByPlannerCode::sFillList()
{
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("schedule", "expedite");
  q = mql.toQuery(params);
  _exception->populate(q, true);
}

void dspExpediteExceptionsByPlannerCode::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ExpediteExceptionsByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspExpediteExceptionsByPlannerCode::setParams(ParameterList & params)
{
  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  params.append("releaseOrder", tr("Release Order"));
  params.append("startProduction", tr("Start Production"));
  params.append("expediteProduction", tr("Expedite Production"));
  params.append("days", _days->value());
}

void dspExpediteExceptionsByPlannerCode::sPopulateMenu( QMenu *, QTreeWidgetItem * )
{

}
