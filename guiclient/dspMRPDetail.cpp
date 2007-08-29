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

#include "dspMRPDetail.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <datecluster.h>
#include "dspAllocations.h"
#include "dspOrders.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "rptMRPDetail.h"

/*
 *  Constructs a dspMRPDetail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspMRPDetail::dspMRPDetail(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_itemsite, SIGNAL(itemSelectionChanged()), this, SLOT(sFillMRPDetail()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_mrp, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
    connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillItemsites()));
    connect(_plannerCode, SIGNAL(updated()), this, SLOT(sFillItemsites()));
    connect(_itemsite, SIGNAL(itemSelected(int)), this, SLOT(sFillMRPDetail()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspMRPDetail::~dspMRPDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspMRPDetail::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspMRPDetail::init()
{
  statusBar()->hide();

  _plannerCode->setType(PlannerCode);

  _itemsite->addColumn("Itemtype",        0,           Qt::AlignCenter );
  _itemsite->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _itemsite->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _itemsite->addColumn(tr("Whs."),        _whsColumn,  Qt::AlignCenter );

  _mrp->addColumn("", 120, Qt::AlignRight);

  sFillItemsites();
}

void dspMRPDetail::sPrint()
{
  ParameterList params;
  params.append("print");
  params.append("itemsite_id", _itemsite->id());
  _periods->getSelected(params);
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  rptMRPDetail newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspMRPDetail::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  int           menuItem;
  int		mrpIndex = 0;

  _column = pColumn;

  menuItem = pMenu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
  while ((mrpIndex < _mrp->topLevelItemCount()) &&
	 (_mrp->topLevelItem(mrpIndex)->text(0) != tr("Allocations")))
    mrpIndex++;
  if (_mrp->topLevelItem(mrpIndex)->text(pColumn).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  while ((mrpIndex < _mrp->topLevelItemCount()) &&
	 (_mrp->topLevelItem(mrpIndex)->text(0) != tr("Orders")))
    mrpIndex++;
  if (_mrp->topLevelItem(mrpIndex)->text(pColumn).toDouble() == 0.0)
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

void dspMRPDetail::sViewAllocations()
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

void dspMRPDetail::sViewOrders()
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

void dspMRPDetail::sIssuePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillMRPDetail();
}

void dspMRPDetail::sIssuePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspMRPDetail::sIssueWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMRPDetail::sFillItemsites()
{
  QString sql( "SELECT itemsite_id, item_type, item_number, (item_descrip1 || ' ' || item_descrip2),"
               "       warehous_code "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_active)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (item_planning_type='M')" );

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY item_number, warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _itemsite->populate(q);
}

void dspMRPDetail::sFillMRPDetail()
{
  _mrp->clear();

  _mrp->setColumnCount(1);

  QString       sql( "SELECT formatQty(itemsite_qtyonhand) AS f_qoh,"
                     "       itemsite_qtyonhand" );

  int           counter = 1;
  bool          show    = FALSE;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    sql += QString(", qtyAllocated(itemsite_id, findPeriodStart(%1), findPeriodEnd(%2)) AS allocations%3, ")
	   .arg(cursor->id())
	   .arg(cursor->id())
	   .arg(counter);

    sql += QString("qtyOrdered(itemsite_id, findPeriodStart(%1), findPeriodEnd(%2)) AS orders%3,")
	   .arg(cursor->id())
	   .arg(cursor->id())
	   .arg(counter);

    sql += QString("qtyFirmedAllocated(itemsite_id, findPeriodStart(%1), findPeriodEnd(%2)) AS firmedallocations%3,")
	   .arg(cursor->id())
	   .arg(cursor->id())
	   .arg(counter);

    sql += QString("qtyFirmed(itemsite_id, findPeriodStart(%1), findPeriodEnd(%2)) AS firmedorders%3")
	   .arg(cursor->id())
	   .arg(cursor->id())
	   .arg(counter);

    _mrp->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _qtyColumn, Qt::AlignRight);
    counter++;

    show = TRUE;
  }

  sql +=  " FROM itemsite "
          "WHERE (itemsite_id=:itemsite_id);";

  if (show)
  {
    XSqlQuery mrp;
    mrp.prepare(sql);
    mrp.bindValue(":itemsite_id", _itemsite->id());
    mrp.exec();
    if (mrp.first())
    {
      XTreeWidgetItem *qoh;
      XTreeWidgetItem *allocations;
      XTreeWidgetItem *orders;
      XTreeWidgetItem *availability;
      XTreeWidgetItem *firmedAllocations;
      XTreeWidgetItem *firmedOrders;
      XTreeWidgetItem *firmedAvailability;
      double        runningAvailability;
      double	    runningFirmed;

      runningFirmed = mrp.value("firmedorders1").toDouble();
      runningAvailability = (mrp.value("itemsite_qtyonhand").toDouble() - mrp.value("allocations1").toDouble() + mrp.value("orders1").toDouble());

      qoh                = new XTreeWidgetItem(_mrp, 0, QVariant(tr("Projected QOH")), mrp.value("f_qoh"));
      allocations        = new XTreeWidgetItem(_mrp, qoh, 0, QVariant(tr("Allocations")), formatQty(mrp.value("allocations1").toDouble()));
      orders             = new XTreeWidgetItem(_mrp, allocations,  0, QVariant(tr("Orders")), formatQty(mrp.value("orders1").toDouble()));
      availability       = new XTreeWidgetItem(_mrp, orders, 0, QVariant(tr("Availability")), formatQty(runningAvailability));
      firmedAllocations  = new XTreeWidgetItem(_mrp, availability, 0, QVariant(tr("Firmed Allocations")), formatQty(mrp.value("firmedallocations1").toDouble()));
      firmedOrders       = new XTreeWidgetItem(_mrp, firmedAllocations, 0, QVariant(tr("Firmed Orders")), formatQty(runningFirmed));
      firmedAvailability = new XTreeWidgetItem(_mrp, firmedOrders, 0, QVariant(tr("Firmed Availability")), formatQty( runningAvailability -
                                                                                                          mrp.value("firmedallocations1").toDouble() +
                                                                                                          runningFirmed ) );
                       
      for (int bucketCounter = 2; bucketCounter < counter; bucketCounter++)
      {
        qoh->setText(bucketCounter, formatQty(runningAvailability));
        allocations->setText(bucketCounter, formatQty(mrp.value(QString("allocations%1").arg(bucketCounter)).toDouble()));
        orders->setText(bucketCounter, formatQty(mrp.value(QString("orders%1").arg(bucketCounter)).toDouble()));

        runningAvailability =  ( runningAvailability - mrp.value(QString("allocations%1").arg(bucketCounter)).toDouble() +
                                                       mrp.value(QString("orders%1").arg(bucketCounter)).toDouble() );
        availability->setText(bucketCounter, formatQty(runningAvailability));

        firmedAllocations->setText(bucketCounter, formatQty(mrp.value(QString("firmedallocations%1").arg(bucketCounter)).toDouble()));

        runningFirmed += mrp.value(QString("firmedorders%1").arg(bucketCounter)).toDouble();
        firmedOrders->setText(bucketCounter, formatQty(runningFirmed));
        firmedAvailability->setText(bucketCounter, formatQty( runningAvailability -
                                                              mrp.value(QString("firmedallocations%1").arg(bucketCounter)).toDouble() +
                                                              runningFirmed ) );
      }
    }
  }
}
