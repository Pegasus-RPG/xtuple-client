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
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));

  _serial = false;
  _itemsiteid = -1;
  _lsdetailid = -1;
  _preassigned = false;
  resize(minimumSize());
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
	       "       itemsite_id, itemsite_perishable, itemsite_warrpurc, "
               "       invhist_ordtype, invhist_ordnumber "
               "FROM itemlocdist, itemsite, item, invhist "
               "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemlocdist_invhist_id=invhist_id) "
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
        _expiration->setEnabled(q.value("itemsite_perishable").toBool());
        _warranty->setEnabled(q.value("itemsite_warrpurc").toBool() && q.value("invhist_ordtype").toString() == "PO");
      }
      else
	_serial = false;

      _itemsiteid = q.value("itemsite_id").toInt();
      _expiration->setEnabled(q.value("itemsite_perishable").toBool());
      _warranty->setEnabled(q.value("itemsite_warrpurc").toBool() && q.value("invhist_ordtype").toString() == "PO");
      _fractional = q.value("item_fractional").toBool();
      
      //If there is preassigned trace info for an associated order, force user to select from list
      q.prepare("SELECT lsdetail_id, lsdetail_lotserial "
                "FROM lsdetail "
                "WHERE ( (lsdetail_source_number=:ordernumber) "
                "AND (lsdetail_source_type=:ordertype)"
                "AND (lsdetail_qtytoassign > 0) )");
      q.bindValue(":ordertype", q.value("invhist_ordtype").toString());
      q.bindValue(":ordernumber", q.value("invhist_ordnumber").toString());
      q.exec();
      if (q.first())
      {
        _lotSerial->setEditable(FALSE);
        _lotSerial->populate(q);
        _preassigned = true;
      }
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
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
  if (_lotSerial->currentText().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter Lot/Serial Number"),
                           tr("<p>You must enter a Lot/Serial number."));
    _lotSerial->setFocus();
    return;
  }
  else if (_lotSerial->currentText().contains(QRegExp("\\s")) &&
      QMessageBox::question(this, tr("Lot/Serial Number Contains Spaces"),
			    tr("<p>The Lot/Serial Number contains spaces. Do "
			       "you want to save it anyway?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    _lotSerial->setFocus();
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
  
  if ( (_warranty->isEnabled()) && (!_warranty->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Warranty Expire Date"),
                           tr("<p>You must enter a warranty expiration date for this "
			      "Lot/Serial number.") );
    _warranty->setFocus();
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
	q.prepare("SELECT COUNT(*) AS count FROM "
		  "(SELECT itemloc_id AS count "
		  "FROM itemloc "
		  "WHERE ((itemloc_itemsite_id=:itemsite_id)"
		  "  AND (itemloc_lotserial=:lotserial))"
		  "UNION "
		  "SELECT itemlocdist_id "
		  "FROM itemlocdist "
		  "WHERE ((itemlocdist_itemsite_id=:itemsite_id) "
		  "  AND (itemlocdist_lotserial=:lotserial) "
		  "  AND (itemlocdist_source_type='D'))) as data;");
    q.bindValue(":itemsite_id", _itemsiteid);
    q.bindValue(":lotserial", _lotSerial->currentText());
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
  
  q.prepare("SELECT lsdetail_id, lsdetail_created, formatdate(lsdetail_created) AS f_created, lsdetail_qtytoassign "
            "FROM lsdetail "
            "WHERE ( (lsdetail_lotserial=:lotserial) "
            "AND (lsdetail_itemsite_id=:itemsiteid) );");
  q.bindValue(":itemsiteid", _itemsiteid);
  q.bindValue(":lotserial", _lotSerial->currentText());
  q.exec();
  if (q.first())
  {
    if ( (_preassigned) && (_qtyToAssign->toDouble() > q.value("lsdetail_qtytoassign").toDouble()) )
    {
      QMessageBox::critical( this, tr("Invalid Qty"),
                           tr( "<p>The quantity being assigned is greater than the "
                           " quantity preassigned to the order being received." ) );
      return;
    }
    if (!_preassigned)
      if (QMessageBox::question(this, tr("Use Existing?"),
				  tr("<p>A record with this lot number already exists with a create date of %1.  "
                                  "Reference this lot?").arg(q.value("f_created").toString()),
				     QMessageBox::Yes | QMessageBox::Default,
				     QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
         _lsdetailid=q.value("lsdetail_id").toInt();;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (_preassigned)
  {
    QMessageBox::critical( this, tr("Lot/Serial not found"),
                           tr( "<p>The preassigned lot number was not found." ) );
    return;
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
              "  itemlocdist_qty, itemlocdist_series, itemlocdist_invhist_id, itemlocdist_warranty ) "
              "SELECT :newItemlocdist_id, 'D', itemlocdist_id,"
              "       itemlocdist_itemsite_id, :lotSerialNumber, :itemsite_expiration,"
              "       :qtyToAssign, :itemlocdist_series, itemlocdist_invhist_id, :itemlocdist_warranty "
              "FROM itemlocdist "
              "WHERE (itemlocdist_id=:itemlocdist_id);" );

  if (_expiration->isEnabled())
    q.bindValue(":itemsite_expiration", _expiration->date());
  else
    q.bindValue(":itemsite_expiration", omfgThis->startOfTime());

  q.bindValue(":newItemlocdist_id", itemlocdistid);
  q.bindValue(":lotSerialNumber", _lotSerial->currentText());
  q.bindValue(":qtyToAssign", _qtyToAssign->toDouble());
  q.bindValue(":itemlocdist_series", _itemlocSeries);
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  if (_warranty->isEnabled())
    q.bindValue(":itemlocdist_warranty", _warranty->date());
  q.exec();

  if (_preassigned)
  {
    q.prepare(  "UPDATE lsdetail SET "
                "  lsdetail_qtytoassign=lsdetail_qtytoassign-COALESCE(:qty,0) "
                "WHERE (lsdetail_id=:lsdetail_id);");
    q.bindValue(":qty",_qtyToAssign->toDouble());
    q.bindValue(":lsdetail_id", _lotSerial->id());
    q.exec();
  }

  else if (_lsdetailid == -1)
  {
    q.prepare( "INSERT INTO lsdetail "
               "( lsdetail_itemsite_id, lsdetail_lotserial, lsdetail_created,"
               "  lsdetail_source_type, lsdetail_source_id, lsdetail_source_number ) "
               "SELECT itemlocdist_itemsite_id, :lotSerialNumber, CURRENT_TIMESTAMP,"
               "       'I', itemlocdist_id, '' "
               "FROM itemlocdist "
               "WHERE (itemlocdist_id=:itemlocdist_id);" );
    q.bindValue(":lotSerialNumber", _lotSerial->currentText());
    q.bindValue(":itemlocdist_id", itemlocdistid);
    q.exec();
  }

  accept();
}
