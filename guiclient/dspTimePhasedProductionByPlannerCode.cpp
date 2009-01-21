/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedProductionByPlannerCode.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <datecluster.h>
#include "dspInventoryHistoryByParameterList.h"
#include "guiclient.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedProductionByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedProductionByPlannerCode::dspTimePhasedProductionByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  QButtonGroup* _unitsGroupInt = new QButtonGroup(this);
  _unitsGroupInt->addButton(_inventoryUnits);
  _unitsGroupInt->addButton(_capacityUnits);
  _unitsGroupInt->addButton(_altCapacityUnits);

  // signals and slots connections
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_production, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));

//  statusBar()->hide();

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _production->addColumn(tr("Planner Code"), _itemColumn, Qt::AlignLeft,   true,  "plancode_code"   );
  _production->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _production->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignLeft,   true,  "uom"   );

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedProductionByPlannerCode::~dspTimePhasedProductionByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedProductionByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedProductionByPlannerCode::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedProductionByPlannerCode", buildParameters());
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

void dspTimePhasedProductionByPlannerCode::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedProductionByPlannerCode");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedProductionByPlannerCode\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedProductionByPlannerCode::buildParameters()
{
  ParameterList params;

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());
  params.append("period_id_list", periodList);

  if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else if(_altCapacityUnits->isChecked())
    params.append("altCapacityUnits");
  else if(_inventoryUnits->isChecked())
    params.append("inventoryUnits");

  if(_showInactive->isChecked())
    params.append("showInactive");

  return params;
}

void dspTimePhasedProductionByPlannerCode::sViewTransactions()
{
  ParameterList params;
  params.append("plancode_id", _production->id());
  params.append("warehous_id", _production->altId());
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("transtype", "R");
  params.append("run");

  dspInventoryHistoryByParameterList *newdlg = new dspInventoryHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedProductionByPlannerCode::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 2)
  {
    menuItem = menu->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
    if (!_privileges->check("ViewInventoryHistory"))
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedProductionByPlannerCode::sCalculate()
{
  _columnDates.clear();
  _production->setColumnCount(3);

  QString sql("SELECT plancode_id, warehous_id, plancode_code, warehous_code, ");

  if (_inventoryUnits->isChecked())
    sql += "uom_name AS uom";

  else if (_capacityUnits->isChecked())
    sql += "itemcapuom(item_id) AS uom";

  else if (_altCapacityUnits->isChecked())
    sql += "itemaltcapuom(item_id) AS uom";

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    if (_inventoryUnits->isChecked())
      sql += QString(", SUM(summProd(itemsite_id, %1)) AS %2,"
                     "  'qty' AS %3_xtnumericrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname);

    else if (_capacityUnits->isChecked())
      sql += QString(", SUM(summProd(itemsite_id, %1) * itemcapinvrat(item_id)) AS %2,"
                     "  'qty' AS %3_xtnumericrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname);

    else if (_altCapacityUnits->isChecked())
      sql += QString(", SUM(summProd(itemsite_id, %1) * itemaltcapinvrat(item_id)) AS %2,"
                     "  'qty' AS %3_xtnumericrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname);

    _production->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, uom, warehous, plancode "
         "WHERE ((itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (itemsite_plancode_id=plancode_id)";

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_plannerCode->isSelected())
    sql += " AND (plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (plancode_code ~ :plancode_pattern)";

  sql += " ) "
         "GROUP BY plancode_id, warehous_id, plancode_code, warehous_code, uom;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _production->populate(q, TRUE);
}

