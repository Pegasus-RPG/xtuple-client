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

#include "transformTrans.h"

#include <QVariant>
#include <QValidator>
#include "distributeInventory.h"

/*
 *  Constructs a transformTrans as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
transformTrans::transformTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_item, SIGNAL(privateIdChanged(int)), _warehouse, SLOT(findItemsites(int)));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));
    connect(_target, SIGNAL(newID(int)), this, SLOT(sPopulateTarget(int)));
    connect(_source, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
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
transformTrans::~transformTrans()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void transformTrans::languageChange()
{
    retranslateUi(this);
}


void transformTrans::init()
{
  _captive = FALSE;

  _qty->setValidator(omfgThis->qtyVal());

  _source->addColumn( tr("Location"),     _itemColumn, Qt::AlignLeft  );
  _source->addColumn( tr("Lot/Serial #"), -1,          Qt::AlignLeft  );
  _source->addColumn( tr("Qty."),         _qtyColumn,  Qt::AlignRight );

  _item->setFocus();
}

enum SetResponse transformTrans::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;
  //int      invhistid = -1;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setCaption(tr("Enter Transform Transaction"));
    }
  }

  return NoError;
}


void transformTrans::sPost()
{
  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Post Transform Transaction"),
                           tr("You must enter a quantity to transform before you may post this Transform Transaction.") );
    _qty->setFocus();
    return;
  }

  if (_qty->toDouble() > _source->currentItem()->text(2).toDouble())
  {
    QMessageBox::critical( this, tr("Cannot Post Transform Transaction"),
                           tr("You may not transform a quantity that is greater that the quantity of the Transform Source.") );
    _qty->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare( "SELECT postTransformTrans(s.itemsite_id, t.itemsite_id, :itemloc_id, :qty, :docnumber, :comments) AS result "
             "FROM itemsite AS s, itemsite AS t "
             "WHERE ( (s.itemsite_warehous_id=t.itemsite_warehous_id)"
             " AND (s.itemsite_warehous_id=:warehous_id)"
             " AND (s.itemsite_item_id=:sourceItemid)"
             " AND (t.itemsite_item_id=:targetItemid) );" );
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":sourceItemid", _item->id());
  q.bindValue(":targetItemid", _target->id());
  q.bindValue(":itemloc_id", _source->altId());
  q.bindValue(":qty", _qty->toDouble());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    qDebug(QString("result=%1").arg(result));

    if (result < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(result) );
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Transform Transaction"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
    }
  }

  if (_captive)
    close();
}

void transformTrans::sPopulateTarget(int pItemid)
{
  q.prepare( "SELECT item_descrip1, item_descrip2 "
             "FROM item "
             "WHERE (item_id=:item_id);" );
  q.bindValue(":item_id", pItemid);
  q.exec();
  if (q.first())
  {
    _descrip1->setText(q.value("item_descrip1").toString());
    _descrip2->setText(q.value("item_descrip2").toString());
  }
}

void transformTrans::sFillList()
{
  _source->clear();

  q.prepare( "SELECT item_id, item_number "
             "FROM item, itemsite, itemtrans "
             "WHERE ( (itemtrans_target_item_id=item_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=:warehous_id)"
             " AND (itemtrans_source_item_id=:item_id) ) "
             "ORDER BY item_number;" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _target->populate(q);

  q.prepare( "SELECT itemsite_id,"
             "       ( (itemsite_loccntrl) OR (itemsite_controlmethod IN ('L', 'S')) ) AS detail "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  { 
    int itemsiteid = q.value("itemsite_id").toInt();

    if (q.value("detail").toBool())
      q.prepare( "SELECT itemloc_itemsite_id AS itemsiteid, itemloc_id AS itemlocid,"
                 "       CASE WHEN (location_id IS NULL) THEN :na"
                 "            ELSE formatLocationName(location_id)"
                 "       END AS locationname,"
                 "       ls_number AS lotserial,"
                 "       itemloc_qty AS qty "
                 "FROM itemloc "
                 "  LEFT OUTER JOIN location ON (itemloc_location_id=location_id) "
                 "  LEFT OUTER JOIN ls ON (itemloc_ls_id=ls_id)"
                 "WHERE ( (itemloc_qty > 0)"
                 " AND (itemloc_itemsite_id=:itemsite_id) );" ); 
    else
      q.prepare( "SELECT itemsite_id AS itemsiteid, -1 AS itemlocid,"
                 "       TEXT(:na) AS locationname,"
                 "       TEXT(:na) AS lotserial,"
                 "       itemsite_qtyonhand AS qty "
                 "FROM itemsite "
                 "WHERE ( (itemsite_qtyonhand > 0)"
                 " AND (itemsite_id=:itemsite_id) );" );

    q.bindValue(":na", tr("N/A"));
    q.bindValue(":itemsite_id", itemsiteid);
    q.exec();
    XTreeWidgetItem *last = 0;
    while (q.next())
      last = new XTreeWidgetItem( _source, last,
				 q.value("itemsiteid").toInt(),
				 q.value("itemlocid").toInt(),
				 q.value("locationname"), q.value("lotserial"),
				 formatQty(q.value("qty").toDouble()) );
  }
}
