/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByItem.h"

#include <QVariant>
#include <QWorkspace>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <q3valuevector.h>
#include <datecluster.h>
#include <parameter.h>
#include <openreports.h>
#include "dspSalesHistoryByItem.h"
#include "guiclient.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedSalesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedSalesByItem::dspTimePhasedSalesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    
  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
  
  _productCategory->setType(ParameterGroup::ProductCategory);
  
  _sohist->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _sohist->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter, true,  "uom" );
  _sohist->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );

  _salesDollars->setEnabled(_privileges->check("ViewCustomerPrices"));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedSalesByItem::~dspTimePhasedSalesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedSalesByItem::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedSalesByItem::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedSalesHistoryByItem", buildParameters());
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

void dspTimePhasedSalesByItem::sViewShipments()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("itemsite_id", _sohist->id());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspSalesHistoryByItem *newdlg = new dspSalesHistoryByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedSalesByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;

  menuItem = menuThis->insertItem(tr("View Sales Detail..."), this, SLOT(sViewShipments()), 0);
  if (!_privileges->check("ViewSalesHistory"))
    menuThis->setItemEnabled(menuItem, FALSE);
}

void dspTimePhasedSalesByItem::sCalculate()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  QString sql("SELECT itemsite_id, item_number");

  _sohist->clear();
  _sohist->setColumnCount(3);

  _columnDates.clear();

  if (_salesDollars->isChecked())
    sql += ", TEXT('$') AS uom, warehous_code";
  
  else if (_inventoryUnits->isChecked())
    sql += ", uom_name AS uom, warehous_code";

  int columns = 1;
  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    if (_salesDollars->isChecked())
      sql += QString(", shipmentsByItemValue(itemsite_id, %1) AS %2,"
                     "  'curr' AS %3_xtnumericrole, 0 AS %4_xttotalrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname)
	     .arg(bucketname);

    else if (_inventoryUnits->isChecked())
      sql += QString(", shipmentsByItemQty(itemsite_id, %1) AS %2,"
                     "  'qty' AS %3_xtnumericrole, 0 AS %4_xttotalrole ")
	     .arg(cursor->id())
	     .arg(bucketname)
	     .arg(bucketname)
	     .arg(bucketname);

    _sohist->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, uom, warehous "
         "WHERE ( (itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_productCategory->isSelected())
    sql += "AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += "AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern))) ";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  q.exec();
  _sohist->populate(q);
}


void dspTimePhasedSalesByItem::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedSalesHistoryByItem");
    
    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedSalesHistoryByItem\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedSalesByItem::buildParameters()
{
  ParameterList params;

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);

  if (_inventoryUnits->isChecked())
    params.append("inventoryUnits");
  else if(_salesDollars->isChecked())
    params.append("salesDollars");

  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());

  params.append("period_id_list", periodList);

  return params;
}
