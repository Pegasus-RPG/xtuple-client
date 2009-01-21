/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "firmPlannedOrdersByPlannerCode.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a firmPlannedOrdersByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
firmPlannedOrdersByPlannerCode::firmPlannedOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_firm, SIGNAL(clicked()), this, SLOT(sFirm()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
firmPlannedOrdersByPlannerCode::~firmPlannedOrdersByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void firmPlannedOrdersByPlannerCode::languageChange()
{
    retranslateUi(this);
}


void firmPlannedOrdersByPlannerCode::init()
{
  _plannerCode->setType(ParameterGroup::PlannerCode);
}

void firmPlannedOrdersByPlannerCode::sFirm()
{
  if (!_cutoffDate->isValid())
  {
    QMessageBox::critical( this, tr("Enter Cut Off Date"),
                           tr( "You must enter a cut off date for the Planned Orders\n"
                               "to be firmed." ) );
    _cutoffDate->setFocus();
    return;
  }

  QString sql( "UPDATE planord "
               "SET planord_firm=TRUE "
               "FROM itemsite "
               "WHERE ( (planord_itemsite_id=itemsite_id)"
               " AND (NOT planord_firm)"
               " AND (planord_startdate <= :cutOffDate)" );

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  sql += ");";

  q.prepare(sql);
  q.bindValue(":cutOffDate", _cutoffDate->date());
  _plannerCode->bindValue(q);
  q.exec();

  accept();
}

