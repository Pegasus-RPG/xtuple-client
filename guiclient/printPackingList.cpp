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
 * The Original Code is xTuple ERP: PostBooks Edition 
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
 * Powered by xTuple ERP: PostBooks Edition
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

#include "printPackingList.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "inputManager.h"
#include "mqlutil.h"
#include "salesOrderList.h"

printPackingList::printPackingList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_print,	     SIGNAL(clicked()),   this, SLOT(sPrint()));
    connect(_shipment,	 SIGNAL(newId(int)),  this, SLOT(sHandleShipment()));
    connect(_order,      SIGNAL(valid(bool)), this, SLOT(sPopulate()));

    _order->setAllowedStatuses(OrderLineEdit::Open);
    _order->setAllowedTypes(OrderLineEdit::Sales |
                            OrderLineEdit::Transfer);
    _order->setFromSitePrivsEnforced(TRUE);

    _captive	= FALSE;

#ifndef Q_WS_MAC
    //_salesOrderList->setMaximumWidth(25);
#endif

    _orderDate->setEnabled(false);

    omfgThis->inputManager()->notify(cBCSalesOrder, this, _order, SLOT(setId(int)));
    _order->setFocus();
}

printPackingList::~printPackingList()
{
  // no need to delete child widgets, Qt does it all for us
}

