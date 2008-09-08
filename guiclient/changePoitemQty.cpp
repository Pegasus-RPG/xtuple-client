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

#include "changePoitemQty.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

changePoitemQty::changePoitemQty(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_change, SIGNAL(clicked()), this, SLOT(sChangeQty()));
  connect(_newQty, SIGNAL(lostFocus()), this, SLOT(sQtyChanged()));
  connect(_po, SIGNAL(newId(int)), this, SLOT(sPopulatePoitem(int)));
  connect(_poitem, SIGNAL(newID(int)), this, SLOT(sPopulate(int)));

  _captive = FALSE;
  _cacheFreight = 0.0;

  _po->setType(cPOUnposted | cPOOpen);
  _commentGroup->setEnabled(_postComment->isChecked());
  
  _newQty->setValidator(omfgThis->qtyVal());
  _newQtyReceived->setPrecision(omfgThis->qtyVal());
  _currentQtyOrdered->setPrecision(omfgThis->qtyVal());
  _currentQtyReceived->setPrecision(omfgThis->qtyVal());
  _currentQtyBalance->setPrecision(omfgThis->qtyVal());
  _newQtyBalance->setPrecision(omfgThis->qtyVal());

  _cmnttype->setType(XComboBox::AllCommentTypes);
}

changePoitemQty::~changePoitemQty()
{
  // no need to delete child widgets, Qt does it all for us
}

void changePoitemQty::languageChange()
{
  retranslateUi(this);
}

enum SetResponse changePoitemQty::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _po->setReadOnly(TRUE);
    _newQty->setFocus();
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT poitem_pohead_id "
               "FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    q.bindValue(":poitem_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _po->setId(q.value("poitem_pohead_id").toInt());
      _po->setReadOnly(TRUE);
      _poitem->setId(param.toInt());
    }

    _newQty->setFocus();
  }

  param = pParams.value("newQty", &valid);
  if (valid)
    _newQty->setDouble(param.toDouble());

  return NoError;
}

void changePoitemQty::sPopulatePoitem(int pPoheadid)
{
  q.prepare( "SELECT poitem_id,"
             "       ( poitem_linenumber || '-' ||"
             "         COALESCE(item_number, poitem_vend_item_number) || ' (' ||"
             "         COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) || ')' ) "
             "FROM poitem LEFT OUTER JOIN "
             "     ( itemsite JOIN item "
             "       ON (itemsite_item_id=item_id)"
             "     ) ON (poitem_itemsite_id=itemsite_id) "
             "WHERE ( (poitem_status <> 'C')"
             " AND (poitem_pohead_id=:pohead_id) ) "
             "ORDER BY poitem_linenumber;" );
  q.bindValue(":pohead_id", pPoheadid);
  q.exec();
  _poitem->populate(q);
}

void changePoitemQty::sPopulate(int pPoitemid)
{
  if (pPoitemid == -1)
  {
    _currentQtyReceived->clear();
    _newQtyReceived->clear();
    _currentQtyOrdered->clear();
    _newQty->clear();
    _currentQtyBalance->clear();
    _newQtyBalance->clear();
    _freight->clear();
  }
  else
  {
    q.prepare( "SELECT poitem_qty_ordered, poitem_qty_received,"
               "        poitem_qty_returned, poitem_freight, pohead_curr_id,"
               "        CURRENT_DATE AS date "
               "FROM poitem, pohead "
               "WHERE ((poitem_pohead_id=pohead_id)"
               "  AND  (poitem_id=:poitem_id));" );
    q.bindValue(":poitem_id", pPoitemid);
    q.exec();
    if (q.first())
    {
      _currentQtyOrdered->setDouble(q.value("poitem_qty_ordered").toDouble());
      _currentQtyReceived->setDouble(q.value("poitem_qty_received").toDouble());
      _currentQtyBalance->setDouble(q.value("poitem_qty_ordered").toDouble() - q.value("poitem_qty_received").toDouble());
      _newQtyReceived->setDouble(q.value("poitem_qty_received").toDouble());
      _cacheFreight = q.value("poitem_freight").toDouble();
      _freight->set(q.value("poitem_freight").toDouble(),
                    q.value("pohead_curr_id").toInt(),
                    q.value("date").toDate());
      sQtyChanged();
    }
  }
}

void changePoitemQty::sChangeQty()
{
  if (_newQty->toDouble() <= 0 &&
      QMessageBox::question(this, tr("Quantity 0?"),
			    tr("<p>You have changed the quantity to 0 or less. "
			       "Are you sure you want to do this?"),
			    QMessageBox::No | QMessageBox::Default,
			    QMessageBox::Yes) == QMessageBox::No)
    return;

  q.prepare("SELECT changePoitemQty(:poitem_id, :qty) AS result;");
  q.bindValue(":poitem_id", _poitem->id());
  q.bindValue(":qty", _newQty->toDouble());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("changePoitemQty", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_postComment->isChecked())
  {
    q.prepare("SELECT postComment(:cmnttype_id, 'PI', :poitem_id, :comment) AS result");
    q.bindValue(":cmnttype_id", _cmnttype->id());
    q.bindValue(":poitem_id", _poitem->id());
    q.bindValue(":comment", _comment->text());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("postcomment", result), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_freight->localValue() != _cacheFreight)
  {
    q.prepare("UPDATE poitem SET poitem_freight=:poitem_freight"
              " WHERE (poitem_id=:poitem_id); ");
    q.bindValue(":poitem_id", _poitem->id());
    q.bindValue(":poitem_freight", _freight->localValue());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  omfgThis->sPurchaseOrdersUpdated(_po->id(), TRUE);
  
  accept();
}

void changePoitemQty::sQtyChanged()
{
  double qtyBalance = (_newQty->toDouble() - _newQtyReceived->toDouble());
  if (qtyBalance < 0)
    qtyBalance = 0;

  _newQtyBalance->setDouble(qtyBalance);
}
