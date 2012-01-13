/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemCost.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "currcluster.h"
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"

#define cItemCost    0x01
#define cBOMItemCost 0x02

static bool _foundCostElems;

itemCost::itemCost(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = false;
  _itemcostid = -1;

  _postCost->setEnabled(_privileges->check("PostActualCosts"));
  _foundCostElems = true; // assume it's true until sPopulateCostelem
}

itemCost::~itemCost()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemCost::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemCost::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _type = cBOMItemCost;
    _bomitemid = param.toInt();
    q.prepare( "SELECT bomitem_item_id "
               "FROM bomitem "
               "WHERE (bomitem_id=:bomitem_id);" );
    q.bindValue(":bomitem_id", _bomitemid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("bomitem_item_id").toInt());
      _item->setReadOnly(TRUE);
    }
  }

  param = pParams.value("bomitem_item_id", &valid);
  if (valid)
  {
    _type = cBOMItemCost;
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("bomitemcost_id", &valid);
  if (valid)
  {
    _type = cBOMItemCost;
    _itemcostid = param.toInt();
    _item->setReadOnly(TRUE);

    q.prepare( "SELECT bomitem_item_id, formatBoolYN(bomitemcost_lowlevel) AS lowlevel,"
               "       bomitemcost_actcost, bomitemcost_curr_id, bomitemcost_updated "
               "FROM bomitem, bomitemcost "
               "WHERE ( (bomitemcost_bomitem_id=bomitem_id)"
               " AND    (bomitemcost_id=:bomitemcost_id) );" );
    q.bindValue(":bomitemcost_id", _itemcostid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("bomitem_item_id").toInt());
      _lowerLevel->setText(q.value("lowlevel").toString());
      _actualCost->set(q.value("bomitemcost_actcost").toDouble(),
                       q.value("bomitemcost_curr_id").toInt(),
                       QDate::currentDate(),
                       false);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _type = cItemCost;
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
    _actualCost->setFocus();
  }
  else
    _item->setFocus();

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    _type = cItemCost;
    _itemcostid = param.toInt();
    _item->setReadOnly(TRUE);

    q.prepare( "SELECT item_id, formatBoolYN(itemcost_lowlevel) AS lowlevel,"
               "       itemcost_actcost, itemcost_curr_id, itemcost_updated "
               "FROM item, itemcost "
               "WHERE ( (itemcost_item_id=item_id)"
               " AND    (itemcost_id=:itemcost_id) );" );
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("item_id").toInt());
      _lowerLevel->setText(q.value("lowlevel").toString());
      _actualCost->set(q.value("itemcost_actcost").toDouble(),
		       q.value("itemcost_curr_id").toInt(),
		       QDate::currentDate(),
		       false);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _lowerLevel->hide();
      _lowerLevelLit->hide();
      if (_type == cItemCost)
        setWindowTitle("Create Item Cost");
      else
        setWindowTitle("Create BOM Item Cost");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _costelem->setEnabled(false);
      if (_type == cItemCost)
        setWindowTitle("Enter Actual Cost");
      else
        setWindowTitle("Enter BOM Item Actual Cost");
    }
    else if (param.toString() == "view")
    {
      _actualCost->setEnabled(false);
      _costelem->setEnabled(false);
      if (_type == cItemCost)
        setWindowTitle("View Actual Cost");
      else
        setWindowTitle("View BOM Item Actual Cost");
    }
  }

  sPopulateCostelem();

  return _foundCostElems ? NoError : UndefinedError;
}

