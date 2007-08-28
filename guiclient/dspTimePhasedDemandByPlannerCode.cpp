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

#include "dspTimePhasedDemandByPlannerCode.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <qworkspace.h>
#include <datecluster.h>
#include "dspWoScheduleByParameterList.h"
#include "rptTimePhasedDemandByPlannerCode.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspTimePhasedDemandByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedDemandByPlannerCode::dspTimePhasedDemandByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* btngrpDisplayUnits = new QButtonGroup(this);
    btngrpDisplayUnits->addButton(_inventoryUnits);
    btngrpDisplayUnits->addButton(_capacityUnits);
    btngrpDisplayUnits->addButton(_altCapacityUnits);

    // signals and slots connections
    connect(_demand, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*,int)));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedDemandByPlannerCode::~dspTimePhasedDemandByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedDemandByPlannerCode::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspTimePhasedDemandByPlannerCode::init()
{
  statusBar()->hide();

  _plannerCode->setType(PlannerCode);

  _demand->addColumn(tr("Planner Code"), _itemColumn, Qt::AlignLeft   );
  _demand->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _demand->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignLeft   );
}

void dspTimePhasedDemandByPlannerCode::sPrint()
{
  ParameterList params;
  params.append("print");
  _periods->getSelected(params);
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_inventoryUnits->isChecked())
    params.append("inventoryUnits");
  else if (_capacityUnits->isChecked())
    params.append("capacityUnits");
  else if (_altCapacityUnits->isChecked())
    params.append("altCapacityUnits");

  rptTimePhasedDemandByPlannerCode newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspTimePhasedDemandByPlannerCode::sViewDemand()
{
  ParameterList params;
  params.append("plancode");
  params.append("plancode_id", _demand->id());
  params.append("warehous_id", _demand->altId());
  params.append("startDate", _columnDates[_column - 3].startDate);
  params.append("endDate", _columnDates[_column - 3].endDate);
  params.append("run");
  
  dspWoScheduleByParameterList *newdlg = new dspWoScheduleByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedDemandByPlannerCode::sPopulateMenu(Q3PopupMenu *menu, Q3ListViewItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;

  if (_column > 1)
  {
    menuItem = menu->insertItem(tr("View Demand..."), this, SLOT(sViewDemand()), 0);
    if ( (!_privleges->check("MaintainWorkOrders")) && (!_privleges->check("ViewWorkOrders")) )
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedDemandByPlannerCode::sFillList()
{
  _columnDates.clear();
  while (_demand->columns() > 3)
    _demand->removeColumn(3);

  QString sql("SELECT plancode_id, warehous_id, plancode_code, warehous_code, ");

  if (_inventoryUnits->isChecked())
    sql += "item_invuom AS uom";

  else if (_capacityUnits->isChecked())
    sql += "item_capuom AS uom";

  else if (_altCapacityUnits->isChecked())
    sql += "item_altcapuom AS uom";

  int columns = 1;
  XListViewItem *cursor = _periods->firstChild();
  if (cursor != 0)
  {
    do
    {
      if (_periods->isSelected(cursor))
      {
        if (_inventoryUnits->isChecked())
          sql += QString(", formatQty(SUM(summDemand(itemsite_id, %1))) AS bucket%2")
                 .arg(cursor->id())
                 .arg(columns++);
  
        else if (_capacityUnits->isChecked())
          sql += QString(", formatQty(SUM(summDemand(itemsite_id, %1) * item_capinvrat)) AS bucket%2")
                 .arg(cursor->id())
                 .arg(columns++);
  
        else if (_altCapacityUnits->isChecked())
          sql += QString(", formatQty(SUM(summDemand(itemsite_id, %1) * item_altcapinvrat)) AS bucket%2")
                 .arg(cursor->id())
                 .arg(columns++);
  
        _demand->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _timeColumn, Qt::AlignRight);

        _columnDates.append(DatePair(((PeriodListViewItem *)cursor)->startDate(), ((PeriodListViewItem *)cursor)->endDate()));
      }
    }
    while ((cursor = cursor->nextSibling()) != 0);
  }

  sql += " FROM itemsite, item, warehous, plancode "
         "WHERE ( (itemsite_active)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_plancode_id=plancode_id)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
 
  if (_plannerCode->isSelected())
    sql += " AND (plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (plancode_code ~ :plancode_pattern) ";

  sql +=  ") "
         "GROUP BY plancode_id, warehous_id, plancode_code, warehous_code, uom;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();
  _demand->populate(q, TRUE);
}

