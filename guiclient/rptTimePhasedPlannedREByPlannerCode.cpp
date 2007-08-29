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

#include "rptTimePhasedPlannedREByPlannerCode.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>

/*
 *  Constructs a rptTimePhasedPlannedREByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptTimePhasedPlannedREByPlannerCode::rptTimePhasedPlannedREByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    QButtonGroup* _costGroupInt = new QButtonGroup(this);
    _costGroupInt->addButton(_useStandardCost);
    _costGroupInt->addButton(_useActualCost);

    QButtonGroup* _salesPriceGroupInt = new QButtonGroup(this);
    _salesPriceGroupInt->addButton(_useListPrice);
    _salesPriceGroupInt->addButton(_useAveragePrice);

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _startEvalDateLit, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _startEvalDate, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _endEvalDateLit, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _endEvalDate, SLOT(setEnabled(bool)));
    connect(_calendar, SIGNAL(select(const PeriodList&)), _periods, SLOT(load(const PeriodList&)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptTimePhasedPlannedREByPlannerCode::~rptTimePhasedPlannedREByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptTimePhasedPlannedREByPlannerCode::languageChange()
{
    retranslateUi(this);
}


void rptTimePhasedPlannedREByPlannerCode::init()
{
  _captive = FALSE;

  _plannerCode->setType(PlannerCode);
}

enum SetResponse rptTimePhasedPlannedREByPlannerCode::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("plancode_id", &valid);
  if (valid)
    _plannerCode->setId(param.toInt());

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
    _plannerCode->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  _useStandardCost->setChecked(pParams.inList("standardCost"));
  _useActualCost->setChecked(pParams.inList("actualCost"));
  _useListPrice->setChecked(pParams.inList("listPrice"));
  _useAveragePrice->setChecked(pParams.inList("averagePrice"));

  param = pParams.value("startDate", &valid);
  if (valid)
    _startEvalDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endEvalDate->setDate(param.toDate());

  _calendar->load(pParams);
  _periods->load(pParams);

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void rptTimePhasedPlannedREByPlannerCode::sPrint()
{
  if ( (_periods->isPeriodSelected()) &&
       ( (_useListPrice->isChecked()) ||
         ( (_useAveragePrice->isChecked()) && (_startEvalDate->isValid()) && (_endEvalDate->isValid()) ) ) )
  {
    ParameterList params;

    _plannerCode->appendValue(params);
    _warehouse->appendValue(params);

    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);

    if (_useActualCost->isChecked())
      params.append("actualCost");
    else if (_useStandardCost->isChecked())
      params.append("standardCost");

    if (_useAveragePrice->isChecked())
    {
      params.append("averagePrice");
      params.append("startEvalDate", _startEvalDate->dateString());
      params.append("endEvalDate", _endEvalDate->dateString());
    }
    else if (_useListPrice->isChecked())
      params.append("listPrice");


    orReport report("TimePhasedPlannedRevenueExpensesByPlannerCode", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    if (_captive)
      accept();
  }
  else
    QMessageBox::critical( this, tr("Incomplete criteria"),
                           tr( "The criteria you specified is not complete. Please make sure all\n"
                               "fields are correctly filled out before running the report." ) );
}
