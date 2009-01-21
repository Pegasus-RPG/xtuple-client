/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createPlannedOrdersByItem.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a createPlannedOrdersByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
createPlannedOrdersByItem::createPlannedOrdersByItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _item->setType(ItemLineEdit::cPlanningMRP);

  // signals and slots connections
  connect(_item, SIGNAL(valid(bool)), _create, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _captive = FALSE;
}

/*
 *  Destroys the object and frees any allocated resources
 */
createPlannedOrdersByItem::~createPlannedOrdersByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void createPlannedOrdersByItem::languageChange()
{
    retranslateUi(this);
}


enum SetResponse createPlannedOrdersByItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _cutOffDate->setFocus();
  }

  return NoError;
}

void createPlannedOrdersByItem::sCreate()
{
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cut Off Date"),
                          tr( "You must enter a valid Cut Off Date before\n"
                              "creating Planned Orders." ));
    _cutOffDate->setFocus();
    return;
  }

  if(!_explodeChildren->isChecked())
  {
  q.prepare( "SELECT createPlannedOrders(itemsite_id, :cutOffDate, :deleteFirmed, FALSE) "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_active)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":cutOffDate", _cutOffDate->date());
  q.bindValue(":deleteFirmed", QVariant(_deleteFirmed->isChecked()));
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  }
  else
  {
    q.prepare( "SELECT createAndExplodePlannedOrders(itemsite_id, :cutOffDate, :deleteFirmed, false)"
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_active)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":cutOffDate", _cutOffDate->date());
    q.bindValue(":deleteFirmed", QVariant(_deleteFirmed->isChecked()));
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
  }   

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    _item->setId(-1);
    _item->setFocus();
  }
}