void itemCost::sSave()
{
  if (_mode != cNew && _mode != cEdit)
    done(_itemcostid);

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_costelem->id() < 0, _costelem,
                          tr("You must select a Costing Element to save this Item Cost."))
     ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Item Cost"), errors))
    return;

  if (_type == cItemCost)
  {
    if (_mode == cNew)
    {
      q.exec("SELECT NEXTVAL('itemcost_itemcost_id_seq') AS itemcost_id;");
      if (q.first())
      {
        _itemcostid = q.value("itemcost_id").toInt();

        q.prepare( "INSERT INTO itemcost "
                   "( itemcost_id, itemcost_item_id,"
                   "  itemcost_costelem_id, itemcost_lowlevel,"
                   "  itemcost_stdcost, itemcost_posted,"
                   "  itemcost_actcost, itemcost_updated, "
                   "  itemcost_curr_id ) "
                   "VALUES "
                   "( :itemcost_id, :itemcost_item_id,"
                   "  :itemcost_costelem_id, FALSE,"
                   "  0, startOfTime(),"
                   "  :itemcost_actcost, CURRENT_DATE, "
                   "  :itemcost_curr_id );" );
        q.bindValue(":itemcost_item_id", _item->id());
        q.bindValue(":itemcost_costelem_id", _costelem->id());
      }
    }
    else if (_mode == cEdit)
    {
        q.prepare( "UPDATE itemcost SET"
                   " itemcost_actcost=:itemcost_actcost,"
                   " itemcost_curr_id=:itemcost_curr_id, "
                   " itemcost_updated=CURRENT_DATE "
                   "WHERE (itemcost_id=:itemcost_id);");
    }
    q.bindValue(":itemcost_id", _itemcostid);
    q.bindValue(":itemcost_actcost", _actualCost->localValue());
    q.bindValue(":itemcost_curr_id", _actualCost->id());

    if (q.exec() && _postCost->isChecked())
    {
      q.prepare("SELECT postCost(:itemcost_id) AS result;");
      q.bindValue(":itemcost_id", _itemcostid);
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("postCost", result),
                      __FILE__, __LINE__);
          return;
        }
      }
    }

    if (q.lastError().type() != QSqlError::NoError)  // if EITHER q.exec() failed
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_type == cBOMItemCost)
  {
    if (_mode == cNew)
    {
      q.exec("SELECT NEXTVAL('bomitemcost_bomitemcost_id_seq') AS itemcost_id;");
      if (q.first())
      {
        _itemcostid = q.value("itemcost_id").toInt();

        q.prepare( "INSERT INTO bomitemcost "
                   "( bomitemcost_id, bomitemcost_bomitem_id,"
                   "  bomitemcost_costelem_id, bomitemcost_lowlevel,"
                   "  bomitemcost_stdcost, bomitemcost_posted,"
                   "  bomitemcost_actcost, bomitemcost_updated, "
                   "  bomitemcost_curr_id ) "
                   "VALUES "
                   "( :bomitemcost_id, :bomitemcost_bomitem_id,"
                   "  :bomitemcost_costelem_id, FALSE,"
                   "  0, startOfTime(),"
                   "  :bomitemcost_actcost, CURRENT_DATE, "
                   "  :bomitemcost_curr_id );" );
        q.bindValue(":bomitemcost_bomitem_id", _bomitemid);
        q.bindValue(":bomitemcost_costelem_id", _costelem->id());
      }
    }
    else if (_mode == cEdit)
    {
      q.prepare( "UPDATE bomitemcost SET"
                 " bomitemcost_actcost=:bomitemcost_actcost,"
                 " bomitemcost_curr_id=:bomitemcost_curr_id, "
                 " bomitemcost_updated=CURRENT_DATE "
                 "WHERE (bomitemcost_id=:bomitemcost_id);");
    }
    q.bindValue(":bomitemcost_id", _itemcostid);
    q.bindValue(":bomitemcost_actcost", _actualCost->localValue());
    q.bindValue(":bomitemcost_curr_id", _actualCost->id());

    if (q.exec() && _postCost->isChecked())
    {
      q.prepare("UPDATE bomitemcost SET bomitemcost_stdcost=round(currToBase(bomitemcost_curr_id, bomitemcost_actcost, CURRENT_DATE),6), "
                "                       bomitemcost_posted=CURRENT_DATE "
                "WHERE bomitemcost_id=:bomitemcost_id;");
      q.bindValue(":bomitemcost_id", _itemcostid);
      q.exec();
    }

    if (q.lastError().type() != QSqlError::NoError)  // if EITHER q.exec() failed
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  done(_itemcostid);
}

void itemCost::sPopulateCostelem()
{
  if (_type == cItemCost)
  {
    if (_mode == cNew)
    {
      ParameterList params;
      params.append("item_id", _item->id());
      params.append("itemtype", _item->itemType());

      q = mqlLoad("costelem", "unusedbyitem").toQuery(params);
      _costelem->populate(q);
      if (q.size() <= 0)
      {
        QMessageBox::warning(this, tr("No Costing Elements Remaining"),
                             tr("<p>Item %1 already has all available costing "
                                "elements assigned. No new Item Costs can be "
                                "created for it until more costing elements are "
                                "defined.")
                             .arg(_item->itemNumber()));
        _foundCostElems = false;
      }
    }
    else // _mode == cEdit || cView)
    {
      QString sql( "SELECT costelem_id, costelem_type "
                   "FROM costelem, itemcost "
                   "WHERE ((costelem_id=itemcost_costelem_id)"
                   "  AND  (itemcost_id=:itemcost_id));");
      q.prepare(sql);
      q.bindValue(":itemcost_id", _itemcostid);
      q.exec();
      _costelem->populate(q);
    }
  }
  if (_type == cBOMItemCost)
  {
    if (_mode == cNew)
    {
      ParameterList params;
      params.append("bomitem_id", _bomitemid);
      params.append("itemtype", _item->itemType());

      q = mqlLoad("costelem", "unusedbyitem").toQuery(params);
      _costelem->populate(q);
      if (q.size() <= 0)
      {
        QMessageBox::warning(this, tr("No Costing Elements Remaining"),
                             tr("<p>BOM Item %1 already has all available costing "
                                "elements assigned. No new BOM Item Costs can be "
                                "created for it until more costing elements are "
                                "defined.")
                             .arg(_item->itemNumber()));
        _foundCostElems = false;
      }
    }
    else // _mode == cEdit || cView)
    {
      QString sql( "SELECT costelem_id, costelem_type "
                   "FROM costelem, bomitemcost "
                   "WHERE ((costelem_id=bomitemcost_costelem_id)"
                   "  AND  (bomitemcost_id=:itemcost_id));");
      q.prepare(sql);
      q.bindValue(":itemcost_id", _itemcostid);
      q.exec();
      _costelem->populate(q);
    }
  }
}
