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

#include "userEventNotification.h"

#include <QSqlError>
#include <QVariant>

userEventNotification::userEventNotification(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_warehouse, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(sWarehouseToggled(QTreeWidgetItem*)));
  connect(_event, SIGNAL(itemSelected(int)), this, SLOT(sAllWarehousesToggled(int)));
  connect(_event, SIGNAL(itemSelectionChanged()), this, SLOT(sFillWarehouseList()));

  _event->addColumn(tr("Module"),      50,   Qt::AlignCenter );
  _event->addColumn(tr("Name"),        150,  Qt::AlignLeft   );
  _event->addColumn(tr("Description"), -1,   Qt::AlignLeft   );
  _event->populate( "SELECT evnttype_id, evnttype_module, evnttype_name, evnttype_descrip "
                    "FROM evnttype "
                    "ORDER BY evnttype_module, evnttype_name" );

  _warehouse->addColumn(tr("Notify"),    50,         Qt::AlignCenter );
  _warehouse->addColumn(tr("Whs."),      _whsColumn, Qt::AlignCenter );
  _warehouse->addColumn(tr("Warehouse"), -1,         Qt::AlignLeft   );
  _warehouse->populate( "SELECT warehous_id, TEXT('-'), warehous_code, warehous_descrip "
                        "FROM warehous "
                        "ORDER BY warehous_code" );
}

userEventNotification::~userEventNotification()
{
  // no need to delete child widgets, Qt does it all for us
}

void userEventNotification::languageChange()
{
  retranslateUi(this);
}

enum SetResponse userEventNotification::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _cUsername = param.toString();
    q.prepare( "SELECT usr_propername "
               "FROM usr "
               "WHERE (usr_username=:usr_username);" );
    q.bindValue(":usr_username", _cUsername);
    q.exec();
    if (q.first())
    {
      _username->setText(_cUsername);
      _properName->setText(q.value("usr_propername").toString());

      sFillWarehouseList();
    }
  }

  return NoError;
}

void userEventNotification::sAllWarehousesToggled(int pEvnttypeid)
{
  if(!_warehouse->topLevelItemCount() > 0)
    return;

  if (_warehouse->topLevelItem(0)->text(0) == tr("Yes"))
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id) );" );
  else
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id) ); "
               "INSERT INTO evntnot "
               "(evntnot_username, evntnot_evnttype_id, evntnot_warehous_id) "
               "SELECT :username, :evnttype_id, warehous_id "
               "FROM warehous;" );

  q.bindValue(":username", _cUsername);
  q.bindValue(":evnttype_id", pEvnttypeid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillWarehouseList();
}

void userEventNotification::sWarehouseToggled(QTreeWidgetItem *selected)
{
  if(!selected)
    return;

  if (selected->text(0) == tr("Yes"))
    q.prepare( "DELETE FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_evnttype_id=:evnttype_id)"
               " AND (evntnot_warehous_id=:warehous_id) );" );
  else
    q.prepare( "INSERT INTO evntnot "
               "(evntnot_username, evntnot_evnttype_id, evntnot_warehous_id) "
               "VALUES "
               "(:username, :evnttype_id, :warehous_id);" );

  q.bindValue(":username", _cUsername);
  q.bindValue(":evnttype_id", _event->id());
  q.bindValue(":warehous_id", ((XTreeWidgetItem *)selected)->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillWarehouseList();
}

void userEventNotification::sFillWarehouseList()
{
  for (int i = 0; i < _warehouse->topLevelItemCount(); i++)
  {
    q.prepare( "SELECT evntnot_id "
               "FROM evntnot "
               "WHERE ( (evntnot_username=:username)"
               " AND (evntnot_warehous_id=:warehous_id)"
               " AND (evntnot_evnttype_id=:evnttype_id) );" );
    q.bindValue(":username", _cUsername);
    q.bindValue(":warehous_id", ((XTreeWidgetItem *)(_warehouse->topLevelItem(i)))->id());
    q.bindValue(":evnttype_id", _event->id());
    q.exec();
    if (q.first())
      _warehouse->topLevelItem(i)->setText(0, tr("Yes"));
    else
      _warehouse->topLevelItem(i)->setText(0, tr("No"));
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
