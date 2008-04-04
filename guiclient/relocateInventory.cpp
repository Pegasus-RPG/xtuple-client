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

#include "relocateInventory.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "inputManager.h"

/*
 *  Constructs a relocateInventory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
relocateInventory::relocateInventory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_item, SIGNAL(valid(bool)), _move, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));
    connect(_move, SIGNAL(clicked()), this, SLOT(sMove()));
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
relocateInventory::~relocateInventory()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void relocateInventory::languageChange()
{
    retranslateUi(this);
}


void relocateInventory::init()
{
  _captive = FALSE;

  _item->setType(ItemLineEdit::cLocationControlled);
  _qty->setValidator(omfgThis->transQtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _source->addColumn(tr("Location"),     _itemColumn, Qt::AlignLeft  );
  _source->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft  );
  _source->addColumn(tr("Qty."),         _qtyColumn,  Qt::AlignRight );

  _target->addColumn(tr("Location"), -1,         Qt::AlignLeft  );
  _target->addColumn(tr("Qty."),     _qtyColumn, Qt::AlignRight );
}

enum SetResponse relocateInventory::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
    _source->setFocus();
  }

  int locid;
  param = pParams.value("source_location_id", &valid);
  if (valid)
  {
    locid = param.toInt();
    for (int i = 0; i < _source->topLevelItemCount(); i++)
    {
      XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
      if (cursor->altId() == locid)
      {
        _source->setCurrentItem(cursor);
        _source->scrollToItem(cursor);
        _source->setEnabled(false);
      }
    }
  }

  param = pParams.value("target_location_id", &valid);
  if (valid)
  {
    locid = param.toInt();
    for (int i = 0; i < _source->topLevelItemCount(); i++)
    {
      XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
      if (cursor->id() == locid)
      {
        _target->setCurrentItem(cursor);
        _target->scrollToItem(cursor);
        _target->setEnabled(false);
      }
    }
  }

  param = pParams.value("itemloc_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    XSqlQuery itemsiteid;
    itemsiteid.prepare( "SELECT itemloc_itemsite_id "
                        "FROM itemloc "
                        "WHERE (itemloc_id=:itemloc_id);" );
    itemsiteid.bindValue(":itemloc_id", param.toInt());
    itemsiteid.exec();
    if (itemsiteid.first())
    {
      _item->setItemsiteid(itemsiteid.value("itemloc_itemsite_id").toInt());
      _item->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);

      for (int i = 0; i < _source->topLevelItemCount(); i++)
      {
	XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
        if (cursor->id() == param.toInt())
        {
          _source->setCurrentItem(cursor);
          _source->scrollToItem(cursor);
        }
      }

      _source->setEnabled(FALSE);
    }
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _qty->setText(param.toString());
    _qty->setEnabled(false);
  }

  return NoError;
}

void relocateInventory::sMove()
{
  if (_source->currentItem() == 0)
  {
    QMessageBox::critical( this, tr("Select Source Location"),
                           tr( "You must select a Source Location before\n"
                               "relocating Inventory." ) );
    _source->setFocus();
    return;
  }

  if (_target->currentItem() == 0)
  {
    QMessageBox::critical( this, tr("Select Target Location"),
                           tr( "You must select a Target Location before\n"
                               "relocating Inventory." ) );
    _target->setFocus();
    return;
  }

  if (((XTreeWidgetItem *)_source->currentItem())->id() == ((XTreeWidgetItem *)_target->currentItem())->id())
  {
    QMessageBox::critical( this, tr("Save Source and Target Location"),
                           tr( "You have selected the same Location for both\n"
                               "the Source and the Target." ) );
    return;
  }

  if (_qty->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Enter Quantity to Relocate"),
                           tr("You must enter a quantity to relocate.") );
    _qty->setFocus();
    return;
  }

  XSqlQuery relocate;
  relocate.prepare( "SELECT relocateInventory(:source, :target, itemsite_id, :qty, :comments) AS result "
                    "FROM itemsite "
                    "WHERE ( (itemsite_item_id=:item_id)"
                    " AND (itemsite_warehous_id=:warehous_id));" );
  relocate.bindValue(":source", _source->id());
  relocate.bindValue(":target", _target->id());
  relocate.bindValue(":qty", _qty->toDouble());
  relocate.bindValue(":comments", _notes->text().stripWhiteSpace());
  relocate.bindValue(":item_id", _item->id());
  relocate.bindValue(":warehous_id", _warehouse->id());
  relocate.exec();

  if(relocate.first() && relocate.value("result").toInt() < 0)
  {
    int result = relocate.value("result").toInt();
    switch(result)
    {
      case -1:
        QMessageBox::warning(this, tr("Not Enough Qty."), tr("You cannot Relocate more inventory than available."));
        break;
      default:
        QMessageBox::warning(this, tr("Unknown Error"), tr("There was an unknown error encountered while trying to relocate this inventory."));
    };
  }
  else if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    sFillList();
    _qty->clear();
    _notes->clear();
    _item->setFocus();
  }
}

void relocateInventory::sFillList()
{
  if (_item->isValid())
  {
    XSqlQuery query;
    query.prepare( "SELECT itemloc_id, itemloc_location_id,"
                   "       CASE WHEN (itemloc_location_id=-1) THEN :undefined"
                   "            ELSE formatLocationName(itemloc_location_id)"
                   "       END AS locationname,"
                   "       formatlotserialnumber(itemloc_ls_id), formatQty(itemloc_qty) "
                   "FROM itemloc, itemsite "
                   "WHERE ( (itemloc_itemsite_id=itemsite_id)"
                   " AND (itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id) ) "
                   "ORDER BY locationname;" );
    query.bindValue(":undefined", tr("Undefined"));
    query.bindValue(":item_id", _item->id());
    query.bindValue(":warehous_id", _warehouse->id());
    query.exec();
    _source->populate(query, true);

    query.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname,"
                   "       formatQty( ( SELECT COALESCE(SUM(itemloc_qty), 0)"
                   "                    FROM itemloc, itemsite"
                   "                    WHERE ( (itemloc_location_id=location_id)"
                   "                     AND (itemloc_itemsite_id=itemsite_id)"
                   "                     AND (itemsite_item_id=:item_id)"
                   "                     AND (itemsite_warehous_id=location_warehous_id) ) ) ) "
                   "FROM location, itemsite "
                   "WHERE ( (itemsite_warehous_id=:warehous_id)"
                   " AND (location_warehous_id=:warehous_id)"
                   " AND (itemsite_item_id=:item_id)"
                   " AND  (validLocation(location_id, itemsite_id)) ) "
                   "ORDER BY locationname;" );
    query.bindValue(":warehous_id", _warehouse->id());
    query.bindValue(":item_id", _item->id());
    query.exec();
    _target->populate(query);
  }
  else
  {
    _source->clear();
    _target->clear();
  }
}
