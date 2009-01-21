/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspExpediteExceptionsByPlannerCode.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

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
  _exception->addColumn(tr("Site"),           _whsColumn,  Qt::AlignCenter,true, "warehous_code");
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
  QString sql( "SELECT planord_id AS order_id, 1 AS order_code,"
               "       CASE WHEN (planord_type='W') THEN ('PW/O-' || formatPloNumber(planord_id))"
               "            WHEN (planord_type='P') THEN ('PP/O-' || formatPloNumber(planord_id))"
               "            ELSE TEXT(planord_number)"
               "       END AS order_number,"
               "       warehous_code, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       CASE WHEN (planord_type='W') THEN formatDate(planord_startdate)"
               "            ELSE formatDate(planord_duedate)"
               "       END AS f_keydate,"
               "       CASE WHEN (planord_type='W') THEN planord_startdate"
               "            ELSE planord_duedate"
               "       END AS keydate, :releaseOrder AS exception "
               "FROM planord, itemsite, item, warehous "
               "WHERE ( (planord_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND ( ( (planord_type='W') AND (planord_startdate <= (CURRENT_DATE + :days)) )"
               "    OR ( (planord_type='P') AND (planord_duedate <= (CURRENT_DATE + :days)) ) ) " );

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += " ) ";

  sql += "UNION SELECT wo_id AS order_id, 2 AS order_code, ('W/O-' || formatWoNumber(wo_id)) AS order_number,"
         "             warehous_code, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
         "             formatDate(wo_startdate) AS f_keydate, wo_startdate AS keydate, :startProduction AS exception "
         "FROM wo, itemsite, item, warehous "
         "WHERE ( (wo_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (wo_status IN ('O', 'E', 'R'))"
         " AND (wo_startdate <= (CURRENT_DATE + :days))";

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += " ) ";

  sql += "UNION SELECT wo_id AS order_id, 2 AS order_code, ('W/O-' || formatWoNumber(wo_id)) AS order_number,"
         "             warehous_code, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
         "             formatDate(wo_startdate) AS f_keydate, wo_startdate AS keydate, :expediteProduction AS exception "
         "FROM wo, itemsite, item, warehous "
         "WHERE ( (wo_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (wo_status='I')"
         " AND (wo_startdate <= (CURRENT_DATE + :days))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += " ) "
         "ORDER BY keydate;";

  q.prepare(sql);
  q.bindValue(":releaseOrder", tr("Release Order"));
  q.bindValue(":startProduction", tr("Start Production"));
  q.bindValue(":expediteProduction", tr("Expedite Production"));
  q.bindValue(":days", _days->value());
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _exception->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspExpediteExceptionsByPlannerCode::sPrint()
{
  ParameterList params;

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  params.append("lookAheadDays", _days->value());

  orReport report("ExpediteExceptionsByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspExpediteExceptionsByPlannerCode::sPopulateMenu( QMenu *, QTreeWidgetItem * )
{

}
