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

#include "dspMPSDetail.h"

#include <Q3PopupMenu>
#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <parameter.h>
#include <datecluster.h>
#include "dspAllocations.h"
#include "dspOrders.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "rptMPSDetail.h"

#define noNeg(x) ((x < 0.0) ? 0.0 : x)

/*
 *  Constructs a dspMPSDetail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspMPSDetail::dspMPSDetail(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_itemsite, SIGNAL(selectionChanged()), this, SLOT(sFillMPSDetail()));
  connect(_mps, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*,int)));
  connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillItemsites()));
  connect(_plannerCode, SIGNAL(updated()), this, SLOT(sFillItemsites()));
  connect(_itemsite, SIGNAL(itemSelected(int)), this, SLOT(sFillMPSDetail()));

  statusBar()->hide();

  _plannerCode->setType(PlannerCode);

  _itemsite->addColumn("Itemtype",         0,           Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"),  -1,          Qt::AlignLeft   );
  _itemsite->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _itemsite->addColumn(tr("Safety Stock"), _qtyColumn,  Qt::AlignRight  );

  _mps->addColumn("", 120, Qt::AlignRight);

  int cid = _metrics->value("DefaultMSCalendar").toInt();
  if(cid > 0)
    _calendar->setId(cid);

  sFillItemsites();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspMPSDetail::~dspMPSDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspMPSDetail::languageChange()
{
    retranslateUi(this);
}

enum SetResponse dspMPSDetail::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _warehouse->setAll();
    sFillItemsites();
    _itemsite->setId(param.toInt());
  }

  return NoError;
}

void dspMPSDetail::sPrint()
{
  ParameterList params;
  params.append("print");
  params.append("itemsite_id", _itemsite->id());
  _periods->getSelected(params);
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  rptMPSDetail newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspMPSDetail::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *, int pColumn)
{
  int           menuItem;
  Q3ListViewItem *cursor = _mps->firstChild();

  _column = pColumn;

  menuItem = pMenu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  while ((cursor != 0) && (cursor->text(0) != tr("Allocations")))
    cursor = cursor->nextSibling();
  if (cursor->text(pColumn).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  while ((cursor != 0) && (cursor->text(0) != tr("Orders")))
    cursor = cursor->nextSibling();
  if (cursor->text(pColumn).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  if (_itemsite->currentItem()->text(0) == "P")
  {
    menuItem = pMenu->insertItem(tr("Issue P/R..."), this, SLOT(sIssuePR()));
    if (!_privleges->check("MaintainPurchaseRequests"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Issue P/O..."), this, SLOT(sIssuePO()));
    if (!_privleges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (_itemsite->currentItem()->text(0) == "M")
  {
    menuItem = pMenu->insertItem(tr("Issue W/O..."), this, SLOT(sIssueWO()));
    if (!_privleges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspMPSDetail::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byRange");
  params.append("startDate", _periods->getSelected(_column)->startDate());
  params.append("endDate", _periods->getSelected(_column)->endDate());
  params.append("run");

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMPSDetail::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byRange");
  params.append("startDate", _periods->getSelected(_column)->startDate());
  params.append("endDate", _periods->getSelected(_column)->endDate());
  params.append("run");

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMPSDetail::sIssuePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillMPSDetail();
}

void dspMPSDetail::sIssuePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspMPSDetail::sIssueWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMPSDetail::sFillItemsites()
{
  QString sql( "SELECT itemsite_id, item_type, item_number, (item_descrip1 || ' ' || item_descrip2),"
               "       warehous_code, formatQty(itemsite_safetystock) "
               "FROM itemsite, item, warehous "
               "WHERE ((itemsite_active)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (item_planning_type='S')" );

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY item_number, warehous_code";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _itemsite->populate(q);
}

void dspMPSDetail::sFillMPSDetail()
{
  _mps->clear();

  while (_mps->columns() > 1)
    _mps->removeColumn(1);

  if (_periods->isPeriodSelected())
  {
    int           counter = 1;
    QString       sql( "SELECT itemsite_qtyonhand, itemsite_safetystock" );
    XListViewItem *cursor = _periods->firstChild();

    do
    {
      if (_periods->isSelected(cursor))
      {
        sql += QString( ", qtyAllocated(itemsite_id, findPeriodStart(%1), findPeriodEnd(%2)) AS allocations%3, "
                        "qtyOrdered(itemsite_id, findPeriodStart(%4), findPeriodEnd(%5)) AS orders%6,"
                        "qtyPlanned(itemsite_id, findPeriodStart(%7), findPeriodEnd(%8)) AS planned%9" )
               .arg(cursor->id())
               .arg(cursor->id())
               .arg(counter)
               .arg(cursor->id())
               .arg(cursor->id())
               .arg(counter)
               .arg(cursor->id())
               .arg(cursor->id())
               .arg(counter);

        sql += QString( ", qtyForecasted(itemsite_id, findPeriodStart(%1), findPeriodEnd(%2)) AS forecast%3")
               .arg(cursor->id())
               .arg(cursor->id())
               .arg(counter);

        sql += QString( ", (findPeriodStart(%1) < (CURRENT_DATE+itemsite_mps_timefence)) AS inside%2")
               .arg(cursor->id())
               .arg(counter);

        _mps->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _qtyColumn, Qt::AlignRight);
        counter++;
      }
    }
    while ((cursor = cursor->nextSibling()) != 0);

    sql +=  QString( " FROM itemsite "
                     "WHERE (itemsite_id=%1);" )
            .arg(_itemsite->id());

    q.prepare(sql);
    q.exec();
    if (q.first())
    {
      XListViewItem *qoh;
      XListViewItem *forecast;
      XListViewItem *allocations;
      XListViewItem *orders;
      XListViewItem *availability;
      XListViewItem *planned;
      XListViewItem *available;
      double        forecasted, actual, demand;
      double        runningAvailability;
      //double        safetyStock = q.value("itemsite_safetystock").toDouble();
      double        toPromise;
      int           lastAtp;

      forecasted = q.value("forecast1").toDouble();
      actual = q.value("allocations1").toDouble();
      if((forecasted > actual) && !q.value("inside1").toBool())
        demand = forecasted;
      else
        demand = actual;

      runningAvailability = (q.value("itemsite_qtyonhand").toDouble() - demand + q.value("orders1").toDouble());
      runningAvailability += q.value("planned1").toDouble();
      lastAtp = 1;
      toPromise = q.value("itemsite_qtyonhand").toDouble()
                + q.value("orders1").toDouble()
                + q.value("planned1").toDouble()
                - actual;

      forecast            = new XListViewItem(_mps, 0, QVariant(tr("Forecast")), formatQty(forecasted));
      allocations         = new XListViewItem(_mps, forecast, 0, QVariant(tr("Allocations")), formatQty(actual));
      orders              = new XListViewItem(_mps, allocations,  0, QVariant(tr("Orders")), formatQty(q.value("orders1").toDouble()));
      availability        = new XListViewItem(_mps, orders, 0, QVariant(tr("Projected QOH")), formatQty(runningAvailability));
      planned             = new XListViewItem(_mps, availability, 0, QVariant(tr("Planned Orders")), formatQty(q.value("planned1").toDouble()));
      qoh                 = new XListViewItem(_mps, planned, 0, QVariant(tr("Availability")), formatQty(runningAvailability - q.value("planned1").toDouble()));
      available           = new XListViewItem(_mps, qoh, 0, QVariant(tr("Available to Promise")) );
                       
      for (int bucketCounter = 2; bucketCounter < counter; bucketCounter++)
      {
        forecasted = q.value(QString("forecast%1").arg(bucketCounter)).toDouble();
        actual = q.value(QString("allocations%1").arg(bucketCounter)).toDouble();
        if((forecasted > actual) && !q.value(QString("inside%1").arg(bucketCounter)).toBool())
          demand = forecasted;
        else
          demand = actual;

        runningAvailability =  ( runningAvailability - demand + q.value(QString("orders%1").arg(bucketCounter)).toDouble() );

        if((q.value(QString("orders%1").arg(bucketCounter)).toDouble() > 0.0 || q.value(QString("planned%1").arg(bucketCounter)).toDouble()) > 0.0)
        {
          available->setText(lastAtp, formatQty(toPromise));
          lastAtp = bucketCounter;
          if(toPromise > 0.0)
            toPromise = 0.0;
          toPromise += q.value(QString("orders%1").arg(bucketCounter)).toDouble()
                     + q.value(QString("planned%1").arg(bucketCounter)).toDouble();
        }
        toPromise -= actual;

        forecast->setText(bucketCounter, formatQty(forecasted));
        allocations->setText(bucketCounter, formatQty(actual));
        orders->setText(bucketCounter, formatQty(q.value(QString("orders%1").arg(bucketCounter)).toDouble()));
        qoh->setText(bucketCounter, formatQty(runningAvailability));
        planned->setText(bucketCounter, formatQty(q.value(QString("planned%1").arg(bucketCounter)).toDouble()));

        runningAvailability += q.value(QString("planned%1").arg(bucketCounter)).toDouble();
                                                       
        availability->setText(bucketCounter, formatQty(runningAvailability));
      }
      available->setText(lastAtp, formatQty(toPromise));
    }
  }
}
