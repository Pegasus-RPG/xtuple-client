/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspRoughCutByWorkCenter.h"

#include <QMenu>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

dspRoughCutByWorkCenter::dspRoughCutByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _workCenterGroupInt = new QButtonGroup(this);
  _workCenterGroupInt->addButton(_allWorkCenters);
  _workCenterGroupInt->addButton(_selectedWorkCenter);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));
  connect(_roughCut, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _roughCut->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _roughCut->addColumn(tr("Work Center"),  -1,          Qt::AlignLeft,   true,  "wrkcnt_code"   );
  _roughCut->addColumn(tr("Total Setup"),  _timeColumn, Qt::AlignRight,  true,  "setuptime"  );
  _roughCut->addColumn(tr("Setup $"),      _costColumn, Qt::AlignRight,  true,  "setupcost"  );
  _roughCut->addColumn(tr("Total Run"),    _timeColumn, Qt::AlignRight,  true,  "runtime"  );
  _roughCut->addColumn(tr("Run $"),        _costColumn, Qt::AlignRight,  true,  "runcost"  );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
}

dspRoughCutByWorkCenter::~dspRoughCutByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspRoughCutByWorkCenter::languageChange()
{
  retranslateUi(this);
}

bool dspRoughCutByWorkCenter::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_selectedWorkCenter->isChecked())
    params.append("wrkcnt_id", _wrkcnt->id());

  return true;
}

void dspRoughCutByWorkCenter::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("RoughCutCapacityPlanByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspRoughCutByWorkCenter::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspRoughCutByWorkCenter::sQuery()
{
  ParameterList params;
  setParams(params);

  QString sql( "SELECT wrkcnt_id, warehous_code, wrkcnt_code,"
               "       SUM(planoper_sutime) AS setuptime,"
               "       (SUM(planoper_sutime) * wrkcnt_setuprate / 60.0) AS setupcost,"
               "       SUM(planoper_rntime) AS runtime,"
               "       (SUM(planoper_rntime) * wrkcnt_runrate / 60.0) AS runcost,"
               "       '1' AS setuptime_xtnumericrole,"
               "       '1' AS runtime_xtnumericrole,"
               "       'cost' AS setupcost_xtnumericrole,"
               "       'cost' AS runcost_xtnumericrole "
               "FROM planoper, planord, wrkcnt, warehous "
               "WHERE ( (planoper_planord_id=planord_id)"
               " AND (planoper_wrkcnt_id=wrkcnt_id)"
               " AND (wrkcnt_warehous_id=warehous_id)"
               " AND (planord_startdate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
	       "<? if exists(\"wrkcnt_id\") ?>"
	       " AND (wrkcnt_id=<? value(\"wrkcnt_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       ") "
	       "GROUP BY wrkcnt_id, warehous_code, wrkcnt_code,"
	       "         wrkcnt_setuprate, wrkcnt_runrate "
	       "ORDER BY warehous_code, wrkcnt_code;" );

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _roughCut->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
