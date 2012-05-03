/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "enterPoitemReturn.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "storedProcErrorLookup.h"

enterPoitemReturn::enterPoitemReturn(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));

  _invVendorUOMRatio->setPrecision(omfgThis->ratioVal());
  _ordered->setPrecision(omfgThis->qtyVal());
  _received->setPrecision(omfgThis->qtyVal());
  _toReturn->setValidator(omfgThis->qtyVal());

  _item->setReadOnly(TRUE);

  _rejectCode->setAllowNull(TRUE);
  _rejectCode->populate( "SELECT rjctcode_id, rjctcode_code "
                         "FROM rjctcode "
                         "ORDER BY rjctcode_code;" );

  _receipts->addColumn(tr("Receipt Date"),    _timeDateColumn,  Qt::AlignCenter, true, "recv_date");
  _receipts->addColumn(tr("Receiving Agent"), 100,		Qt::AlignCenter, true, "recv_agent_username");
  _receipts->addColumn(tr("G/L Post Date"),   _dateColumn,	Qt::AlignCenter, true, "recv_gldistdate");
  _receipts->addColumn(tr("Returnable Qty."), 100,		Qt::AlignRight,  true, "returnable");
  _receipts->addColumn(tr("Purchase Cost"),   -1,		Qt::AlignRight,  true, "recv_purchcost");
  _receiptsLit->hide();
  _receiptsLine->hide();
  _receipts->hide();
}

enterPoitemReturn::~enterPoitemReturn()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoitemReturn::languageChange()
{
  retranslateUi(this);
}

enum SetResponse enterPoitemReturn::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    q.prepare("	SELECT recv_id, recv_date, recv_agent_username, recv_gldistdate,"
              "        recv_qty-COALESCE(SUM(poreject_qty),0) AS returnable, recv_purchcost "
              "	FROM recv LEFT OUTER JOIN poreject ON (recv_id = poreject_recv_id) "
              " WHERE recv_orderitem_id=:poitem_id AND recv_order_type='PO' "
              " GROUP BY recv_id, recv_date, recv_agent_username, recv_gldistdate, recv_purchcost, recv_qty;");
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    _receipts->clear();
    _receipts->populate(q);

    q.prepare(	"SELECT CASE WHEN itemsite_costmethod='A' THEN TRUE ELSE FALSE END AS costmethod_average "
                "FROM poitem LEFT OUTER JOIN itemsite ON (poitem_itemsite_id = itemsite_id) "
                "WHERE poitem_id = :poitem_id;");
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    if (q.first())
    {
      _receipts->setVisible(q.value("costmethod_average").toBool() && _metrics->boolean("AllowReceiptCostOverride"));
      _receiptsLit->setVisible(q.value("costmethod_average").toBool() && _metrics->boolean("AllowReceiptCostOverride"));
      _receiptsLine->setVisible(q.value("costmethod_average").toBool() && _metrics->boolean("AllowReceiptCostOverride"));
    }

    q.prepare( "SELECT pohead_number, poitem_linenumber,"
               "       COALESCE(itemsite_id, -1) AS itemsiteid,"
               "       noNeg(poitem_qty_received - poitem_qty_returned) AS returnable,"
               "       poitem_vend_item_number, poitem_vend_uom, poitem_vend_item_descrip,"
               "       poitem_qty_ordered,"
               "       noNeg(poitem_qty_received - poitem_qty_returned) AS returnableqty,"
               "       poitem_invvenduomratio "
               "FROM pohead, poitem LEFT OUTER JOIN "
               "             ( itemsite JOIN item "
               "               ON (itemsite_item_id=item_id)"
               "             ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ( (poitem_pohead_id=pohead_id)"
               " AND (poitem_id=:poitem_id) );" );
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
      _lineNumber->setText(q.value("poitem_linenumber").toString());
      _vendorItemNumber->setText(q.value("poitem_vend_item_number").toString());
      _vendorDescrip->setText(q.value("poitem_vend_item_descrip").toString());
      _vendorUOM->setText(q.value("poitem_vend_uom").toString());
      _ordered->setDouble(q.value("poitem_qty_ordered").toDouble());
      _received->setDouble(q.value("returnableqty").toDouble());
      _invVendorUOMRatio->setDouble(q.value("poitem_invvenduomratio").toDouble());

      if (q.value("itemsiteid").toInt() != -1)
        _item->setItemsiteid(q.value("itemsiteid").toInt());

      _cachedReceived = q.value("returnable").toDouble();

      q.prepare( "SELECT COALESCE(SUM(poreject_qty), 0) AS qtytoreturn "
                 "FROM poreject "
                 "WHERE ( (poreject_poitem_id=:poitem_id)"
                 " AND (NOT poreject_posted) );" );
      q.bindValue(":poitem_id", _poitemid);
      q.exec();
      if (q.first())
        _toReturn->setText(q.value("qtytoreturn").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void enterPoitemReturn::sReturn()
{
  if (_rejectCode->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You may not enter this return until you select a Reject Code.\n") );
    _rejectCode->setFocus();
    return;
  }

  if (_cachedReceived < _toReturn->toDouble())
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You may not enter a return whose returned quantity is greater than the returnable quantity.") );
    _toReturn->setFocus();
    return;
  }

  if (_toReturn->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You must enter a quantity to return.") );
    _toReturn->setFocus();
    return;
  }

  if (_receipts->isVisible() && _receipts->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Enter Return"),
                           tr("You may not enter this return until you select a Receipt.\n") );
    _receipts->setFocus();
    return;
  }

  if (_receipts->isVisible())
  {
    QList<XTreeWidgetItem*> items = _receipts->selectedItems();
    if (_toReturn->toDouble() > items.at(0)->text(3).toDouble())
    {
      QMessageBox::critical( this, tr("Cannot Enter Return"),
                            tr("Quantity to return may not be greater than the returnable quantity from the selected receipt.\n") );
      _toReturn->setFocus();
      return;
    }
  }

  q.prepare("SELECT enterPoReturn(:poitem_id, :qty, :rjctcode_id, :recv_id) AS result;");
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":qty", _toReturn->toDouble());
  q.bindValue(":rjctcode_id", _rejectCode->id());
  if (_receipts->id() != -1)
    q.bindValue(":recv_id", _receipts->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical(this, tr("Cannot Enter Return"),
			    storedProcErrorLookup("enterPoReturn", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}
