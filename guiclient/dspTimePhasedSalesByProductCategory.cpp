/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedSalesByProductCategory.h"

#include <QVariant>
#include <QWorkspace>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <q3valuevector.h>
#include <datecluster.h>
#include <parameter.h>
#include <openreports.h>
#include <metasql.h>
#include "dspSalesHistoryByParameterList.h"
#include "guiclient.h"
#include "submitReport.h"
#include "mqlutil.h"

/*
 *  Constructs a dspTimePhasedSalesByProductCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedSalesByProductCategory::dspTimePhasedSalesByProductCategory(QWidget* parent, const char* name, Qt::WFlags fl)
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
  
  _sohist->addColumn(tr("Prod. Cat."), _itemColumn, Qt::AlignLeft,   true,  "prodcat_code"   );
  _sohist->addColumn(tr("UOM"),        _uomColumn,  Qt::AlignCenter, true,  "uom" );
  _sohist->addColumn(tr("Site"),       _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );

  _salesDollars->setEnabled(_privileges->check("ViewCustomerPrices"));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedSalesByProductCategory::~dspTimePhasedSalesByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedSalesByProductCategory::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedSalesByProductCategory::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    orReport report("TimePhasedSalesHistoryByProductCategory", buildParameters());
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

void dspTimePhasedSalesByProductCategory::sViewShipments()
{
  if (_column > 2)
  {
    ParameterList params;
    params.append("prodcat_id", _sohist->id());
    params.append("warehous_id", _sohist->altId());
    params.append("startDate", _columnDates[_column - 3].startDate);
    params.append("endDate", _columnDates[_column - 3].endDate);
    params.append("run");

    dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedSalesByProductCategory::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;

  _column = pColumn;

  intMenuItem = menuThis->insertItem(tr("View Sales Detail..."), this, SLOT(sViewShipments()), 0);
  if (!_privileges->check("ViewSalesHistory"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void dspTimePhasedSalesByProductCategory::sCalculate()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _sohist->clear();
  _sohist->setColumnCount(3);

  _columnDates.clear();

  ParameterList params;
  if (! setParams(params))
    return;

  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket_%1").arg(cursor->id());

    _sohist->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  MetaSQLQuery mql = mqlLoad("timePhasedSales", "detail");
  q = mql.toQuery(params);
  _sohist->populate(q, true);
}


void dspTimePhasedSalesByProductCategory::sSubmit()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params(buildParameters());
    params.append("report_name", "TimePhasedSalesHistoryByProductCategory");
    
    submitReport newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.check() == cNoReportDefinition)
      QMessageBox::critical( this, tr("Report Definition Not Found"),
                             tr( "The report defintions for this report, \"TimePhasedSalesHistoryByProductCategory\" cannot be found.\n"
                                 "Please contact your Systems Administrator and report this issue." ) );
    else
      newdlg.exec();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}

ParameterList dspTimePhasedSalesByProductCategory::buildParameters()
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

  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());

  params.append("period_id_list", periodList);

  return params;
}

bool dspTimePhasedSalesByProductCategory::setParams(ParameterList & params)
{
  params.append("byProdcat");
  params.append("period_list",_periods->periodList());

  if (_salesDollars->isChecked())
    params.append("salesDollars");
  else if (_inventoryUnits->isChecked())
    params.append("inventoryUnits");
  else if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else if (_altCapacityUnits->isChecked())
    params.append("altCapacityUnits");

  if ((_productCategory->isSelected()) || (_productCategory->isPattern()))
    _productCategory->appendValue(params);

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  return true;
}
