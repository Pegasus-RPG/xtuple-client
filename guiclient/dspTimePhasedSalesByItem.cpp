/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspTimePhasedSalesByItem.h"

#include <QVariant>
#include <datecluster.h>
#include <QWorkspace>
#include <QMessageBox>
#include <QStatusBar>
#include <q3valuevector.h>
#include <parameter.h>
#include <openreports.h>
#include "dspSalesHistoryByItem.h"
#include "rptTimePhasedSalesByItem.h"
#include "OpenMFGGUIClient.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedSalesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedSalesByItem::dspTimePhasedSalesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

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
    
    init();
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

//Added by qt3to4:
#include <QMenu>

void dspTimePhasedSalesByItem::init()
{
  statusBar()->hide();

  _productCategory->setType(ProductCategory);
  
  _sohist->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _sohist->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Whs."),        _whsColumn,  Qt::AlignCenter );

  _salesDollars->setEnabled(_privleges->check("ViewCustomerPrices"));
}

void dspTimePhasedSalesByItem::sPrint()
{
  ParameterList params;
  params.append("print");
  _periods->getSelected(params);
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);

  if (_salesDollars->isChecked())
    params.append("salesDollars");
  else if (_inventoryUnits->isChecked())
    params.append("inventoryUnits");

#if 0
  else if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else
    params.append("altCapacityUnits");
#endif

  rptTimePhasedSalesByItem newdlg(this, "", TRUE);
  newdlg.set(params);
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
  if (!_privleges->check("ViewSalesHistory"))
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
    sql += ", item_invuom AS uom, warehous_code";

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    if (_salesDollars->isChecked())
      sql += QString(", shipmentsByItemValue(itemsite_id, %2) AS bucket%1")
	     .arg(columns++)
	     .arg(cursor->id());

    else if (_inventoryUnits->isChecked())
      sql += QString(", shipmentsByItemQty(itemsite_id, %2) AS bucket%1")
	     .arg(columns++)
	     .arg(cursor->id());

    _sohist->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM itemsite, item, warehous "
         "WHERE ( (itemsite_item_id=item_id)"
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
  if (q.first())
  {
    Q3ValueVector<Numeric> totals(columns);;
    XTreeWidgetItem *last = 0;

    do
    {
      last = new XTreeWidgetItem( _sohist, last, q.value("itemsite_id").toInt(),
				 q.value("item_number"), q.value("uom"),
				 q.value("warehous_code") );

      for (int column = 1; column < columns; column++)
      {
        QString bucketName = QString("bucket%1").arg(column);
        totals[column] += q.value(bucketName).toDouble();

        if (_salesDollars->isChecked())
          last->setText((column + 2), formatMoney(q.value(bucketName).toDouble()));
	else if (_inventoryUnits->isChecked())
          last->setText((column + 2), formatQty(q.value(bucketName).toDouble()));
      }
    }
    while (q.next());

//  Add the totals row
    XTreeWidgetItem *total = new XTreeWidgetItem(_sohist, last, -1, QVariant(tr("Totals:")));
    for (int column = 1; column < columns; column++)
    {
      if (_salesDollars->isChecked())
        total->setText((column + 2), formatMoney(totals[column].toDouble()));
      else if (_inventoryUnits->isChecked())
        total->setText((column + 2), formatQty(totals[column].toDouble()));
    }
  }
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

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  QList<QVariant> periodList;
  for (int i = 0; i < selected.size(); i++)
    periodList.append(((XTreeWidgetItem*)selected[i])->id());

  params.append("period_id_list", periodList);

  return params;
}
