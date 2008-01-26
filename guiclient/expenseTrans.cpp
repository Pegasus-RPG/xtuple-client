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

#include "expenseTrans.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include <QStatusBar>
#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a expenseTrans as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
expenseTrans::expenseTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQOH(int)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));

  _captive = FALSE;

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());
  
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
expenseTrans::~expenseTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void expenseTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse expenseTrans::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Expense Transaction"));
      _usernameLit->clear();
      _transDate->setEnabled(_privleges->check("SetTansactionDates"));
      _transDate->setDate(omfgThis->dbDate());

      _item->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
 
      setCaption(tr("Expense Transaction"));
      _transDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _post->hide();
      _expcat->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _close->setFocus();
    
      q.prepare( "SELECT invhist_itemsite_id, invhist_transdate, "
                 "       formatQty(invhist_invqty) AS transqty,"
                 "       formatQty(invhist_qoh_before) AS qohbefore,"
                 "       formatQty(invhist_qoh_after) AS qohafter,"
                 "       invhist_ordnumber, invhist_comments,"
                 "       invhist_transtype, invhist_ordnumber, invhist_user "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      q.bindValue(":invhist_id", invhistid);
      q.exec();
      if (q.first())
      {
        _transDate->setText(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setText(q.value("transqty"));
        _beforeQty->setText(q.value("qohbefore").toString());
        _afterQty->setText(q.value("qohafter").toString());
        _documentNum->setText(q.value("invhist_ordnumber"));
        _notes->setText(q.value("invhist_comments").toString());
        _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
      }
    }
  }

  return NoError;
}

void expenseTrans::sPost()
{
  if (_qty->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Post Transaction"),
                          tr("You must enter a valid Quantity before posting this Expense Transaction.") );
    _qty->setFocus();
    return;
  }
  
  if (!_expcat->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Post Transaction"),
                          tr("You must select an Expense Category before posting this Expense Transaction.") );
    _qty->setFocus();
    return;
  }
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare( "SELECT invExpense(itemsite_id, :qty, :expcatid, :docNumber, :comments) AS result "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":expcatid", _expcat->id());
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
      systemError( this, tr("A System Error occurred at %1::%2, Item ID #%3, Warehouse ID #%4, Error #%5.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(_item->id())
                         .arg(_warehouse->id())
                         .arg(q.value("result").toInt()) );
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == QDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Expense Transaction"), tr("Transaction Canceled") );
        return;
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
    systemError( this, tr("A System Error occurred at %1::%2, Item Site ID #%3, Warehouse ID #%4.")
                       .arg(__FILE__)
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_warehouse->id()) );
  }
}

void expenseTrans::sPopulateQOH(int pWarehousid)
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
      _cachedQOH = q.value("itemsite_qtyonhand").toDouble();
      _beforeQty->setText(q.value("qoh").toString());
      sPopulateQty();
    }
  }
}

void expenseTrans::sPopulateQty()
{
  _afterQty->setText(formatQty(_cachedQOH - _qty->toDouble()));
}

