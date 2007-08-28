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

#include "dspTimePhasedAvailableCapacityByWorkCenter.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <datecluster.h>
#include <qworkspace.h>
#include <parameter.h>
#include "dspWoOperationsByWorkCenter.h"
#include "rptTimePhasedAvailableCapacityByWorkCenter.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspTimePhasedAvailableCapacityByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedAvailableCapacityByWorkCenter::dspTimePhasedAvailableCapacityByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_load, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*,int)));
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
dspTimePhasedAvailableCapacityByWorkCenter::~dspTimePhasedAvailableCapacityByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedAvailableCapacityByWorkCenter::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspTimePhasedAvailableCapacityByWorkCenter::init()
{
  statusBar()->hide();

  _load->addColumn(tr("Work Center"), _itemColumn, Qt::AlignLeft);
}

void dspTimePhasedAvailableCapacityByWorkCenter::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _periods->getSelected(params);
  params.append("print");

  rptTimePhasedAvailableCapacityByWorkCenter newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspTimePhasedAvailableCapacityByWorkCenter::sViewLoad()
{
  ParameterList params;
  params.append("wrkcnt_id", _load->id());
  params.append("startDate", _columnDates[_column - 1].startDate);
  params.append("endDate", _columnDates[_column - 1].endDate);
  params.append("run");

  dspWoOperationsByWorkCenter *newdlg = new dspWoOperationsByWorkCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailableCapacityByWorkCenter::sPopulateMenu(Q3PopupMenu *menu, Q3ListViewItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 0)
  {
    menuItem = menu->insertItem(tr("View Load..."), this, SLOT(sViewLoad()), 0);
    if ( (!_privleges->check("MaintainWoOperations")) && (!_privleges->check("ViewWoOperations")) )
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedAvailableCapacityByWorkCenter::sFillList()
{
  _columnDates.clear();
  while (_load->columns() > 1)
    _load->removeColumn(1);

  QString sql("SELECT wrkcnt_id, wrkcnt_code ");

  int columns = 1;
  XListViewItem *cursor = _periods->firstChild();
  if (cursor != 0)
  {
    do
    {
      if (_periods->isSelected(cursor))
      {
        sql += QString(", formatTime(workCenterCapacity(wrkcnt_id, %1) - loadByWorkCenter(wrkcnt_id, %2)) AS bucket%3")
               .arg(cursor->id())
               .arg(cursor->id())
               .arg(columns);
  
        _load->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _timeColumn, Qt::AlignRight);

        _columnDates.append(DatePair(((PeriodListViewItem *)cursor)->startDate(), ((PeriodListViewItem *)cursor)->endDate()));
      }
    }
    while ((cursor = cursor->nextSibling()) != 0);
  }

  sql += " FROM wrkcnt ";

  if (_warehouse->isSelected())
    sql += "WHERE (wrkcnt_warehous_id=:warehous_id) ";

  sql += "ORDER BY wrkcnt_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();
  _load->populate(q);
}

