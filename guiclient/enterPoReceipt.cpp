/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// rename to enterReceipt
#include "enterPoReceipt.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "distributeInventory.h"
#include "enterPoitemReceipt.h"
#include "getLotInfo.h"
#include "mqlutil.h"
#include "printLabelsByOrder.h"
#include "storedProcErrorLookup.h"

enterPoReceipt::enterPoReceipt(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_all,		SIGNAL(clicked()),	this, SLOT(sReceiveAll()));
  connect(_enter,	SIGNAL(clicked()),	this, SLOT(sEnter()));
  connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
  connect(_post,	SIGNAL(clicked()),	this, SLOT(sPost()));
  connect(_print,	SIGNAL(clicked()),	this, SLOT(sPrint()));
  connect(_save,	SIGNAL(clicked()),	this, SLOT(sSave()));

  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Purchase |
			  OrderLineEdit::Return |
			  OrderLineEdit::Transfer);
  _order->setToSitePrivsEnforced(TRUE);
  
  if (_metrics->boolean("EnableReturnAuth"))
  {
      _order->setExtraClause("RA", "(SELECT SUM(raitem_qtyauthorized) > 0 "
                       "  FROM raitem"
                       "  WHERE ((raitem_rahead_id=orderhead_id)"
                       "     AND (orderhead_type = 'RA'))) "
                       " AND "
                       "(SELECT TRUE "
                       " FROM raitem"
                       " WHERE ((raitem_rahead_id=orderhead_id)"
                       "   AND  (raitem_disposition IN ('R','P','V')) "
                       "   AND  (orderhead_type = 'RA')) "
                       " LIMIT 1)");
  }
  _order->setFocus();

  _orderitem->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter  , true,  "linenumber");
  _orderitem->addColumn(tr("Due Date"),     _dateColumn, Qt::AlignLeft    , true,  "duedate");
  _orderitem->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft    , true,  "item_number");
  _orderitem->addColumn(tr("Description"),  -1,          Qt::AlignLeft    , true,  "itemdescription");
  _orderitem->addColumn(tr("Inv. UOM"),     _uomColumn,  Qt::AlignCenter  , true,  "inv_uom");
  _orderitem->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter  , true,  "warehous_code");
  _orderitem->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft    , true,  "vend_item_number");
  _orderitem->addColumn(tr("Vend. UOM"),    _uomColumn,  Qt::AlignCenter  , true,  "vend_uom");
  _orderitem->addColumn(tr("Manufacturer"), _orderColumn,  Qt::AlignLeft  , false, "manuf_name");
  _orderitem->addColumn(tr("Manuf. Item#"), _orderColumn,  Qt::AlignCenter, false, "manuf_item_number");
  _orderitem->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight   , true,  "qty_ordered");
  _orderitem->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight   , true,  "qty_received");
  _orderitem->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight   , true,  "qty_returned");
  _orderitem->addColumn(tr("To Receive"),   _qtyColumn,  Qt::AlignRight   , true,  "qty_toreceive");

  _captive = FALSE;
}

enterPoReceipt::~enterPoReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoReceipt::languageChange()
{
  retranslateUi(this);
}

enum SetResponse enterPoReceipt::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _order->setId(param.toInt(), "PO");
    _orderitem->setFocus();
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _order->setId(param.toInt(), "TO");
    _orderitem->setFocus();
  }

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _order->setId(param.toInt(), "RA");
    _orderitem->setFocus();
  }

  return NoError;
}

void enterPoReceipt::setParams(ParameterList & params)
{
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_order->isValid())
  {
    if (_order->isPO())
      params.append("pohead_id", _order->id());
    else if (_order->isRA())
      params.append("rahead_id", _order->id());
    else if (_order->isTO())
      params.append("tohead_id", _order->id());

    params.append("orderid",   _order->id());
    params.append("ordertype", _order->type());
  }

  params.append("nonInventory",	tr("Non-Inventory"));
  params.append("na",		tr("N/A"));
}