void printPackingList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printPackingList::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _order->setId(param.toInt(), "SO");

  param = pParams.value("tohead_id", &valid);
  if (valid)
    _order->setId(param.toInt(), "TO");

  // TODO: deprecate cosmisc_id parameters
  param = pParams.value("cosmisc_id", &valid);
  if (valid)
  {
    _shipment->setId(param.toInt());
    _headtype = "SO";
  }

  param = pParams.value("head_type", &valid);
  if (valid)
    _headtype = param.toString();

  param = pParams.value("head_id", &valid);
  if (valid)
  {
    if (_headtype == "SO")
      _order->setId(param.toInt(), "SO");
    else if (_headtype == "TO")
      _order->setId(param.toInt(), "TO");
    else
      return UndefinedError;
  }

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    _shipment->setId(param.toInt());
    q.prepare("SELECT shiphead_order_type, shiphead_order_id "
	      "FROM shiphead "
	      "WHERE shiphead_id=:shiphead_id;");
    q.bindValue(":shiphead_id", param);
    q.exec();
    if (q.first())
    {
      _headtype = q.value("shiphead_order_type").toString();
      if (_headtype == "SO")
        _order->setId(q.value("shiphead_order_id").toInt(), "SO");
      else if (_headtype == "TO")
        _order->setId(q.value("shiphead_order_id").toInt(), "TO");
      else
	return UndefinedError;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if(pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (_order->isValid() || _shipment->isValid())
    _print->setFocus();

  return NoError;
}

void printPackingList::sPrint()
{
  if (_headtype == "SO")
  {
    if (! _order->isValid())
    {
      QMessageBox::warning( this, tr("Enter Sales Order"),
			    tr("<p>You must enter a Sales Order Number."));
      if (! _order->isValid())
        _order->setFocus();
      else
        _shipment->setFocus();
      return;
    }

    q.prepare( "SELECT findCustomerForm(cohead_cust_id, :form) AS reportname "
	       "FROM cohead "
	       "WHERE (cohead_id=:head_id);" );
    q.bindValue(":head_id", _order->id());
  }
  else if (_headtype == "TO")
  {
    if (! _order->isValid())
    {
      QMessageBox::warning(this, tr("Enter Transfer Order"),
			   tr("<p>You must enter a Transfer Order Number."));
      _shipment->setFocus();
      return;
    }
    q.prepare( "SELECT findTOForm(:head_id, :form) AS reportname;" );
    q.bindValue(":head_id", _order->id());
  }

  q.bindValue(":form", (_shipment->id() > 0) ? "P" : "L");

  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("head_id", _order->id());
    if (_headtype == "SO")
      params.append("sohead_id", _order->id());
    else
      params.append("sohead_id", -1);
    if (_headtype == "TO")
      params.append("tohead_id", _order->id());
    else
      params.append("tohead_id", -1);
    params.append("head_type", _headtype);
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");

    if (_shipment->id() > 0)
    {
      params.append("cosmisc_id", _shipment->id());
      params.append("shiphead_id", _shipment->id());
    }

    orReport report(q.value("reportname").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
      return;
    }

    if (_captive)
      accept();
    else
    {
      _close->setText(tr("&Close"));
      _order->setId(-1);
      _headtype = "";
      _order->setFocus();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printPackingList::sPopulate()
{
  if (_order->isSO())
  {
    _headtype = "SO";
    _shipment->setType(ShipmentClusterLineEdit::SalesOrder);
    _shipment->limitToOrder(_order->id());
  }
  else if (_order->isTO())
  {
    _headtype = "TO";
    _shipment->setType(ShipmentClusterLineEdit::TransferOrder);
    _shipment->limitToOrder(_order->id());
  }
  else
  {
    _headtype = "";
    _shipment->setType(ShipmentClusterLineEdit::All);
    _shipment->removeOrderLimit();
  }

// qDebug("sPopulate: _headtype %s, _shipment %d, _order %d",
// _headtype.toAscii().data(), _shipment->id(), _order->id());

  _print->setEnabled(_order->isValid());

  if (! _headtype.isEmpty())
  {
    ParameterList destp;

    if (_order->isSO())
      destp.append("sohead_id", _order->id());
    else if (_order->isTO())
      destp.append("tohead_id", _order->id());
    destp.append("to", tr("Transfer Order"));

    QString dests = "<? if exists(\"sohead_id\") ?>"
		    "SELECT cohead_number AS order_number,"
		    "       cohead_orderdate AS orderdate,"
		    "       cohead_custponumber AS alternate_number,"
		    "       cust_name AS name, cust_phone AS phone "
		    "FROM cohead, cust "
		    "WHERE ( (cohead_cust_id=cust_id)"
		    " AND (cohead_id=<? value(\"sohead_id\") ?>) );"
		    "<? elseif exists(\"tohead_id\") ?>"
		    "SELECT tohead_number AS order_number,"
		    "       tohead_orderdate AS orderdate,"
		    "       <? value(\"to\") ?> AS alternate_number,"
		    "       tohead_destname AS name, tohead_destphone AS phone "
		    "FROM tohead "
		    "WHERE (tohead_id=<? value(\"tohead_id\") ?>);"
		    "<? endif ?>"
		    ;
    MetaSQLQuery destm(dests);
    q = destm.toQuery(destp);
    if (q.first())
    {
      _orderDate->setDate(q.value("orderdate").toDate());
      _poNumber->setText(q.value("alternate_number").toString());
      _custName->setText(q.value("name").toString());
      _custPhone->setText(q.value("phone").toString());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    QString sql("SELECT shiphead_id "
		"FROM shiphead "
		"WHERE ((shiphead_order_id=<? value(\"order_id\") ?>) "
		"  AND  (shiphead_order_type = <? value(\"ordertype\") ?>)"
		"<? if exists(\"shiphead_id\") ?>"
		"  AND (shiphead_id=<? value(\"shiphead_id\") ?>)"
		"<? else ?>"
		"  AND  (NOT shiphead_shipped) "
		"<? endif ?>"
		") "
		"ORDER BY shiphead_number "
		"LIMIT 1;");

    ParameterList params;
    params.append("order_id", _order->id());
    params.append("ordertype", _headtype);
    if (_shipment->id() > 0)
      params.append("shiphead_id", _shipment->id());
    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.first())
    {
      if (q.value("shiphead_id").toInt() != _shipment->id())
	_shipment->setId(q.value("shiphead_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      _shipment->setId(-1);
  }
  else
  {
    _shipment->removeOrderLimit();
    _shipment->clear();
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}

void printPackingList::sHandleShipment()
{
  if (! _shipment->isValid())
    return;

  ParameterList params;
  params.append("shiphead_id", _shipment->id());
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  MetaSQLQuery mql = mqlLoad(":/sr/forms/printPackingList/HandleShipment.mql");
  q = mql.toQuery(params);
  if (q.first())
  {
    _headtype = q.value("shiphead_order_type").toString();
    int orderid = q.value("shiphead_order_id").toInt();
    if (_headtype == "SO" && ! _order->isValid())
      _order->setId(orderid);
    else if (_headtype == "TO" && ! _order->isValid())
      _order->setId(orderid);
    else if (orderid != _order->id())
    {
      if (_headtype == "SO")
      {
      if (QMessageBox::question(this, tr("Shipment for different Order"),
				tr("<p>Shipment %1 is for Sales Order %2. "
				   "Are you sure the Shipment Number is correct?")
				   .arg(_shipment->number())
				   .arg(q.value("number").toString()),
				QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
	_order->setId(q.value("shiphead_order_id").toInt());
      else
	_shipment->clear();
      }
      else if (_headtype == "TO")
      {
      if (QMessageBox::question(this, tr("Shipment for different Order"),
				tr("<p>Shipment %1 is for Transfer Order %2. "
				   "Are you sure the Shipment Number is correct?")
				   .arg(_shipment->number())
				   .arg(q.value("number").toString()),
				QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
	_order->setId(q.value("shiphead_order_id").toInt());
      else
	_shipment->clear();
      }
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
