/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourcePrice.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

itemSourcePrice::itemSourcePrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _itemsrcpid = -1;
  _itemsrcid = -1;
}

itemSourcePrice::~itemSourcePrice()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemSourcePrice::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemSourcePrice::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    _itemsrcid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

  param = pParams.value("curr_effective", &valid);
  if (valid)
    _price->setEffective(param.toDate());

  param = pParams.value("itemsrcp_id", &valid);
  if (valid)
  {
    _itemsrcpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _qtyBreak->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void itemSourcePrice::sSave()
{
  XSqlQuery itemSave;
  itemSave.prepare("SELECT itemsrcp_id"
            "  FROM itemsrcp" 
            " WHERE ((itemsrcp_id != :itemsrcp_id)"
            "   AND  (itemsrcp_itemsrc_id=:itemsrcp_itemsrc_id)"
            "   AND  (itemsrcp_qtybreak=:qtybreak));");
  itemSave.bindValue(":itemsrcp_id", _itemsrcpid);
  itemSave.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  itemSave.bindValue(":qtybreak", _qtyBreak->toDouble());
  itemSave.exec();
  if(itemSave.first())
  {
    QMessageBox::warning(this, tr("Duplicate Qty. Break"),
      tr("A Qty. Break with the specified Qty. already exists for this Item Source.") );
    return;
  }

  if (_mode == cNew)
  {
    itemSave.exec("SELECT NEXTVAL('itemsrcp_itemsrcp_id_seq') AS itemsrcp_id;");
    if (itemSave.first())
      _itemsrcpid = itemSave.value("itemsrcp_id").toInt();
//  ToDo

    itemSave.prepare( "INSERT INTO itemsrcp "
               "(itemsrcp_id, itemsrcp_itemsrc_id, itemsrcp_qtybreak, itemsrcp_price, itemsrcp_updated, itemsrcp_curr_id) "
               "VALUES "
               "(:itemsrcp_id, :itemsrcp_itemsrc_id, :itemsrcp_qtybreak, :itemsrcp_price, CURRENT_DATE, :itemsrcp_curr_id);" );
  }
  else if (_mode == cEdit)
    itemSave.prepare( "UPDATE itemsrcp "
               "SET itemsrcp_qtybreak=:itemsrcp_qtybreak, "
	       "    itemsrcp_price=:itemsrcp_price, "
	       "    itemsrcp_updated=CURRENT_DATE, "
	       "    itemsrcp_curr_id=:itemsrcp_curr_id "
               "WHERE (itemsrcp_id=:itemsrcp_id);" );

  itemSave.bindValue(":itemsrcp_id", _itemsrcpid);
  itemSave.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  itemSave.bindValue(":itemsrcp_qtybreak", _qtyBreak->toDouble());
  itemSave.bindValue(":itemsrcp_price", _price->localValue());
  itemSave.bindValue(":itemsrcp_curr_id", _price->id());
  itemSave.exec();

  done(_itemsrcpid);
}

void itemSourcePrice::populate()
{
  XSqlQuery itempopulate;
  itempopulate.prepare( "SELECT itemsrcp_qtybreak,"
             "       itemsrcp_price, itemsrcp_curr_id,"
             "       itemsrcp_updated, itemsrcp_itemsrc_id "
             "FROM itemsrcp "
             "WHERE (itemsrcp_id=:itemsrcp_id);" );
  itempopulate.bindValue(":itemsrcp_id", _itemsrcpid);
  itempopulate.exec();
  if (itempopulate.first())
  {
    _itemsrcid = itempopulate.value("itemsrcp_itemsrc_id").toInt();
    _qtyBreak->setDouble(itempopulate.value("itemsrcp_qtybreak").toDouble());
    _price->setLocalValue(itempopulate.value("itemsrcp_price").toDouble());
    _price->setEffective(itempopulate.value("itemsrcp_updated").toDate());
    _price->setId(itempopulate.value("itemsrcp_curr_id").toInt());
  }
}
