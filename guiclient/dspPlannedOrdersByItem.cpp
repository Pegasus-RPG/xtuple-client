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

#include "dspPlannedOrdersByItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <Q3PopupMenu>
#include <parameter.h>
#include "dspRunningAvailability.h"
#include "firmPlannedOrder.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "deletePlannedOrder.h"
#include "rptPlannedOrdersByItem.h"

/*
 *  Constructs a dspPlannedOrdersByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPlannedOrdersByItem::dspPlannedOrdersByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_planord, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  statusBar()->hide();

  _planord->addColumn(tr("Order #"),    _orderColumn, Qt::AlignCenter );
  _planord->addColumn(tr("Type"),       _uomColumn,   Qt::AlignCenter );
  _planord->addColumn(tr("Whs."),       _whsColumn,   Qt::AlignCenter );
  _planord->addColumn(tr("Start Date"), _dateColumn,  Qt::AlignCenter );
  _planord->addColumn(tr("Due Date"),   _dateColumn,  Qt::AlignCenter );
  _planord->addColumn(tr("Qty"),        _qtyColumn,   Qt::AlignRight  );
  _planord->addColumn(tr("Firm"),       _uomColumn,   Qt::AlignCenter );
  _planord->addColumn(tr("Comments"),   -1,           Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPlannedOrdersByItem::~dspPlannedOrdersByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPlannedOrdersByItem::languageChange()
{
    retranslateUi(this);
}

void dspPlannedOrdersByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");
  _warehouse->appendValue(params);

  rptPlannedOrdersByItem newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspPlannedOrdersByItem::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()), 0);
  pMenu->insertSeparator();
  if (!_privleges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (pSelected->text(6) == "No")
  {
    menuItem = pMenu->insertItem(tr("Firm Order..."), this, SLOT(sFirmOrder()), 0);
    if (!_privleges->check("FirmPlannedOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Soften Order..."), this, SLOT(sSoftenOrder()), 0);
    if (!_privleges->check("SoftenPlannedOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("Release Order..."), this, SLOT(sReleaseOrder()), 0);
  if ( (!_privleges->check("ReleasePlannedOrders")) ||
       ((pSelected->text(1) == "W/O") && (!_privleges->check("MaintainWorkOrders")) ) ||
       ((pSelected->text(1) == "P/O") && (!_privleges->check("MaintainPurchaseRequests")) ) )
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Order..."), this, SLOT(sDeleteOrder()), 0);
  if (!_privleges->check("DeletePlannedOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPlannedOrdersByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _planord->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPlannedOrdersByItem::sFirmOrder()
{
  ParameterList params;
  params.append("planord_id", _planord->id());

  firmPlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByItem::sSoftenOrder()
{
  q.prepare( "UPDATE planord "
             "SET planord_firm=FALSE "
             "WHERE (planord_id=:planord_id);" );
  q.bindValue(":planord_id", _planord->id());
  q.exec();

  sFillList();
}

void dspPlannedOrdersByItem::sReleaseOrder()
{
  if (_planord->selectedItem()->text(1) == "W/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _planord->id());

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (_planord->selectedItem()->text(1) == "P/O")
  {
    ParameterList params;
    params.append("mode", "release");
    params.append("planord_id", _planord->id());

    purchaseRequest newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      sFillList();
  }
}

void dspPlannedOrdersByItem::sDeleteOrder()
{
  ParameterList params;
  params.append("planord_id", _planord->id());

  deletePlannedOrder newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspPlannedOrdersByItem::sFillList()
{
  QString sql( "SELECT planord_id, planord_itemsite_id,"
               "       formatPloNumber(planord_id),"
               "       CASE WHEN (planord_type='P') THEN 'P/O'"
               "            WHEN (planord_type='W') THEN 'W/O'"
               "            ELSE '?'"
               "       END,"
               "       warehous_code, formatDate(planord_startdate),"
               "       formatDate(planord_duedate),"
               "       formatQty(planord_qty), formatBoolYN(planord_firm),"
               "       firstLine(planord_comments) "
               "FROM planord, itemsite, warehous "
               "WHERE ( (planord_itemsite_id=itemsite_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=:item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY planord_duedate, planord_number, planord_subnumber;";

  q.prepare(sql);
  q.bindValue(":item_id", _item->id());
  _warehouse->bindValue(q);
  q.exec();
  _planord->populate(q, TRUE);
}

