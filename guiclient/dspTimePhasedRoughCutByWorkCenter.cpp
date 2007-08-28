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

#include "dspTimePhasedRoughCutByWorkCenter.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <datecluster.h>
#include <parameter.h>
#include "rptTimePhasedRoughCutByWorkCenter.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspTimePhasedRoughCutByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspTimePhasedRoughCutByWorkCenter::dspTimePhasedRoughCutByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_selectedWorkCenter, SIGNAL(toggled(bool)), _workCenters, SLOT(setEnabled(bool)));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_calendar, SIGNAL(select(ParameterList&)), _periods, SLOT(load(ParameterList&)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspTimePhasedRoughCutByWorkCenter::~dspTimePhasedRoughCutByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspTimePhasedRoughCutByWorkCenter::languageChange()
{
    retranslateUi(this);
}


void dspTimePhasedRoughCutByWorkCenter::init()
{
  statusBar()->hide();

  _workCenters->populate( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip) "
                          "FROM wrkcnt "
                          "ORDER BY wrkcnt_code;" );

  _roughCut->addColumn("", 80, Qt::AlignRight);
}

void dspTimePhasedRoughCutByWorkCenter::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _periods->getSelected(params);
  params.append("print");

  if(_selectedWorkCenter->isChecked())
    params.append("wrkcnt_id", _workCenters->id());

  rptTimePhasedRoughCutByWorkCenter newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspTimePhasedRoughCutByWorkCenter::sFillList()
{
  _columnDates.clear();
  _roughCut->clear();
  while (_roughCut->columns() > 1)
    _roughCut->removeColumn(1);

  QString sql("SELECT");
  int     columns = 1;
  bool    show    = FALSE;
  XListViewItem *cursor = _periods->firstChild();
  if (cursor)
  {
    do
    {
      if (_periods->isSelected(cursor))
      {
        if (show)
          sql += ",";
        else
          show = TRUE;

        sql += QString( " formatTime(SUM(plannedSetupTime(wrkcnt_id, %1))) AS setup%2,"
                        " formatCost(SUM(plannedSetupTime(wrkcnt_id, %3) * wrkcnt_setuprate / 60.0)) AS setupcost%4,"
                        " formatTime(SUM(plannedRunTime(wrkcnt_id, %5))) AS run%6,"
                        " formatCost(SUM(plannedRunTime(wrkcnt_id, %7) * wrkcnt_runrate / 60.0)) AS runcost%8" )
               .arg(cursor->id())
               .arg(columns)
               .arg(cursor->id())
               .arg(columns)
               .arg(cursor->id())
               .arg(columns)
               .arg(cursor->id())
               .arg(columns++);
    
        _roughCut->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _qtyColumn, Qt::AlignRight);

        _columnDates.append(DatePair(((PeriodListViewItem *)cursor)->startDate(), ((PeriodListViewItem *)cursor)->endDate()));
      }
    }
    while ((cursor = cursor->nextSibling()) != 0);
  }

  sql += " FROM wrkcnt ";

  if (_warehouse->isSelected())
    sql += "WHERE (wrkcnt_warehous_id=:warehous_id)";

  if (_selectedWorkCenter->isChecked())
  {
    if (_warehouse->isSelected())
      sql += " AND (wrkcnt_id=:wrkcnt_id);";
    else
      sql += "WHERE (wrkcnt_id=:wrkcnt_id);";
  }

  if (show)
  {
    q.prepare(sql);
    _warehouse->bindValue(q);
    q.bindValue(":wrkcnt_id", _workCenters->id());
    q.exec();
    if (q.first())
    {
      XListViewItem *setup     = new XListViewItem(_roughCut, 0, QVariant(tr("Setup Time")), q.value("setup1"));
      XListViewItem *setupCost = new XListViewItem(_roughCut, setup, 0, QVariant(tr("Setup Cost")), q.value("setupcost1"));
      XListViewItem *run       = new XListViewItem(_roughCut, setupCost,  0, QVariant(tr("Run Time")), q.value("run1"));
      XListViewItem *runCost   = new XListViewItem(_roughCut, run, 0, QVariant(tr("Run Cost")), q.value("runcost1"));
                       
      for (int column = 1; column < columns; column++)
      {
        setup->setText((column + 1), q.value(QString("setup%1").arg(column)).toString());
        setupCost->setText((column + 1), q.value(QString("setupcost%1").arg(column)).toString());
        run->setText((column + 1), q.value(QString("run%1").arg(column)).toString());
        runCost->setText((column + 1), q.value(QString("runcost%1").arg(column)).toString());
      }
    }
  }
}

