/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedBookingsByProductCategory.h"

#include <QVariant>
#include <QWorkspace>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <q3valuevector.h>
#include <parameter.h>
#include <datecluster.h>
#include <openreports.h>
#include "guiclient.h"
#include "dspBookingsByProductCategory.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedBookingsByProductCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedBookingsByProductCategory::dspTimePhasedBookingsByProductCategory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calculate, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  
  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
    
  _productCategory->setType(ParameterGroup::ProductCategory);

  _soitem->addColumn(tr("Prod. Cat."), _itemColumn, Qt::AlignLeft,   true,  "prodcat_code"   );
  _soitem->addColumn(tr("Site"),       _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _soitem->addColumn(tr("UOM"),        _uomColumn,  Qt::AlignLeft,   true,  "uom"   );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedBookingsByProductCategory::~dspTimePhasedBookingsByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedBookingsByProductCategory::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedBookingsByProductCategory::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedBookingsByProductCategory", buildParameters());
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

void dspTimePhasedBookingsByProductCategory::sViewBookings()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("prodcat_id", _soitem->id());
    params.append("warehous_id", _soitem->altId());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspBookingsByProductCategory *newdlg = new dspBookingsByProductCategory();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedBookingsByProductCategory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  int menuItem;

  _column = pColumn;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    menuItem = pMenu->insertItem(tr("View Bookings..."), this, SLOT(sViewBookings()), 0);
}

void dspTimePhasedBookingsByProductCategory::sFillList()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _soitem->clear();
  _soitem->setColumnCount(3);

  _columnDates.clear();

  QString sql("SELECT prodcat_id, warehous_id, prodcat_code, warehous_code");

  if (_salesDollars->isChecked())
    sql += ", TEXT('$') AS uom";
  
  else if (_inventoryUnits->isChecked())
    sql += ", uom_name AS uom";

  else if (_capacityUnits->isChecked())
    sql += ", itemcapuom(item_id) AS uom";

  else if (_altCapacityUnits->isChecked())
    sql += ", itemaltcapuom(item_id) AS uom";

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);

    if (_salesDollars->isChecked())
      sql += QString(", SUM(bookingsByItemValue(itemsite_id, %1)) AS %2,"
                     "  'curr' AS %3_xtnumericrole, 0 AS %4_xttotalrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname)
	     .arg(bucketname);

    else if (_inventoryUnits->isChecked())
      sql += QString(", SUM(bookingsByItemQty(itemsite_id, %1)) AS %2,"
                     "  'qty' AS %3_xtnumericrole, 0 AS %4_xttotalrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname)
	     .arg(bucketname);

    else if (_capacityUnits->isChecked())
      sql += QString(", SUM(bookingsByItemQty(itemsite_id, %1) * itemcapinvrat(item_id)) AS %2,"
                     "  'qty' AS %3_xtnumericrole, 0 AS %4_xttotalrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname)
	     .arg(bucketname);

    else if (_altCapacityUnits->isChecked())
      sql += QString(", SUM(bookingsByItemQty(itemsite_id, %1) * itemaltcapinvrat(item_id)) AS %2,"
                     "  'qty' AS %3_xtnumericrole, 0 AS %4_xttotalrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname)
	     .arg(bucketname);

    _soitem->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);

    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, uom, warehous, prodcat "
         "WHERE ( (itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (item_prodcat_id=prodcat_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id) ";

  if (_productCategory->isSelected())
    sql += "AND (prodcat_id=:prodcat_id) ";
  else if (_productCategory->isPattern())
    sql += "AND (prodcat_code ~ :prodcat_pattern) ";

  sql += ") "
         "GROUP BY prodcat_id, warehous_id, prodcat_code, uom, warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  q.exec();
  _soitem->populate(q, true);
}

void dspTimePhasedBookingsByProductCategory::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedBookingsByProductCategory");
    
    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedBookingsByProductCategory\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedBookingsByProductCategory::buildParameters()
{
  ParameterList params;

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);

  if(_inventoryUnits->isChecked())
    params.append("inventoryUnits");
  else if(_capacityUnits->isChecked())
    params.append("capacityUnits");
  else if(_altCapacityUnits->isChecked())
    params.append("altCapacityUnits");
  else if(_salesDollars->isChecked())
    params.append("salesDollars");

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());

  params.append("period_id_list", periodList);

  return params;
}
