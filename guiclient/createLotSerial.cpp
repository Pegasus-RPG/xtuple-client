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

#include "createLotSerial.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

createLotSerial::createLotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));

  _qtyToAssign->setValidator(omfgThis->qtyVal());
  _serial = false;
  _itemsiteid = -1;
}

createLotSerial::~createLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void createLotSerial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createLotSerial::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemloc_series", &valid);
  if (valid)
    _itemlocSeries = param.toInt();

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _itemlocdistid = param.toInt();

    q.prepare( "SELECT item_fractional, itemsite_controlmethod, "
	       "       itemsite_id, itemsite_perishable "
               "FROM itemlocdist, itemsite, item "
               "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemlocdist_id=:itemlocdist_id) );" );
    q.bindValue(":itemlocdist_id", _itemlocdistid);
    q.exec();
    if (q.first())
    {
      if (q.value("itemsite_controlmethod").toString() == "S")
      {
	_serial = true;
        _qtyToAssign->setText("1");
        _qtyToAssign->setEnabled(FALSE);
      }
      else
	_serial = false;

      _itemsiteid = q.value("itemsite_id").toInt();
      _expiration->setEnabled(q.value("itemsite_perishable").toBool());
      _fractional = q.value("item_fractional").toBool();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void createLotSerial::sAssign()
{
  if (_lotSerial->text().contains(QRegExp("\\s")) &&
      QMessageBox::question(this, tr("Lot/Serial Number Contains Spaces"),
			    tr("<p>The Lot/Serial Number contains spaces. Do "
			       "you want to save it anyway?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    return;
  }

  if (_qtyToAssign->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity"),
                           tr("<p>You must enter a positive value to assign to "
			      "this Lot/Serial number.") );
    _qtyToAssign->setFocus();
    return;
  }

  if ( (_expiration->isEnabled()) && (!_expiration->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("<p>You must enter an expiration date to this "
			      "Perishable Lot/Serial number.") );
    _expiration->setFocus();
    return;
  }

  if (!_fractional)
  {
    if (_qtyToAssign->toDouble() != _qtyToAssign->text().toInt())
    {
      QMessageBox::critical( this, tr("Item is Non-Fractional"),
                             tr( "<p>The Item in question is not stored in "
				 "fractional quantities. You must enter a "
				 "whole value to assign to this Lot/Serial "
				 "number." ) );
      _qtyToAssign->setFocus();
      return;
    }
  }

  if (_serial)
  {
    q.prepare("SELECT COUNT(*) AS count "
	      "FROM lsdetail "
	      "WHERE ((lsdetail_itemsite_id=:itemsite_id)"
	      "  AND  (lsdetail_lotserial=:lotserial));");
    q.bindValue(":itemsite_id", _itemsiteid);
    q.bindValue(":lotserial", _lotSerial->text());
    q.exec();
    if (q.first())
    {
      if (q.value("count").toInt() > 0)
      {
	QMessageBox::critical(this, tr("Duplicate Serial Number"),
			      tr("This Serial Number has already been used "
			         "and cannot be reused."));

	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

  }

  q.exec("SELECT NEXTVAL('itemlocdist_itemlocdist_id_seq') AS itemlocdist_id;");
  if (!q.first() || q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
 
  int itemlocdistid = q.value("itemlocdist_id").toInt();

   q.prepare( "INSERT INTO itemlocdist "
              "( itemlocdist_id, itemlocdist_source_type, itemlocdist_source_id,"
              "  itemlocdist_itemsite_id, itemlocdist_lotserial, itemlocdist_expiration,"
              "  itemlocdist_qty, itemlocdist_series, itemlocdist_invhist_id ) "
              "SELECT :newItemlocdist_id, 'D', itemlocdist_id,"
              "       itemlocdist_itemsite_id, :lotSerialNumber, :itemsite_expiration,"
              "       :qtyToAssign, :itemlocdist_series, itemlocdist_invhist_id "
              "FROM itemlocdist "
              "WHERE (itemlocdist_id=:itemlocdist_id);" );

  if (_expiration->isEnabled())
    q.bindValue(":itemsite_expiration", _expiration->date());
  else
    q.bindValue(":itemsite_expiration", omfgThis->startOfTime());

  q.bindValue(":newItemlocdist_id", itemlocdistid);
  q.bindValue(":lotSerialNumber", _lotSerial->text());
  q.bindValue(":qtyToAssign", _qtyToAssign->toDouble());
  q.bindValue(":itemlocdist_series", _itemlocSeries);
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();

  q.prepare( "INSERT INTO lsdetail "
             "( lsdetail_itemsite_id, lsdetail_lotserial, lsdetail_created,"
             "  lsdetail_source_type, lsdetail_source_id, lsdetail_source_number ) "
             "SELECT itemlocdist_itemsite_id, :lotSerialNumber, CURRENT_TIMESTAMP,"
             "       'I', itemlocdist_id, '' "
             "FROM itemlocdist "
             "WHERE (itemlocdist_id=:itemlocdist_id);" );
  q.bindValue(":lotSerialNumber", _lotSerial->text());
  q.bindValue(":itemlocdist_id", itemlocdistid);
  q.exec();

  accept();
}
