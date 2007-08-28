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

#include "dspTimePhasedBookingsByProductCategory.h"

#include <qvariant.h>
#include <qworkspace.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <parameter.h>
#include <datecluster.h>
#include <q3valuevector.h>
#include <openreports.h>
#include "OpenMFGGUIClient.h"
#include "dspBookingsByProductCategory.h"
#include "rptTimePhasedBookingsByProductCategory.h"
#include "submitReport.h"

/*
 *  Constructs a dspTimePhasedBookingsByProductCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedBookingsByProductCategory::dspTimePhasedBookingsByProductCategory(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_soitem, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*,int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_calculate, SIGNAL(clicked()), this, SLOT(sFillList()));
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

//Added by qt3to4:
#include <Q3PopupMenu>

void dspTimePhasedBookingsByProductCategory::init()
{
  statusBar()->hide();

  _productCategory->setType(ProductCategory);

  _soitem->addColumn(tr("Prod. Cat."), _itemColumn, Qt::AlignLeft   );
  _soitem->addColumn(tr("Whs."),       _whsColumn,  Qt::AlignCenter );
  _soitem->addColumn(tr("UOM"),        _uomColumn,  Qt::AlignLeft   );
}

void dspTimePhasedBookingsByProductCategory::sPrint()
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
  else if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else
    params.append("altCapacityUnits");

  rptTimePhasedBookingsByProductCategory newdlg(this, "", TRUE);
  newdlg.set(params);
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

void dspTimePhasedBookingsByProductCategory::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected, int pColumn)
{
  int menuItem;

  _column = pColumn;

  if (((XListViewItem *)pSelected)->id() != -1)
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
  while (_soitem->columns() > 3)
    _soitem->removeColumn(3);

  _columnDates.clear();

  QString sql("SELECT prodcat_id, warehous_id, prodcat_code, warehous_code");

  if (_salesDollars->isChecked())
    sql += ", TEXT('$') AS uom";
  
  else if (_inventoryUnits->isChecked())
    sql += ", item_invuom AS uom";

  else if (_capacityUnits->isChecked())
    sql += ", item_capuom AS uom";

  else if (_altCapacityUnits->isChecked())
    sql += ", item_altcapuom AS uom";

  int columns = 1;
  XListViewItem *cursor = _periods->firstChild();
  if (cursor != 0)
  {
    do
    {
      if (_periods->isSelected(cursor))
      {
        if (_salesDollars->isChecked())
          sql += QString(", SUM(bookingsByItemValue(itemsite_id, %2)) AS bucket%1")
                 .arg(columns++)
                 .arg(cursor->id());
  
        else if (_inventoryUnits->isChecked())
          sql += QString(", SUM(bookingsByItemQty(itemsite_id, %2)) AS bucket%1")
                 .arg(columns++)
                 .arg(cursor->id());
  
        else if (_capacityUnits->isChecked())
          sql += QString(", SUM(bookingsByItemQty(itemsite_id, %2) * item_capinvrat) AS bucket%1")
                 .arg(columns++)
                 .arg(cursor->id());
  
        else if (_altCapacityUnits->isChecked())
          sql += QString(", SUM(bookingsByItemQty(itemsite_id, %2) * item_altcapinvrat) AS bucket%1")
                 .arg(columns++)
                 .arg(cursor->id());
  
        _soitem->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _qtyColumn, Qt::AlignRight);

        _columnDates.append(DatePair(((PeriodListViewItem *)cursor)->startDate(), ((PeriodListViewItem *)cursor)->endDate()));
      }
    }
    while ((cursor = cursor->nextSibling()) != 0);
  }

  sql += " FROM itemsite, item, warehous, prodcat "
         "WHERE ( (itemsite_item_id=item_id)"
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
  if (q.first())
  {
    Q3ValueVector<Numeric> totals(columns);;

    do
    {
      XListViewItem *item = new XListViewItem( _soitem, _soitem->lastItem(),
                                               q.value("prodcat_id").toInt(), q.value("warehous_id").toInt(),
                                               q.value("prodcat_code"), q.value("warehous_code"),
                                               q.value("uom") );

      for (int column = 1; column < columns; column++)
      {
        QString bucketName = QString("bucket%1").arg(column);
        totals[column] += q.value(bucketName).toDouble();

        if ( (_inventoryUnits->isChecked()) || (_capacityUnits->isChecked()) || (_altCapacityUnits->isChecked()) )
          item->setText((column + 2), formatQty(q.value(bucketName).toDouble()));
        else if (_salesDollars->isChecked())
          item->setText((column + 2), formatMoney(q.value(bucketName).toDouble()));
      }
    }
    while (q.next());

    XListViewItem *total = new XListViewItem(_soitem, _soitem->lastItem(), -1, QVariant(tr("Totals:")));
    for (int column = 1; column < columns; column++)
    {
      if ( (_inventoryUnits->isChecked()) || (_capacityUnits->isChecked()) || (_altCapacityUnits->isChecked()) )
        total->setText((column + 2), formatQty(totals[column].toDouble()));
      else if (_salesDollars->isChecked())
        total->setText((column + 2), formatMoney(totals[column].toDouble()));
    }
  }
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

  XListViewItem *cursor = _periods->firstChild();
  QList<QVariant> periodList;
  while (cursor)
  {
    if (cursor->isSelected())
      periodList.append(cursor->id());

    cursor = cursor->nextSibling();
  }
  params.append("period_id_list", periodList);

  return params;
}

