/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedAvailability.h"

#include <QVariant>
#include <QWorkspace>
//#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <datecluster.h>
#include <openreports.h>
#include "guiclient.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedAvailability as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedAvailability::dspTimePhasedAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
  connect(_availability, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _availability->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _availability->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "uom_name"   );
  _availability->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedAvailability::~dspTimePhasedAvailability()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedAvailability::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedAvailability::sPrint()
{
  if (_calendar->isValid() && (_periods->isPeriodSelected()))
  {
    orReport report("TimePhasedAvailability", buildParameters());
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

void dspTimePhasedAvailability::sSubmit()
{
  if (_calendar->isValid() && (_periods->isPeriodSelected()))
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedAvailability");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedAvailability\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedAvailability::buildParameters()
{
  ParameterList params;

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());

  params.append("period_id_list", periodList);

  return params;
}

void dspTimePhasedAvailability::sViewAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byDate", _columnDates[_column - 3].startDate);
  params.append("run");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byRange");
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("run");

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _availability->id());
  params.append("byRange");
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("run");

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspTimePhasedAvailability::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _availability->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailability::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 2)
  {
    menuItem = pMenu->insertItem(tr("View Availability Detail..."), this, SLOT(sViewAvailability()), 0);
    menuItem = pMenu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
    menuItem = pMenu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  
    if (((XTreeWidgetItem *)pSelected)->altId() == 1)
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("Create W/O..."), this, SLOT(sCreateWO()), 0);
    }
    else if (((XTreeWidgetItem *)pSelected)->altId() == 2)
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
      menuItem = pMenu->insertItem(tr("Create P/O..."), this, SLOT(sCreatePO()), 0);
    }
  }
}

void dspTimePhasedAvailability::sCalculate()
{
  _columnDates.clear();
  _availability->clear();
  _availability->setColumnCount(3);

  QString sql( "SELECT itemsite_id, itemtype,"
               "       item_number, uom_name, warehous_code,"
               "       reorderlevel " );

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", %1,"
                   "  'qty' AS %2_xtnumericrole,"
                   "  CASE WHEN (%3 < reorderlevel) THEN 'error' END AS %4_qtforegroundrole "    )
	   .arg(bucketname)
	   .arg(bucketname)
	   .arg(bucketname)
	   .arg(bucketname);
  }

  sql +=       "FROM ( "
               "SELECT itemsite_id,"
               "       CASE WHEN (item_type IN ('F', 'B', 'C', 'Y', 'R')) THEN 0"
               "            WHEN (item_type IN ('M')) THEN 1"
               "            WHEN (item_type IN ('P', 'O')) THEN 2"
               "            ELSE 0"
               "       END AS itemtype,"
               "       item_number, uom_name, warehous_code,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel ";

  columns = 1;
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", qtyAvailable(itemsite_id, findPeriodStart(%1)) AS %2 " )
	   .arg(cursor->id())
	   .arg(bucketname);

    _availability->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);

    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, warehous, uom "
         "WHERE ((itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += ") ) AS data "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  if (q.first())
  {
    _availability->populate(q, true);
  }
}