void enterPoReceipt::sPrint()
{
  ParameterList params;
  setParams(params);

  printLabelsByOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void enterPoReceipt::post(const QString pType, const int pId)
{
  enterPoReceipt * w = new enterPoReceipt(0, "enterPoReceipt");
  w->setWindowModality(Qt::WindowModal);
  ParameterList params;
  if (pType == "PO")
    params.append("pohead_id", pId);
  else if (pType == "RA")
    params.append("rahead_id", pId);
  else if (pType == "TO")
    params.append("tohead_id", pId);
  w->set(params);
  w->sPost();
}

void enterPoReceipt::sPost()
{
  ParameterList params;	// shared by several queries
  setParams(params);

  QString checks = "SELECT SUM(qtyToReceive(recv_order_type,"
		   "                        recv_orderitem_id)) AS qtyToRecv "
		   "FROM recv, orderitem "
		   "WHERE ((recv_order_type=<? value(\"ordertype\") ?>)"
		   "  AND  (recv_orderitem_id=orderitem_id)"
		   "  AND  (orderitem_orderhead_id=<? value(\"orderid\") ?>));";
  MetaSQLQuery checkm(checks);
  q = checkm.toQuery(params);
  if (q.first())
  {
    if (q.value("qtyToRecv").toDouble() == 0)
    {
      QMessageBox::critical(this, tr("Nothing selected for Receipt"),
			    tr("<p>No Line Items have been selected "
			       "for receipt. Select at least one Line Item and "
			       "enter a Receipt before trying to Post."));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible insertgltransaction failures
  QString posts = "SELECT postReceipts(<? value (\"ordertype\") ?>,"
		  "                    <? value(\"orderid\") ?>,0) AS result;" ;
  MetaSQLQuery postm(posts);
  q = postm.toQuery(params);
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("postReceipts", result),
		  __FILE__, __LINE__);
      return;
    }

    QString lotnum = QString::null;
    QDate expdate = omfgThis->startOfTime();
    QDate warrdate;
    if(result > 0 && _singleLot->isChecked())
    {
      // first find out if we have any lot controlled items that need distribution
      q.prepare("SELECT count(*) AS result"
                "  FROM itemlocdist, itemsite"
                " WHERE ((itemlocdist_itemsite_id=itemsite_id)"
                "   AND  (itemlocdist_reqlotserial)"
                "   AND  (itemsite_controlmethod='L')"
                "   AND  (itemlocdist_series=:itemlocdist_series) ); ");
      q.bindValue(":itemlocdist_series", result);
      q.exec();
      // if we have any then ask for a lot# and optionally expiration date.
      if(q.first() && (q.value("result").toInt() > 0) )
      {
        getLotInfo newdlg(this, "", TRUE);

        // find out if any itemsites that are lot controlled are perishable
        q.prepare("SELECT itemsite_perishable,itemsite_warrpurc"
            "  FROM itemlocdist, itemsite"
            " WHERE ((itemlocdist_itemsite_id=itemsite_id)"
            "   AND  (itemlocdist_series=:itemlocdist_series) ); ");
        q.bindValue(":itemlocdist_series", result);
        q.exec();
        if(q.first())
        {
          newdlg.enableExpiration(q.value("itemsite_perishable").toBool()); 
          newdlg.enableWarranty(q.value("itemsite_warrpurc").toBool());
        }

          if(newdlg.exec() == XDialog::Accepted)
          {
            lotnum = newdlg.lot();
            expdate = newdlg.expiration();
            warrdate = newdlg.warranty();
          }
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          rollback.exec();
          return;
        }
      }

      if (distributeInventory::SeriesAdjust(result, this, lotnum, expdate, warrdate) == XDialog::Rejected)
      {
        QMessageBox::information( this, tr("Enter Receipts"), tr("Post Canceled") );
        rollback.exec();
        return;
      }
    
      q.exec("COMMIT;");
      
      // TODO: update this to sReceiptsUpdated?
      omfgThis->sPurchaseOrderReceiptsUpdated();
      
      if (_captive)
      {
        _orderitem->clear();
        close();
      }
      else
      {
        _order->setId(-1);
        _close->setText(tr("&Close"));
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else // select succeeded but returned no rows
      q.exec("COMMIT;");
}

void enterPoReceipt::sSave()
{
  // most of the work is done in enterPoitemReceipt
  // don't call this->close() 'cause that deletes the recv records
  XWidget::close();
}

void enterPoReceipt::sEnter()
{
  ParameterList params;
  params.append("lineitem_id", _orderitem->id());
  params.append("order_type", _order->type());
  params.append("mode", "new");

  enterPoitemReceipt newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void enterPoReceipt::sFillList()
{
  _orderitem->clear();

  disconnect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
  if (_order->isValid())
  {
    ParameterList params;
    setParams(params);
    MetaSQLQuery fillm = mqlLoad("receipt", "detail");
    q = fillm.toQuery(params);
    _orderitem->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
      return;
    }
  }
  connect(_order,	SIGNAL(valid(bool)),	this, SLOT(sFillList()));
}

void enterPoReceipt::close()
{
  QList<QTreeWidgetItem*> zeroItems = _orderitem->findItems("^[0.]*$", Qt::MatchRegExp, 9);
  if (_order->isValid() &&
      zeroItems.size() != _orderitem->topLevelItemCount())
  {
    if (QMessageBox::question(this, tr("Cancel Receipts?"),
			      tr("<p>Are you sure you want to cancel all "
				 "unposted Receipts for this Order?"),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      ParameterList params;
      setParams(params);
      QString dels = "SELECT deleteRecvForOrder(<? value (\"ordertype\") ?>,"
		     "                   <? value(\"orderid\") ?>) AS result;" ;
      MetaSQLQuery delm(dels);
      q = delm.toQuery(params);
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteRecvForOrder", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      omfgThis->sPurchaseOrderReceiptsUpdated();
    }
    else
      return;
  }
  if(!isVisible())
    deleteLater();
  else
    XWidget::close();
}

void enterPoReceipt::sReceiveAll()
{
  ParameterList params;
  setParams(params);
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth", TRUE);
  MetaSQLQuery recvm = mqlLoad("receipt", "receiveAll");
  q = recvm.toQuery(params);

  while (q.next())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("enterReceipt", result),
		  __FILE__, __LINE__);
      return;
    }
    omfgThis->sPurchaseOrderReceiptsUpdated();
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}
