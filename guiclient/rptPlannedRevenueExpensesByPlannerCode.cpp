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

#include "rptPlannedRevenueExpensesByPlannerCode.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>

/*
 *  Constructs a rptPlannedRevenueExpensesByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptPlannedRevenueExpensesByPlannerCode::rptPlannedRevenueExpensesByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
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
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _startEvalDateLit, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _startEvalDate, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _endEvalDateLit, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _endEvalDate, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptPlannedRevenueExpensesByPlannerCode::~rptPlannedRevenueExpensesByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptPlannedRevenueExpensesByPlannerCode::languageChange()
{
    retranslateUi(this);
}


void rptPlannedRevenueExpensesByPlannerCode::init()
{
  _captive = FALSE;

  _plannerCode->setType(PlannerCode);
}

enum SetResponse rptPlannedRevenueExpensesByPlannerCode::set(ParameterList &pParams)
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

  param = pParams.value("startDate", &valid);
  if (valid)
    _startDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endDate->setDate(param.toDate());

  param = pParams.value("startEvalDate", &valid);
  if (valid)
    _startEvalDate->setDate(param.toDate());

  param = pParams.value("endEvalDate", &valid);
  if (valid)
    _endEvalDate->setDate(param.toDate());

  _useStandardCost->setChecked(pParams.inList("useStandardCost"));
  _useActualCost->setChecked(pParams.inList("useActualCost"));
  _useListPrice->setChecked(pParams.inList("useListPrice"));
  _useAveragePrice->setChecked(pParams.inList("useAveragePrice"));

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError;
  }

  return NoError;
}

void rptPlannedRevenueExpensesByPlannerCode::sPrint()
{
  if(!_startDate->isValid()) {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _startDate->setFocus();
      return;
  }

  if(!_endDate->isValid()) {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _endDate->setFocus();
      return;
  }

  if (_useAveragePrice->isChecked())
  {
    if ( (!_startEvalDate->isValid()) || (!_endEvalDate->isValid()) )
    {
      QMessageBox::critical( this, tr("Enter Start and End Evaluation Dates"),
                             tr("You must enter both a start and end date for Average Sales Price calculation") );
      return;
    }
  }

  ParameterList params;

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  params.append("startDate", _startDate->date());
  params.append("endDate", _endDate->date());

  if(_useActualCost->isChecked())
    params.append("useActualCost");
  else
    params.append("useStandardCost");

  if(_useAveragePrice->isChecked())
  {
    params.append("useAveragePrice");
    params.append("startEvalDate", _startEvalDate->date());
    params.append("endEvalDate", _endEvalDate->date());
  }
  else
    params.append("useListPrice");

  orReport report("PlannedRevenueExpensesByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
  }

  if(_captive)
    accept();
}

