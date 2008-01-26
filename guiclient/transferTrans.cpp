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

#include "transferTrans.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include <qstatusbar.h>
#include "distributeInventory.h"

/*
 *  Constructs a transferTrans as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
transferTrans::transferTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(newId(int)), _toWarehouse, SLOT(findItemsites(int)));
    connect(_fromWarehouse, SIGNAL(newID(int)), this, SLOT(sPopulateFromQty(int)));
    connect(_toWarehouse, SIGNAL(newID(int)), this, SLOT(sPopulateToQty(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQty(const QString&)));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _fromWarehouse, SLOT(setId(int)));
    connect(_item, SIGNAL(newId(int)), _fromWarehouse, SLOT(findItemsites(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
transferTrans::~transferTrans()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void transferTrans::languageChange()
{
    retranslateUi(this);
}


void transferTrans::init()
{
  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());
}

enum SetResponse transferTrans::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _fromWarehouse->setEnabled(FALSE);
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _captive = TRUE;

    _qty->setText(formatQty(param.toDouble()));
    _qty->setEnabled(FALSE);
  }

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Inter-Warehouse Transfer"));
      _usernameLit->clear();
      _transDate->setEnabled(_privleges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());

      if (!_item->isValid())
        _item->setFocus();
      else if (_qty->text().length() == 0)
	_qty->setFocus();
      else
        _documentNum->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setCaption(tr("Inter-Warehouse Transaction Information"));
      _transDate->setEnabled(FALSE);
      _item->setEnabled(FALSE);
      _toWarehouse->setEnabled(FALSE);
      _fromWarehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _post->hide();

      q.prepare( "SELECT invhist_itemsite_id, invhist_transdate,"
                 "       formatQty(abs(invhist_invqty)) AS transqty,"
                 "       CASE WHEN (invhist_invqty > 0) THEN formatQty(invhist_qoh_before)"
                 "            ELSE ''"
                 "       END AS qohbefore,"
                 "       CASE WHEN (invhist_invqty > 0) THEN formatQty(invhist_qoh_after)"
                 "            ELSE ''"
                 "       END AS qohafter,"
                 "       CASE WHEN (invhist_invqty > 0) THEN ''"
                 "            ELSE formatQty(invhist_qoh_before)"
                 "       END AS fromqohbefore,"
                 "       CASE WHEN (invhist_invqty > 0) THEN ''"
                 "            ELSE formatQty(invhist_qoh_after)"
                 "       END AS fromqohafter,"
                 "       invhist_docnumber, invhist_comments,"
                 "       CASE WHEN (invhist_invqty > 0) THEN itemsite_warehous_id"
                 "            ELSE invhist_xfer_warehous_id"
                 "       END AS toWarehouse,"
                 "       CASE WHEN (invhist_invqty > 0) THEN invhist_xfer_warehous_id"
                 "            ELSE itemsite_warehous_id"
                 "       END AS fromWarehouse,"
                 "       invhist_user"
                 "  FROM invhist, itemsite"
                 " WHERE ((invhist_itemsite_id=itemsite_id)"
                 "   AND  (invhist_id=:invhist_id)); " );
      q.bindValue(":invhist_id", invhistid);
      q.exec();
      if (q.first())
      {
        _transDate->setText(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setText(q.value("transqty"));
        _fromBeforeQty->setText(q.value("fromqohbefore").toString());
        _fromAfterQty->setText(q.value("fromqohafter").toString());
        _toBeforeQty->setText(q.value("qohbefore").toString());
        _toAfterQty->setText(q.value("qohafter").toString());
        _documentNum->setText(q.value("invhist_docnumber"));
        _notes->setText(q.value("invhist_comments").toString());
        _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
        _toWarehouse->setId(q.value("toWarehouse").toInt());
        _fromWarehouse->setId(q.value("fromWarehouse").toInt());
      }

      _close->setFocus();
    }
  }

  return NoError;
}

void transferTrans::sPost()
{
  double qty = QString(_qty->text()).toDouble();

  if (qty == 0)
  {
    QMessageBox::warning( this, tr("Invalid Transaction Quantity"),
                          tr( "You must enter a positive quantity to tranfer before\n"
                              "creating this Transaction") );
    _qty->setFocus();
    return;
  }

  if (_fromWarehouse->id() == _toWarehouse->id())
  {
    QMessageBox::warning( this, tr("Target Warehouse same as Source Warehouse"),
                          tr( "The Target Warehouse is the same as the Source Warehouse.\n"
                              "You must select a different Warehouse for each before\n"
                              "creating this Transaction") );
    _fromWarehouse->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare( "SELECT interWarehouseTransfer( :item_id, :from_warehous_id, :to_warehous_id,"
             "                               :qty, 'Misc', :docNumber, :comments ) AS result;");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":from_warehous_id", _fromWarehouse->id());
  q.bindValue(":to_warehous_id", _toWarehouse->id());
  q.bindValue(":qty", qty);
  q.bindValue(":docNumber", _documentNum->text());
  q.bindValue(":comments", _notes->text());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at transferTrans::%1, Item ID #%2, To Warehouse ID #%3, From Warehouse ID #%4, Error #%5.")
                         .arg(__LINE__)
                         .arg(_item->id())
                         .arg(_toWarehouse->id())
                         .arg(_fromWarehouse->id())
                         .arg(q.value("result").toInt()) );
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == QDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Transfer Transaction"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
      
      if (_captive)
        close();
      else
      {
        _close->setText(tr("&Close"));
        _item->setId(-1);
        _qty->clear();
        _documentNum->clear();
        _notes->clear();
        _fromBeforeQty->clear();
        _fromAfterQty->clear();
        _toBeforeQty->clear();
        _toAfterQty->clear();
        _item->setFocus();
      }
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at transferTrans::%1, Item Site ID #%2, To Warehouse ID #%3, From Warehouse #%4.")
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_toWarehouse->id())
                       .arg(_fromWarehouse->id()) );
    return;
  }
}

void transferTrans::sPopulateFromQty(int pWarehousid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT itemsite_qtyonhand,"
               "       formatQty(itemsite_qtyonhand) AS qoh "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    if (q.first())
    {
      _cachedFromBeforeQty = q.value("itemsite_qtyonhand").toDouble();
      _fromBeforeQty->setText(q.value("qoh").toString());

      if (_qty->text().length())
        _fromAfterQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble() - _qty->toDouble()));
    }
  }
}

void transferTrans::sPopulateToQty(int pWarehousid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT itemsite_qtyonhand,"
               "       formatQty(itemsite_qtyonhand) AS qoh "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    if (q.first())
    {
      _cachedToBeforeQty = q.value("itemsite_qtyonhand").toDouble();
      _toBeforeQty->setText(q.value("qoh").toString());

      if (_qty->text().length())
        _toAfterQty->setText(formatQty(q.value("itemsite_qtyonhand").toDouble() + _qty->toDouble()));
    }
  }
}

void transferTrans::sUpdateQty(const QString &pQty)
{
  if (_mode != cView)
  {
    _fromAfterQty->setText(formatQty(_cachedFromBeforeQty - pQty.toDouble()));
    _toAfterQty->setText(formatQty(_cachedToBeforeQty + pQty.toDouble()));
  }
}
