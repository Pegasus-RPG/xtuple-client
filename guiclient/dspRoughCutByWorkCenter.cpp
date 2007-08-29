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

#include "dspRoughCutByWorkCenter.h"

#include <QVariant>
#include <QStatusBar>
#include <parameter.h>
#include "rptRoughCutByWorkCenter.h"

/*
 *  Constructs a dspRoughCutByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspRoughCutByWorkCenter::dspRoughCutByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _workCenterGroupInt = new QButtonGroup(this);
    _workCenterGroupInt->addButton(_allWorkCenters);
    _workCenterGroupInt->addButton(_selectedWorkCenter);

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_roughCut, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_selectedWorkCenter, SIGNAL(toggled(bool)), _wrkcnt, SLOT(setEnabled(bool)));
    connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspRoughCutByWorkCenter::~dspRoughCutByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspRoughCutByWorkCenter::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspRoughCutByWorkCenter::init()
{
  statusBar()->hide();

  _wrkcnt->populate( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip) "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );

  _roughCut->addColumn(tr("Whs."),         _whsColumn,  Qt::AlignCenter );
  _roughCut->addColumn(tr("Work Center"),  -1,          Qt::AlignLeft   );
  _roughCut->addColumn(tr("Total Setup"),  _timeColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Setup $"),      _costColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Total Run"),    _timeColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Run $"),        _costColumn, Qt::AlignRight  );
}

void dspRoughCutByWorkCenter::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("print");

  if (_selectedWorkCenter->isChecked())
    params.append("wrkcnt_id", _wrkcnt->id());

  rptRoughCutByWorkCenter newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspRoughCutByWorkCenter::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspRoughCutByWorkCenter::sQuery()
{
  QString sql( "SELECT wrkcnt_id, warehous_code, wrkcnt_code,"
               "       formatTime(SUM(planoper_sutime)),"
               "       formatCost(SUM(planoper_sutime) * wrkcnt_setuprate / 60.0),"
               "       formatTime(SUM(planoper_rntime)),"
               "       formatCost(SUM(planoper_rntime) * wrkcnt_runrate / 60.0) "
               "FROM planoper, planord, wrkcnt, warehous "
               "WHERE ( (planoper_planord_id=planord_id)"
               " AND (planoper_wrkcnt_id=wrkcnt_id)"
               " AND (wrkcnt_warehous_id=warehous_id)"
               " AND (planord_startdate BETWEEN :startDate AND :endDate)" );

  if (_selectedWorkCenter->isChecked())
    sql += " AND (wrkcnt_id=:wrkcnt_id)";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY wrkcnt_id, warehous_code, wrkcnt_code,"
         "         wrkcnt_setuprate, wrkcnt_runrate "
         "ORDER BY warehous_code, wrkcnt_code;";

  q.prepare(sql);
  _dates->bindValue(q);
  _warehouse->bindValue(q);
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  _roughCut->populate(q);
}

