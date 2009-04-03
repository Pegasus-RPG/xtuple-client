/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createPlannedOrdersByPlannerCode.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "submitAction.h"
#include <metasql.h>
#include "mqlutil.h"

/*
 *  Constructs a createPlannedOrdersByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
createPlannedOrdersByPlannerCode::createPlannedOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  _plannerCode->setType(ParameterGroup::PlannerCode);
}

/*
 *  Destroys the object and frees any allocated resources
 */
createPlannedOrdersByPlannerCode::~createPlannedOrdersByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void createPlannedOrdersByPlannerCode::languageChange()
{
    retranslateUi(this);
}


void createPlannedOrdersByPlannerCode::sCreate()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("schedule", "create");
  q = mql.toQuery(params);
  q.exec();

  accept();
}


void createPlannedOrdersByPlannerCode::sSubmit()
{
  ParameterList params;
  if (! setParams(params))
    return;

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();
}

bool createPlannedOrdersByPlannerCode::setParams(ParameterList &pParams)
{
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cut Off Date"),
                          tr( "You must enter a valid Cut Off Date before\n"
                              "creating Planned Orders." ));
    _cutOffDate->setFocus();
    return false;
  }

  pParams.append("action_name", "RunMRP");

  pParams.append("MPS", QVariant(false));
  pParams.append("planningType", "M");
  _plannerCode->appendValue(pParams);
  _warehouse->appendValue(pParams); 
  
  pParams.append("cutOffDate", _cutOffDate->date());
  pParams.append("cutoff_offset", QDate::currentDate().daysTo(_cutOffDate->date()));
  pParams.append("deleteFirmed", QVariant(_deleteFirmed->isChecked()));

  return true;
}

