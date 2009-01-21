/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedProductionByItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <datecluster.h>
#include "dspInventoryHistoryByItem.h"
#include "guiclient.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedProductionByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedProductionByItem::dspTimePhasedProductionByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_production, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _production->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _production->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _production->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "uom_name"   );

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedProductionByItem::~dspTimePhasedProductionByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedProductionByItem::languageChange()
{
    retranslateUi(this);
}

void dspTimePhasedProductionByItem::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedProductionByItem", buildParameters());
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

void dspTimePhasedProductionByItem::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedProductionByItem");

    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedProductionByItem\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedProductionByItem::buildParameters()
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

void dspTimePhasedProductionByItem::sViewTransactions()
{
  ParameterList params;
  params.append("itemsite_id", _production->id());
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("transtype", "R");
  params.append("run");
  _warehouse->appendValue(params);

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedProductionByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 1)
  {
    menuItem = menu->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
    if (!_privileges->check("ViewInventoryHistory"))
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedProductionByItem::sCalculate()
{
  _columnDates.clear();
  _production->setColumnCount(3);

  QString sql("SELECT itemsite_id, item_number, warehous_code, uom_name");

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", summProd(itemsite_id, %1) AS %2,"
                   "  'qty' AS %3_xtnumericrole ")
	   .arg(cursor->id())
	   .arg(bucketname)
	   .arg(bucketname);

    _production->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, uom, warehous "
         "WHERE ((itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_plannerCode->isSelected())
    sql += "AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += "AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern))) ";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _production->populate(q);
}

