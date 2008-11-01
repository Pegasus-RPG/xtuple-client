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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include <QSqlError>

#include <metasql.h>
#include <qvariant.h>
#include <parameter.h>

#include "updateReorderLevels.h"
#include "mqlutil.h"
#include "submitAction.h"

/*
 *  Constructs a updateReorderLevels as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateReorderLevels::updateReorderLevels(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    _daysGroupInt = new QButtonGroup(this);
    _daysGroupInt->addButton(_leadTime);
    _daysGroupInt->addButton(_fixedDays);

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_fixedDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
    connect(_leadTime, SIGNAL(toggled(bool)), _leadTimePad, SLOT(setEnabled(bool)));
    connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    connect(_preview, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
    
    _results->addColumn(tr("Site")            ,  _whsColumn,  Qt::AlignLeft,   true, "reordlvl_warehous_code");
    _results->addColumn(tr("Item Number")     , _itemColumn,  Qt::AlignLeft,   true, "reordlvl_item_number");
    _results->addColumn(tr("Description")     ,          -1,  Qt::AlignLeft,   true, "reordlvl_item_descrip");
    _results->addColumn(tr("Leadtime")        ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_leadtime");
    _results->addColumn(tr("Curr. Level")     ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_curr_level");
    _results->addColumn(tr("Days Stock")      ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_daysofstock");
    _results->addColumn(tr("Total Usage")     ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_total_usage");
    _results->addColumn(tr("New Level")       ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_calc_level");
    
    if (!_metrics->boolean("EnableBatchManager"))
      _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateReorderLevels::~updateReorderLevels()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateReorderLevels::languageChange()
{
    retranslateUi(this);
}


enum SetResponse updateReorderLevels::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("classcode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    setWindowTitle("Update Reorder Levels by Class Code");
  }

  param = pParams.value("plancode", &valid);
  if (valid)
  {
    setWindowTitle("Update Reorder Levels by Planner Code");
    _parameter->setType(ParameterGroup::PlannerCode);
  }

  param = pParams.value("item", &valid);
  if (valid)
  {
    setWindowTitle("Update Reorder Level by Item");
    _stack->setCurrentIndex(1);
  }

  return NoError;
}

bool updateReorderLevels::setParams(ParameterList &params)
{  
  if (_item->id() != -1)
    params.append("item_id", _item->id());
  else
    _parameter->appendValue(params);
  _warehouse->appendValue(params);
  
    if (_leadTime->isChecked())
    {
      params.append("addLeadtime"),
      params.append("daysOfStock", _leadTimePad->value());
    }
    else if (_fixedDays->isChecked())
      params.append("daysOfStock", _days->value());
  
  params.append("period_id_list",_periods->periodList());
  
  return true;
}

void updateReorderLevels::sUpdate()
{
  QString method;
  qDebug(_periods->periodString());
  if (_periods->topLevelItemCount() > 0)
  {
    QString sql;

    if (_preview->isChecked())
      method = "preview";
    else
      method = "update";

    ParameterList params;
    if (! setParams(params))
      return;

    MetaSQLQuery mql = mqlLoad("updateReorderLevels", method);
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
        
    if (_preview->isChecked())
    {
      if (q.first())
      {
        _totalDays->setText(q.value("reordlvl_total_days").toString());
        _results->populate(q, true);
        _tab->setCurrentIndex(1);
      }
    }
    else
      accept();
  }
    
    /*
      if (_leadTime->isChecked())
        sql = QString( "SELECT updateReorderLevel(itemsite_id, (itemsite_leadtime + :leadTimePad), '{%1}') AS result "
                       "FROM itemsite, plancode "
                       "WHERE ( (itemsite_plancode_id=plancode_id)" )
              .arg(_periods->periodString());

      else if (_fixedDays->isChecked())
        sql = QString( "SELECT updateReorderLevel(itemsite_id, :days, '{%1}') AS result "
                       "FROM itemsite, plancode "
                       "WHERE ( (itemsite_plancode_id=plancode_id)" )
              .arg(_periods->periodString());

      if (_warehouse->isSelected())
        sql += " AND (itemsite_warehous_id=:warehous_id)";

      if (_parameter->isSelected())
        sql += " AND (plancode_id=:plancode_id)";
      else if (_parameter->isPattern())
        sql += " AND (plancode_code ~ :plancode_pattern)";

      sql += ");";

      q.prepare(sql);
      q.bindValue(":leadTimePad", _leadTimePad->value());
      q.bindValue(":days", _days->value());
      _warehouse->bindValue(q);
      _parameter->bindValue(q);
      q.exec();*/
}

void updateReorderLevels::sSubmit()
{
  if (_periods->topLevelItemCount() > 0)
  {
    ParameterList params;
    params.append("action_name", "UpdateReorderLevel");
    params.append("period_id_list", _periods->periodString());
    _warehouse->appendValue(params);
    _parameter->appendValue(params);

    if (_leadTime->isChecked())
      params.append("leadtimepad", _leadTimePad->value());
    else if (_fixedDays->isChecked())
      params.append("fixedlookahead", _days->value());

    submitAction newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      accept();
  }
}

void updateReorderLevels::sHandleButtons()
{
  if (_preview->isChecked())
    _update->setText("C&alculate");
  else
    _update->setText("Up&date");
}


