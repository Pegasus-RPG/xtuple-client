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

#include "adjustmentTrans.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include <QSqlError>
#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a adjustmentTrans as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
adjustmentTrans::adjustmentTrans(QWidget* parent, Qt::WindowFlags fl)
    : QMainWindow(parent, fl)
{
  setupUi(this);

  (void)statusBar();

  _adjustmentTypeGroupInt = new QButtonGroup(this);

  // signals and slots connections
  connect(_item, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQOH(int)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_adjustmentTypeGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sPopulateQty()));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_absolute, SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->transQtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
adjustmentTrans::~adjustmentTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void adjustmentTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse adjustmentTrans::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      invhistid = -1;
  bool     noQty     = TRUE;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _captive = TRUE;

    _qty->setText(formatQty(param.toDouble()));
    _qty->setEnabled(FALSE);
    _afterQty->setText(formatQty(param.toDouble()));
    _absolute->setChecked(TRUE);
    _adjustmentTypeGroup->setEnabled(FALSE);

    noQty = FALSE;
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

      setCaption(tr("Enter Miscellaneous Adjustment"));
      _usernameLit->clear();
      _transactionDate->setEnabled(_privleges->check("AlterTransactionDates"));
      _transactionDate->setDate(omfgThis->dbDate());

      if (!_item->isValid())
        _item->setFocus();
      else if (noQty)
	_qty->setFocus();
      else
        _documentNum->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setCaption(tr("Miscellaneous Adjustment"));
      _transactionDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _adjustmentTypeGroup->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _post->hide();

      q.prepare( "SELECT invhist_itemsite_id, invhist_transdate,"
                 "       formatQty(invhist_invqty) AS f_transqty,"
                 "       formatQty(invhist_qoh_before) AS f_qohbefore,"
                 "       formatQty(invhist_qoh_after) AS f_qohafter,"
                 "       invhist_ordnumber, invhist_comments,"
                 "       invhist_user "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      q.bindValue(":invhist_id", invhistid);
      q.exec();
      if (q.first())
      {
        _transactionDate->setText(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setText(q.value("f_transqty"));
        _beforeQty->setText(q.value("f_qohbefore").toString());
        _afterQty->setText(q.value("f_qohafter").toString());
        _documentNum->setText(q.value("invhist_ordnumber"));
        _notes->setText(q.value("invhist_comments").toString());
        _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
      }
      else
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

      _close->setFocus();
    }
  }

  return NoError;
}

void adjustmentTrans::sPost()
{

  if (_qty->text().length() == 0)
  {
    QMessageBox::warning(  this, tr("Enter Adjustment Quantitiy"),
                           tr(  "You must enter a valid Adjustment Quantity before entering this\n"
                                "Adjustment Transaction."  ));
    _qty->setFocus();
    return;
  }
  
  double qty = 0.0;

  if (_absolute->isChecked())
    qty = (_qty->toDouble() - _cachedQOH);
  else if (_relative->isChecked())
    qty = _qty->toDouble();

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery tx;
  tx.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  q.prepare( "SELECT invAdjustment(itemsite_id, :qty, :docNumber, :comments) AS result "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":qty", qty);
  q.bindValue(":docNumber", _documentNum->text());
  q.bindValue(":comments", _notes->text());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at adjustmentTrans::%1, Item ID #%2, Warehouse ID #%3, Error #%4.")
                           .arg(__LINE__)
                           .arg(_item->id())
                           .arg(_warehouse->id())
                           .arg(q.value("result").toInt()) );
    }
    else
    {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == QDialog::Rejected)
      {
        QMessageBox::information( this, tr("Inventory Adjustment"), tr("Transaction Canceled") );
        rollback.exec();
        return;
      }

      tx.exec("COMMIT;");

      //Since everything accepted, post G/L transactions to trial balance if item location or lot serial distributions
      if (q.value("result").toInt() > 0)
      {   
        XSqlQuery post;
        post.prepare("SELECT postItemlocseries(:itemlocseries) AS result;");
        post.bindValue(":itemlocseries", q.value("result").toInt());
        post.exec();
        if (post.first())
          if (!post.value("result").toBool())
                QMessageBox::warning( this, tr("Inventory Adjustment"), 
            tr("There was an error posting the transaction.  Contact your administrator") );
        else if (post.lastError().type() != QSqlError::None)
        {
          systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }

      if (_captive)
        close();
      else
      {
        _close->setText(tr("&Close"));
        _item->setId(-1);
        _qty->clear();
        _documentNum->clear();
        _notes->clear();
        _beforeQty->clear();
        _afterQty->clear();
        _item->setFocus();
      }
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at adjustmentTrans::%1, Item Site ID #%2, Warehouse ID #%3.")
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_warehouse->id()) );
  }
}

void adjustmentTrans::sPopulateQOH(int pWarehousid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT itemsite_freeze, itemsite_qtyonhand,"
               "       formatQty(itemsite_qtyonhand) AS qoh "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id));" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    if (q.first())
    {
      _cachedQOH = q.value("itemsite_qtyonhand").toDouble();
      _beforeQty->setText(q.value("qoh").toString());

      if (q.value("itemsite_freeze").toBool())
        _absolute->setPaletteForegroundColor(QColor("red"));
      else
        _absolute->setPaletteForegroundColor(QColor("black"));
    }
    else
      _absolute->setPaletteForegroundColor(QColor("black"));

    sPopulateQty();
  }
}

void adjustmentTrans::sPopulateQty()
{
  if (_mode == cNew)
  {
    if (_qty->text().stripWhiteSpace().length())
    {
      if (_absolute->isChecked())
        _afterQty->setText(formatQty(_qty->toDouble()));
  
      else if (_relative->isChecked())
        _afterQty->setText(formatQty(_cachedQOH + _qty->toDouble()));
    }
    else
      _afterQty->clear();
  }
}

