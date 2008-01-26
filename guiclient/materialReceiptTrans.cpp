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

#include "materialReceiptTrans.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include <qstatusbar.h>
#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a materialReceiptTrans as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
materialReceiptTrans::materialReceiptTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_issueToWo, SIGNAL(toggled(bool)), _wo, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateQty()));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQty()));
    init();

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
materialReceiptTrans::~materialReceiptTrans()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void materialReceiptTrans::languageChange()
{
    retranslateUi(this);
}


void materialReceiptTrans::init()
{
  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());
  _wo->setType(cWoOpen | cWoExploded | cWoReleased | cWoIssued);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _item->setFocus();
}

enum SetResponse materialReceiptTrans::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      _invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    _invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Material Receipt"));
      _usernameLit->clear();
      _transDate->setDate(omfgThis->dbDate());

      connect(_qty, SIGNAL(textChanged(const QString &)), this, SLOT(sUpdateQty(const QString &)));
      connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQty()));
      connect(_issueToWo, SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setCaption(tr("Material Receipt"));
      _transDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _issueToWo->setEnabled(FALSE);
      _wo->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _post->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
 
      q.prepare( "SELECT invhist_itemsite_id, invhist_transdate,"
                 "       formatQty(invhist_invqty) AS invqty,"
                 "       formatQty(invhist_qoh_before) AS qohbefore,"
                 "       formatQty(invhist_qoh_after) AS qohafter,"
                 "       invhist_ordnumber, invhist_comments,"
                 "       invhist_ordtype, invhist_ordnumber, invhist_user "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      q.bindValue(":invhist_id", _invhistid);
      q.exec();
      if (q.first())
      {
        _transDate->setText(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setText(q.value("invqty"));
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

void materialReceiptTrans::sPost()
{
  if (!_item->isValid())
  {
    QMessageBox::warning(  this, tr("Select Item Number"),
                           tr(  "You must select an Item to receive before entering this\n"
                                "Material Receipt Transaction."  ));
    _item->setFocus();
    return;
  }

  if (_qty->text().length() == 0)
  {
    QMessageBox::warning(  this, tr("Enter Receipt Quantitiy"),
                           tr(  "You must enter a valid Receipt Quantity before entering this\n"
                                "Material Receipt Transaction."  ));
    _qty->setFocus();
    return;
  }
  
  if (_issueToWo->isChecked())
  {
    q.prepare( "SELECT womatl_id, womatl_issuemethod "
               "FROM womatl, wo, itemsite "
               "WHERE ( ( womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=wo_id)"
               " AND (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (wo_id=:wo_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
    {
      if ( (q.value("womatl_issuemethod").toString() == "S") ||
           (q.value("womatl_issuemethod").toString() == "M") )
      {
        int womatlid = q.value("womatl_id").toInt();

	q.prepare( "SELECT invReceiptIssueToWomatl(itemsite_id, :qty, :docNumber, :womatl_id, :comments) AS result "
                   "FROM itemsite "
                   "WHERE ( (itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id) );" );
        q.bindValue(":qty", _qty->toDouble());
        q.bindValue(":docNumber", _documentNum->text());
        q.bindValue(":womatl_id", womatlid);
        q.bindValue(":comments", _notes->text());
        q.bindValue(":item_id", _item->id());
        q.bindValue(":warehous_id", _warehouse->id());
        q.exec();

        if (_captive)
          close();
        else
        {
          _close->setText(tr("&Close"));
          _item->setId(-1);
          _qty->clear();
          _beforeQty->clear();
          _afterQty->clear();
          _documentNum->clear();
          _issueToWo->setChecked(FALSE);
          _wo->setId(-1);
          _notes->clear();
          
          _item->setFocus();
        }
      }
      else
        QMessageBox::critical( this, tr("Cannot Receive and Issue Material"),
                               tr( "The select Item may not be issued againt the selected W/O as the\n"
                                   "W/O Material Requirement Issue Method is Pull.  Material is issued\n"
                                   "to this W/O Material Requirement via a Backflush." ) );
    }
    else
      QMessageBox::critical( this, tr("Cannot Receive and Issue Material"),
                             tr( "The select Item may not be issued againt the selected W/O as there\n"
                                 "there isn't a W/O Material Requirement for the selected W/O/Item combination." ) );
  }
  else
  {
    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
    q.prepare( "SELECT invReceipt(itemsite_id, :qty, '', :docNumber, :comments) AS result "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":qty", _qty->toDouble());
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
        systemError( this, tr("A System Error occurred at materialReceiptTrans::%1, Item ID #%2, Warehouse ID #%3, Error #%4.")
                           .arg(__LINE__)
                           .arg(_item->id())
                           .arg(_warehouse->id())
                           .arg(q.value("result").toInt()) );
        return;
      }
      else if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == QDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Enter Receipt"), tr("Transaction Canceled") );
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
        _beforeQty->clear();
        _afterQty->clear();
        _documentNum->clear();
        _issueToWo->setChecked(FALSE);
        _wo->setId(-1);
        _notes->clear();
        _item->setFocus();
      }
    }
    else
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at materialReceiptTrans::%1, Item Site ID #%2, Warehouse ID #%3.")
                         .arg(__LINE__)
                         .arg(_item->id())
                         .arg(_warehouse->id()) );
      return;
    }
  }
}

void materialReceiptTrans::sPopulateQty()
{
  q.prepare( "SELECT itemsite_qtyonhand, formatQty(itemsite_qtyonhand) AS f_qoh "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    _cachedQOH = q.value("itemsite_qtyonhand").toDouble();
    _beforeQty->setText(q.value("f_qoh").toString());

    if (_issueToWo->isChecked())
      _afterQty->setText(q.value("f_qoh").toString());
    else if (_qty->text().length())
      _afterQty->setText(formatQty(_cachedQOH + _qty->toDouble()));
  }
//  ToDo

  _wo->setWarehouse(_warehouse->id());
}

void materialReceiptTrans::sUpdateQty(const QString &pQty)
{
  if (_issueToWo->isChecked())
    _afterQty->setText(_beforeQty->text());
  else
    _afterQty->setText(formatQty(_cachedQOH + pQty.toDouble()));
}
