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

#include "dspTimePhasedAvailability.h"

#include <qvariant.h>
#include <qworkspace.h>
#include <qstatusbar.h>
#include <datecluster.h>
#include "OpenMFGGUIClient.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "rptTimePhasedAvailability.h"

/*
 *  Constructs a dspTimePhasedAvailability as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedAvailability::dspTimePhasedAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sCalculate()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
    connect(_availability, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*,int)));
    init();
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

//Added by qt3to4:
#include <Q3PopupMenu>

void dspTimePhasedAvailability::init()
{
  statusBar()->hide();

  _plannerCode->setType(PlannerCode);

  _availability->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _availability->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft   );
  _availability->addColumn(tr("Whs."),        _whsColumn,  Qt::AlignCenter );
}

void dspTimePhasedAvailability::sPrint()
{
  ParameterList params;
  params.append("print");
  _periods->getSelected(params);
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  rptTimePhasedAvailability newdlg(this, "", TRUE);
  newdlg.set(params);
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

void dspTimePhasedAvailability::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 2)
  {
    menuItem = pMenu->insertItem(tr("View Availability Detail..."), this, SLOT(sViewAvailability()), 0);
    menuItem = pMenu->insertItem(tr("View Allocations..."), this, SLOT(sViewAllocations()), 0);
    menuItem = pMenu->insertItem(tr("View Orders..."), this, SLOT(sViewOrders()), 0);
  
    if (((XListViewItem *)pSelected)->altId() == 1)
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("Create W/O..."), this, SLOT(sCreateWO()), 0);
    }
    else if (((XListViewItem *)pSelected)->altId() == 2)
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
  while (_availability->columns() > 3)
    _availability->removeColumn(3);

  QString sql( "SELECT itemsite_id, item_number, item_invuom, warehous_code,"
               "       CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel,"
               "       CASE WHEN (item_type IN ('F', 'B', 'C', 'Y', 'R')) THEN 0"
               "            WHEN (item_type IN ('M')) THEN 1"
               "            WHEN (item_type IN ('P', 'O')) THEN 2"
               "            ELSE 0"
               "       END AS itemtype" );

  int columns = 1;
  XListViewItem *cursor = _periods->firstChild();
  if (cursor != 0)
  {
    do
    {
      if (_periods->isSelected(cursor))
      {
        sql += QString(", formatQty(qtyAvailable(itemsite_id, findPeriodStart(%1))) AS bucket%2")
               .arg(cursor->id())
               .arg(columns++);

        _availability->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _qtyColumn, Qt::AlignRight);

        _columnDates.append(DatePair(((PeriodListViewItem *)cursor)->startDate(), ((PeriodListViewItem *)cursor)->endDate()));
      }
    }
    while ((cursor = cursor->nextSibling()) != 0);
  }

  sql += " FROM itemsite, item, warehous "
         "WHERE ((itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  if (q.first())
  {
    double        reorderLevel;
    XListViewItem *last = NULL;

    do
    {
      reorderLevel = q.value("reorderlevel").toDouble();

      last = new XListViewItem( _availability, last, q.value("itemsite_id").toInt(), q.value("itemtype").toInt(),
                                q.value("item_number"), q.value("item_invuom"),
                                q.value("warehous_code") );

      for (int column = 1; column < columns; column++)
      {
        last->setText((column + 2), q.value(QString("bucket%1").arg(column)).toString());

        if (q.value(QString("bucket%1").arg(column)).toDouble() < reorderLevel)
          last->setColor((column + 2), "red");

      }
    }
    while (q.next());
  }
}

