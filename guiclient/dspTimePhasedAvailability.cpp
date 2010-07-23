/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedAvailability.h"

#include <QVariant>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <datecluster.h>
#include <metasql.h>
#include <openreports.h>

#include "guiclient.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "submitReport.h"
#include "mqlutil.h"

dspTimePhasedAvailability::dspTimePhasedAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

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

dspTimePhasedAvailability::~dspTimePhasedAvailability()
{
  // no need to delete child widgets, Qt does it all for us
}

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

  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
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
  QAction *menuItem;

  _column = pColumn;
  if (_column > 2)
  {
    menuItem = pMenu->addAction(tr("View Availability Detail..."), this, SLOT(sViewAvailability()));
    menuItem = pMenu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
    menuItem = pMenu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  
    if (((XTreeWidgetItem *)pSelected)->altId() == 1)
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("Create W/O..."), this, SLOT(sCreateWO()));
    }
    else if (((XTreeWidgetItem *)pSelected)->altId() == 2)
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
      menuItem = pMenu->addAction(tr("Create P/O..."), this, SLOT(sCreatePO()));
    }
  }
}

void dspTimePhasedAvailability::sCalculate()
{
  _columnDates.clear();
  _availability->clear();
  _availability->setColumnCount(3);
  ParameterList params;

  if (! setParams(params))
    return;

  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket_%1").arg(cursor->id());
    _availability->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  MetaSQLQuery mql = mqlLoad("timePhasedAvailability", "detail");
  q = mql.toQuery(params);

  if (q.first())
    _availability->populate(q, true);
}

bool dspTimePhasedAvailability::setParams(ParameterList & params)
{
  params.append("item_list", _periods->periodList());
  params.append("period_list", _periods->periodList());
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  if (_plannerCode->isSelected())
    params.append("plancode_id", _plannerCode->id());
  else if (_plannerCode->isPattern())
  {
    QString pattern = _plannerCode->pattern();
    if (pattern.length() == 0)
      return false;

    params.append("plancode_pattern", _plannerCode->pattern());
  }
  return true;
}
