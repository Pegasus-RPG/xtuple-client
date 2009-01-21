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
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cut Off Date"),
                          tr( "You must enter a valid Cut Off Date before\n"
                              "creating Planned Orders." ));
    _cutOffDate->setFocus();
    return;
  }

  QString sql( "SELECT createPlannedOrders(itemsite_id, :cutOffDate, :deleteFirmed, FALSE) "
               "FROM (SELECT CASE WHEN(item_type='M') THEN 1 "
               "                  WHEN(item_type='F') THEN 2 "
               "                  ELSE 3 "
               "             END AS orderid, "
               "             bomLevelByItem(item_id) AS levelid, "
               "             itemsite_id "
               "        FROM itemsite, item, plancode "
               "       WHERE ( (itemsite_plancode_id=plancode_id)"
               "         AND (itemsite_active)"
               "         AND (item_active)"
               "         AND (item_planning_type='M')"
               "         AND (itemsite_item_id=item_id) ");

  if (_plannerCode->isSelected())
    sql += " AND (plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (plancode_code ~ :plancode_pattern)";


  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY orderid, levelid) AS data; ";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.bindValue(":cutOffDate", _cutOffDate->date());
  q.bindValue(":deleteFirmed", QVariant(_deleteFirmed->isChecked()));
  q.exec();

  accept();
}


void createPlannedOrdersByPlannerCode::sSubmit()
{
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cut Off Date"),
                          tr( "You must enter a valid Cut Off Date before\n"
                              "creating Planned Orders." ));
    _cutOffDate->setFocus();
    return;
  }

  ParameterList params;

  params.append("action_name", "RunMRP");

  _plannerCode->appendValue(params);
  _warehouse->appendValue(params); 
  
  params.append("cutoff_offset", QDate::currentDate().daysTo(_cutOffDate->date()));
  
  if (_deleteFirmed->isChecked())
    params.append("DeleteFirmed");

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();

}
