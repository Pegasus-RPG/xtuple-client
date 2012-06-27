/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannedOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

plannedOrder::plannedOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl),
      _captive(false)
{
  setupUi(this);

  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sHandleItemsite(int)));
  connect(_dueDate, SIGNAL(newDate(const QDate &)), this, SLOT(sUpdateStartDate()));
  connect(_leadTime, SIGNAL(valueChanged(int)), this, SLOT(sUpdateStartDate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured |
                 ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

plannedOrder::~plannedOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

void plannedOrder::languageChange()
{
    retranslateUi(this);
}

enum SetResponse plannedOrder::set(const ParameterList &pParams)
{
  XSqlQuery plannedet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("planord_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _planordid = param.toInt();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      populateFoNumber();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      populate();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _startDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _leadTimeLit->hide();
      _leadTime->hide();
      _daysLit->hide();

      populate();
    }
  }

  return NoError;
}

void plannedOrder::sClose()
{
  XSqlQuery plannedClose;
  if (_mode == cNew)
  {
    plannedClose.prepare("SELECT releasePlanNumber(:orderNumber);");
    plannedClose.bindValue(":orderNumber", _number->text().toInt());
    plannedClose.exec();
  }

  reject();
}

void plannedOrder::sCreate()
{
  XSqlQuery plannedCreate;
  if (!(_item->isValid()))
  {
    QMessageBox::information( this, tr("No Item Number Selected"),
                              tr("You must enter or select a valid Item number before creating this Planned Order")  );
    return;
  }

  if (!_qty->text().length())
  {
    QMessageBox::information( this, tr("Invalid Quantity Ordered"),
                              tr( "You have entered an invalid Qty. Ordered.\n"
                                  "Please correct before creating this Planned Order"  ) );
    _qty->setFocus();
    return;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::information( this, tr("Invalid Due Date Entered"),
                              tr( "You have entered an invalid Due Date.\n"
                                  "Please correct before creating this Planned Order"  ) );
    _dueDate->setFocus();
    return;
  }

  plannedCreate.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  plannedCreate.bindValue(":item_id", _item->id());
  plannedCreate.bindValue(":warehous_id", _warehouse->id());
  plannedCreate.exec();
  if (!plannedCreate.first())
  {
    QMessageBox::information( this, tr("Invalid Item Site"),
                              tr("The Item and Site entered is and invalid Item Site combination.")  );
    return;
  }

  int itemsiteid = plannedCreate.value("itemsite_id").toInt();
  int _supplyItemsiteId = -1;
  if (_toButton->isChecked())
  {
    plannedCreate.prepare("SELECT itemsite_id "
              "FROM itemsite "
              "WHERE ( (itemsite_item_id=:item_id)"
              "  AND   (itemsite_warehous_id=:warehous_id) ); ");
    plannedCreate.bindValue(":item_id", _item->id());
    plannedCreate.bindValue(":warehous_id", _fromWarehouse->id());
    plannedCreate.exec();
    if (plannedCreate.first())
    {
      if (plannedCreate.value("itemsite_id").toInt() == itemsiteid)
      { 
        QMessageBox::warning( this, tr("Cannot Save Planned Order"),
          tr("The Supplied From Site must be different from the Transfer To Site.") );
        return;
      }
      else
        _supplyItemsiteId = plannedCreate.value("itemsite_id").toInt();
    }
    else
    { 
      QMessageBox::warning( this, tr("Cannot Save Planned"),
        tr("Cannot find Supplied From Item Site.") );
      return;
    }
  }

  int foid = 0;

  if(cEdit == _mode)
  {
    plannedCreate.prepare( "UPDATE planord "
               "SET planord_number=:planord_number, "
               "    planord_type=:planord_type, "
               "    planord_itemsite_id=:planord_itemsite_id, "
               "    planord_supply_itemsite_id=:planord_supply_itemsite_id, "
               "    planord_comments=:planord_comments, "
               "    planord_qty=:planord_qty, "
               "    planord_duedate=:planord_dueDate, "
               "    planord_startdate=(DATE(:planord_dueDate) - :planord_leadTime) "
               "WHERE (planord_id=:planord_id);" );
    plannedCreate.bindValue(":planord_number", _number->text().toInt());
    plannedCreate.bindValue(":planord_itemsite_id", itemsiteid);
    if (_poButton->isChecked())
      plannedCreate.bindValue(":planord_type", "P");
    else if (_woButton->isChecked())
      plannedCreate.bindValue(":planord_type", "W");
    else if (_toButton->isChecked())
    {
      plannedCreate.bindValue(":planord_type", "T");
      plannedCreate.bindValue(":planord_supply_itemsite_id", _supplyItemsiteId);
    }
    plannedCreate.bindValue(":planord_qty", _qty->toDouble());
    plannedCreate.bindValue(":planord_dueDate", _dueDate->date());
    plannedCreate.bindValue(":planord_leadTime", _leadTime->value());
    plannedCreate.bindValue(":planord_comments", _notes->toPlainText());
    plannedCreate.bindValue(":planord_id", _planordid);
    plannedCreate.exec();
    if (plannedCreate.lastError().type() != QSqlError::NoError)
    {
      systemError(this, plannedCreate.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    plannedCreate.prepare( "SELECT explodePlannedOrder( :planord_id, true) AS result;" );
    plannedCreate.bindValue(":planord_id", _planordid);
    plannedCreate.exec();
    if (plannedCreate.first())
    {
      double result = plannedCreate.value("result").toDouble();
      if (result < 0.0)
      {
        systemError(this, tr("ExplodePlannedOrder returned %, indicating an "
                             "error occurred.").arg(result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (plannedCreate.lastError().type() != QSqlError::NoError)
    {
      systemError(this, plannedCreate.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    plannedCreate.prepare( "SELECT createPlannedOrder( :orderNumber, :itemsite_id, :qty, "
               "                           (DATE(:dueDate) - :leadTime), :dueDate, "
               "                           :type, :supply_itemsite_id, :notes) AS result;" );
    plannedCreate.bindValue(":orderNumber", _number->text().toInt());
    plannedCreate.bindValue(":itemsite_id", itemsiteid);
    plannedCreate.bindValue(":qty", _qty->toDouble());
    plannedCreate.bindValue(":dueDate", _dueDate->date());
    plannedCreate.bindValue(":leadTime", _leadTime->value());
    plannedCreate.bindValue(":notes",    _notes->toPlainText());
    if (_poButton->isChecked())
      plannedCreate.bindValue(":type", "P");
    else if (_woButton->isChecked())
      plannedCreate.bindValue(":type", "W");
    else if (_toButton->isChecked())
    {
      plannedCreate.bindValue(":type", "T");
      plannedCreate.bindValue(":supply_itemsite_id", _supplyItemsiteId);
    }
  
    plannedCreate.exec();
    if (!plannedCreate.first())
    {
      systemError( this, tr("A System Error occurred at %1::%2.")
                         .arg(__FILE__)
                         .arg(__LINE__) );
      return;
    }

    foid = XDialog::Rejected;
    switch (plannedCreate.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Planned Order not Exploded"),
                               tr( "The Planned Order was created but not Exploded as there is no valid Bill of Materials for the selected Item.\n"
                                   "You must create a valid Bill of Materials before you may explode this Planned Order." ));
        break;
  
      case -2:
        QMessageBox::critical( this, tr("Planned Order not Exploded"),
                               tr( "The Planned Order was created but not Exploded as Component Items defined in the Bill of Materials\n"
                                   "for the selected Planned Order Item do not exist in the selected Planned Order Site.\n"
                                   "You must create Item Sites for these Component Items before you may explode this Planned Order." ));
        break;

      default:
        foid = plannedCreate.value("result").toInt();
        break;
    }
  }

  if (_captive)
    done(foid);
  else
  {
    populateFoNumber();
    _item->setId(-1);
    _typeGroup->setEnabled(FALSE);
    _qty->clear();
    _dueDate->setNull();
    _leadTime->setValue(0);
    _startDate->setNull();
    _notes->clear();
    _close->setText(tr("&Close"));

    _item->setFocus();
  }
}

void plannedOrder::populate()
{
  XSqlQuery planord;
  planord.prepare( "SELECT planord.*, (planord_duedate - planord_startdate) AS leadtime,"
                   "       itemsite_warehous_id AS supplywarehousid "
                   "FROM planord LEFT OUTER JOIN itemsite ON (planord_supply_itemsite_id=itemsite_id) "
                   "WHERE (planord_id=:planord_id);" );
  planord.bindValue(":planord_id", _planordid);
  planord.exec();
  if (planord.first())
  {
    _number->setText(planord.value("planord_number").toString());
    _item->setItemsiteid(planord.value("planord_itemsite_id").toInt());
    _qty->setDouble(planord.value("planord_qty").toDouble());
    _dueDate->setDate(planord.value("planord_duedate").toDate());
    _startDate->setDate(planord.value("planord_startdate").toDate());
    _leadTime->setValue(planord.value("leadtime").toInt());
    _notes->setText(planord.value("planord_comments").toString());
    if (planord.value("planord_type").toString() == "P")
      _poButton->setChecked(TRUE);
    else if (planord.value("planord_type").toString() == "W")
      _woButton->setChecked(TRUE);
    else
    {
      _toButton->setChecked(TRUE);
      _fromWarehouse->setId(planord.value("supplywarehousid").toInt());
    }
  }
  else if (planord.lastError().type() != QSqlError::NoError)
  {
    systemError(this, planord.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void plannedOrder::sUpdateStartDate()
{
  if(_dueDate->isValid())
    _startDate->setDate(_dueDate->date().addDays(_leadTime->value() * -1));
}

void plannedOrder::sHandleItemsite(int pWarehousid)
{
  XSqlQuery plannedHandleItemsite;
  plannedHandleItemsite.prepare( "SELECT itemsite_leadtime, itemsite_wosupply, itemsite_posupply, item_type "
             "FROM itemsite JOIN item ON (item_id=itemsite_item_id) "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  plannedHandleItemsite.bindValue(":item_id", _item->id());
  plannedHandleItemsite.bindValue(":warehous_id", pWarehousid);
  plannedHandleItemsite.exec();
  if (!plannedHandleItemsite.first())
  {
    systemError(this, plannedHandleItemsite.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _leadTime->setValue(plannedHandleItemsite.value("itemsite_leadtime").toInt());
  
  if ( plannedHandleItemsite.value("itemsite_posupply").toBool() &&
      (plannedHandleItemsite.value("item_type").toString() == "P" ||
       plannedHandleItemsite.value("item_type").toString() == "O" ||
       plannedHandleItemsite.value("item_type").toString() == "T" ||
       plannedHandleItemsite.value("item_type").toString() == "M") )
    _poButton->setEnabled(TRUE);
  else
    _poButton->setEnabled(FALSE);
  if ( plannedHandleItemsite.value("itemsite_wosupply").toBool() &&
      (plannedHandleItemsite.value("item_type").toString() == "P" ||
       plannedHandleItemsite.value("item_type").toString() == "T" ||
       plannedHandleItemsite.value("item_type").toString() == "M") )
    _woButton->setEnabled(TRUE);
  else
    _woButton->setEnabled(FALSE);
  if ( plannedHandleItemsite.value("itemsite_wosupply").toBool() && plannedHandleItemsite.value("itemsite_posupply").toBool() && plannedHandleItemsite.value("item_type").toString() == "P" )
  {
    _poButton->setChecked(TRUE);
    _woButton->setChecked(FALSE);
  }
  else if ( plannedHandleItemsite.value("itemsite_wosupply").toBool() )
  {
    _poButton->setChecked(FALSE);
    _woButton->setChecked(TRUE);
  }
  else
  {
    _poButton->setChecked(TRUE);
    _woButton->setChecked(FALSE);
  }

  plannedHandleItemsite.prepare( "SELECT COALESCE(COUNT(*), 0) AS supplysites "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id <> :warehous_id) );" );
  plannedHandleItemsite.bindValue(":item_id", _item->id());
  plannedHandleItemsite.bindValue(":warehous_id", pWarehousid);
  plannedHandleItemsite.exec();
  if (!plannedHandleItemsite.first())
  {
    systemError(this, plannedHandleItemsite.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (plannedHandleItemsite.value("supplysites").toInt() > 0)
    _toButton->setEnabled(TRUE);
  else
    _toButton->setEnabled(FALSE);

  plannedHandleItemsite.prepare( "SELECT COALESCE(supply.itemsite_id, -1) AS supplyitemsiteid,"
             "       COALESCE(supply.itemsite_warehous_id, -1) AS supplywarehousid "
             "FROM itemsite LEFT OUTER JOIN itemsite supply ON (supply.itemsite_id=itemsite.itemsite_supply_itemsite_id)"
             "WHERE ( (itemsite.itemsite_item_id=:item_id)"
             "  AND   (itemsite.itemsite_warehous_id=:warehous_id) );" );
  plannedHandleItemsite.bindValue(":item_id", _item->id());
  plannedHandleItemsite.bindValue(":warehous_id", pWarehousid);
  plannedHandleItemsite.exec();
  if (!plannedHandleItemsite.first())
  {
    systemError(this, plannedHandleItemsite.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (plannedHandleItemsite.value("supplyitemsiteid").toInt() != -1)
  {
    _toButton->setChecked(TRUE);
    _fromWarehouse->setId(plannedHandleItemsite.value("supplywarehousid").toInt());
  }
  else
    _fromWarehouse->setId(pWarehousid);
}

void plannedOrder::populateFoNumber()
{
  XSqlQuery plannedpopulateFoNumber;
  plannedpopulateFoNumber.exec("SELECT fetchPlanNumber() AS foNumber;");
  if (plannedpopulateFoNumber.first())
    _number->setText(plannedpopulateFoNumber.value("foNumber").toString());
  else
  {
  {
    systemError(this, plannedpopulateFoNumber.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
    _number->setText("Error");
    return;
  }
}

