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

#include "postMiscProduction.h"

#include <QMessageBox>
#include <QVariant>

#include "distributeInventory.h"
//#include "distributeBreederProduction.h"

postMiscProduction::postMiscProduction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _captive = FALSE;
  _itemsiteid = -1;
  _sense = 1;
  _qty = 0;

  _item->setType(ItemLineEdit::cManufactured);
  _qtyToPost->setValidator(omfgThis->qtyVal());
  
  _immediateTransfer->setEnabled(_privileges->check("CreateInterWarehouseTrans"));
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs")) 
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _immediateTransfer->hide();
    _transferWarehouse->hide();
  }
  else
    _transferWarehouse->setEnabled(_immediateTransfer->isChecked());

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backflush->setChecked(true);

  _nonPickItems->setEnabled(_backflush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();
}

postMiscProduction::~postMiscProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

void postMiscProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postMiscProduction::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _warehouse->setEnabled(FALSE);
    _item->setReadOnly(TRUE);

    _qtyToPost->setFocus();
  }

  return NoError;
}

void postMiscProduction::sPost()
{
  _qty = _qtyToPost->toDouble();
  if (_disassembly->isChecked())
    _qty = _qty * -1;
  
  if (_qty == 0)
  {
    QMessageBox::warning( this, tr("Invalid Quantity"),
                        tr( "The quantity may not be zero." ) );
    return;
  }
  
  if (_immediateTransfer->isChecked())
  {
    if (_warehouse->id() == _transferWarehouse->id())
    {
      QMessageBox::warning( this, tr("Cannot Post Immediate Transfer"),
                            tr( "Transaction canceled. Cannot post an immediate transfer for the newly posted production as the\n"
                                "transfer Site is the same as the production Site.  You must manually\n"
                                "transfer the production to the intended Site." ) );
      return;
    }
  }
  
  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  if (_qty > 0)
    q.bindValue(":warehous_id", _warehouse->id());
  else
    q.bindValue(":warehous_id", _transferWarehouse->id());
  q.exec();
  if (q.first())
  {
    _itemsiteid = q.value("itemsite_id").toInt();

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
    
    if (_qty > 0)
    {
      if (post())
      {
        if (_immediateTransfer->isChecked())
        {
          if (transfer())
            q.exec("COMMIT;");
          else
            rollback.exec();
        }
        else
          q.exec("COMMIT;");
      }
      else
        rollback.exec();
    }
    else
    {
      _sense = -1;
      if (_immediateTransfer->isChecked())
      {
        if (transfer())
        {
          if (post())
            q.exec("COMMIT;");
          else
            rollback.exec();
        }
        else
          rollback.exec();
      }
      else
      {
        if (post())
          q.exec("COMMIT;");
        else
          rollback.exec();
      }
    }

    if (_captive)
      accept();
    else
    {
      _item->setId(-1);
      _qtyToPost->clear();
      _documentNum->clear();
      _comments->clear();
      _close->setText(tr("&Close"));

      _item->setFocus();
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2, Item Number %3.")
                      .arg(__FILE__)
                      .arg(__LINE__)
                      .arg(_item->itemNumber()) );
}

bool postMiscProduction::post()
{
  q.prepare( "SELECT postMiscProduction( :itemsite_id, :qty, :backflushMaterials,"
             "                           :docNumber, :comments ) AS result;" );
  q.bindValue(":itemsite_id", _itemsiteid);
  q.bindValue(":qty", _qty);
  q.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  q.bindValue(":docNumber", _documentNum->text().trimmed());
  q.bindValue(":comments", _comments->toPlainText());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      systemError(this, tr("A System Error occurred at %1::%2, Item Number %3, Error %4.")
                        .arg(__FILE__)
                        .arg(__LINE__)
                        .arg(_item->itemNumber())
                        .arg(q.value("result").toInt()) );
      return false;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
        return false;
      }
    }
  }
  return true;
}

bool postMiscProduction::transfer()
{
  q.prepare( "SELECT interWarehouseTransfer( :item_id, :from_warehous_id, :to_warehous_id,"
             "                               :qty, 'W', :documentNumber, 'Transfer from Misc. Production Posting' ) AS result;" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":from_warehous_id", _warehouse->id());
  q.bindValue(":to_warehous_id", _transferWarehouse->id());
  q.bindValue(":qty", _qty * _sense);
  q.bindValue(":documentNumber", _documentNum->text().trimmed());
  q.exec();
  if (q.first())
  {
    if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
      return false;
    }
  }
  else
  {
    systemError( this, tr("A System Error occurred at interWarehousTransfer::%1, Item Site ID #%2, Site ID #%3 to Site ID #%4.")
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_warehouse->id())
                       .arg(_transferWarehouse->id()));
    return false;
  }
  return true;
}



