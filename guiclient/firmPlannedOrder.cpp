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

#include "firmPlannedOrder.h"

#include <QSqlError>
#include <QValidator>
#include <QVariant>

firmPlannedOrder::firmPlannedOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_firm, SIGNAL(clicked()), this, SLOT(sFirm()));
  _quantity->setValidator(omfgThis->qtyVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

firmPlannedOrder::~firmPlannedOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void firmPlannedOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse firmPlannedOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("planord_id", &valid);
  if (valid)
  {
    _planordid = param.toInt();
    _item->setReadOnly(TRUE);

    q.prepare( "SELECT planord.*,"
               "       itemsite_leadtime "
               "FROM planord JOIN itemsite ON (itemsite_id=itemsite_id) "
               "WHERE (planord_id=:planord_id);" );
    q.bindValue(":planord_id", _planordid);
    q.exec();
    if (q.first())
    {
      _item->setItemsiteid(q.value("planord_itemsite_id").toInt());
      _quantity->setDouble(q.value("planord_qty").toDouble());
      _dueDate->setDate(q.value("planord_duedate").toDate());
      _comments->setText(q.value("planord_comments").toString());
      _number = q.value("planord_number").toInt();
      _itemsiteid = q.value("planord_itemsite_id").toInt();
      _leadTime = q.value("itemsite_leadtime").toInt();
  
      if (q.value("planord_type").toString() == "P")
        _orderType->setText(tr("Purchase Order"));
      else if (q.value("planord_type").toString() == "W")
        _orderType->setText(tr("Work Order"));
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      reject();
    }
  }

  return NoError;
}

void firmPlannedOrder::sFirm()
{
  q.prepare( "SELECT deletePlannedOrder( :planord_id, true) AS result;" );
  q.bindValue(":planord_id", _planordid);
  q.exec();
  if (q.first())
  {
    bool result = q.value("result").toBool();
    if (! result)
    {
      systemError(this, tr("DeletePlannedOrder returned FALSE, indicating an "
                           "error occurred."),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT createPlannedOrder( :orderNumber, :itemsite_id, :qty, "
             "                           (DATE(:dueDate) - :leadTime), :dueDate) AS result;" );
  q.bindValue(":orderNumber", _number);
  q.bindValue(":itemsite_id", _itemsiteid);
  q.bindValue(":qty", _quantity->toDouble());
  q.bindValue(":dueDate", _dueDate->date());
  q.bindValue(":leadTime", _leadTime);
  q.exec();
  if (q.first())
  {
    double result = q.value("result").toDouble();
    if (result < 0.0)
    {
      systemError(this, tr("CreatePlannedOrder returned %, indicating an "
                           "error occurred.").arg(result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "UPDATE planord "
             "SET planord_comments=:planord_comments, planord_firm=TRUE "
             "WHERE (planord_number=:orderNumber);" );
  q.bindValue(":planord_comments", _comments->text());
  q.bindValue(":orderNumber", _number);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_planordid);
}
