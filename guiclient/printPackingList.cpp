/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPackingList.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>
#include <xsqlquery.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "inputManager.h"

printPackingList::printPackingList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_order,     SIGNAL(valid(bool)), this, SLOT(sPopulate()));
  connect(_print,       SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_reprint, SIGNAL(toggled(bool)), this, SLOT(sHandleReprint()));
  connect(_shipment,   SIGNAL(newId(int)), this, SLOT(sHandleShipment()));

  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Sales |
                          OrderLineEdit::Transfer);
  _order->setFromSitePrivsEnforced(TRUE);
  _shipment->setStatus(ShipmentClusterLineEdit::Unshipped);

  _captive    = FALSE;
  _shipformid = -1;

  _orderDate->setEnabled(false);

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _order, SLOT(setId(int)));
  _order->setFocus();
  adjustSize();
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
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _order->setId(param.toInt(), "SO");

  param = pParams.value("tohead_id", &valid);
  if (valid)
    _order->setId(param.toInt(), "TO");

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
    XSqlQuery shipq;
    shipq.prepare("SELECT shiphead_order_type, shiphead_order_id, shiphead_shipform_id "
                  "FROM shiphead "
                  "WHERE shiphead_id=:shiphead_id;");
    shipq.bindValue(":shiphead_id", param);
    shipq.exec();
    if (shipq.first())
    {
      _shipformid = shipq.value("shiphead_shipform_id").toInt();
      _headtype = shipq.value("shiphead_order_type").toString();
      if (_headtype == "SO")
        _order->setId(shipq.value("shiphead_order_id").toInt(), "SO");
      else if (_headtype == "TO")
        _order->setId(shipq.value("shiphead_order_id").toInt(), "TO");
      else
        return UndefinedError;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Shipment"),
                                  shipq, __FILE__, __LINE__))
      return UndefinedError;
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

ParameterList printPackingList::getParams()
{
  ParameterList params;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! _order->isValid() && _headtype == "SO", _order,
                          tr("You must enter a Sales Order Number"))
         << GuiErrorCheck(! _order->isValid() && _headtype == "TO", _order,
                          tr("You must enter a Transfer Order Number"))
     ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Print"), errors))
    return params;

  if (_printPack->isChecked())
    params.append("form", "P");
  else if (_printPick->isChecked())
    params.append("form", "L");
  else
    params.append("form", (_shipment->id() > 0) ? "P" : "L");

  if (_shipformid > 0 && ! _printPick->isChecked())
    params.append("shipformid", _shipformid);

  if (_headtype == "SO")
    params.append("sohead_id", _order->id());
  else if (_headtype == "TO")
    params.append("tohead_id", _order->id());

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth");

  if (_shipment->id() > 0)
    params.append("shiphead_id", _shipment->id());

  if (_metrics->boolean("EnableSOReservations"))
    params.append("EnableSOReservations");

  if (_metrics->boolean("EnableSOReservationsByLocation"))
    params.append("EnableSOReservationsByLocation");

  QString msg;
  bool    valid = false;
  MetaSQLQuery formm = MQLUtil::mqlLoad("packingList", "getreport", msg, &valid);
  if (! valid)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Form"),
                         msg, __FILE__, __LINE__);
    params.clear();
    return params;
  }

  XSqlQuery formq = formm.toQuery(params);
  formq.exec();
  if (formq.first())
    params.append("reportname", formq.value("reportname").toString());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Form"),
                                formq, __FILE__, __LINE__))
  {
    params.clear();
    return params;
  }

  return params;
}

void printPackingList::sPrint()
{
  ParameterList params = getParams();
  if (params.isEmpty())
    return;

  orReport report(params.value("reportname").toString(), params);
  if (report.isValid())
  {
    report.print();
    emit finishedPrinting();
  }
  else
  {
    report.reportError(this);
    if (_captive)
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

void printPackingList::sPopulate()
{
  if (_order->isSO() && _order->isValid())
  {
    _headtype = "SO";
    _shipment->setType(ShipmentClusterLineEdit::SalesOrder);
    _shipment->limitToOrder(_order->id());
  }
  else if (_order->isTO() && _order->isValid())
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

  _print->setEnabled(_order->isValid());

  if (! _headtype.isEmpty())
  {
    ParameterList destp;

    if (_order->isSO())
      destp.append("sohead_id", _order->id());
    else if (_order->isTO())
      destp.append("tohead_id", _order->id());
    destp.append("to", tr("Transfer Order"));

    QString dests = "<? if exists('sohead_id') ?>"
                    "SELECT cohead_number AS order_number,"
                    "       cohead_orderdate AS orderdate,"
                    "       cohead_custponumber AS alternate_number,"
                    "       cust_name AS name, cust_phone AS phone "
                    "FROM cohead, cust "
                    "WHERE ( (cohead_cust_id=cust_id)"
                    " AND (cohead_id=<? value('sohead_id') ?>) );"
                    "<? elseif exists('tohead_id') ?>"
                    "SELECT tohead_number AS order_number,"
                    "       tohead_orderdate AS orderdate,"
                    "       '' AS alternate_number,"
                    "       tohead_destname AS name, tohead_destphone AS phone "
                    "FROM tohead "
                    "WHERE (tohead_id=<? value('tohead_id') ?>);"
                    "<? endif ?>"
                    ;
    MetaSQLQuery destm(dests);
    XSqlQuery destq = destm.toQuery(destp);
    if (destq.first())
    {
      _orderDate->setDate(destq.value("orderdate").toDate());
      _poNumber->setText(destq.value("alternate_number").toString());
      _custName->setText(destq.value("name").toString());
      _custPhone->setText(destq.value("phone").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Recipient Information"),
                                  destq, __FILE__, __LINE__))
      return;

    ParameterList params;
    params.append("order_id", _order->id());
    params.append("ordertype", _headtype);
    if (_shipment->id() > 0)
      params.append("shiphead_id", _shipment->id());

    MetaSQLQuery mql("SELECT shiphead_id "
                     "FROM shiphead "
                     "WHERE ((shiphead_order_id=<? value('order_id') ?>) "
                     "  AND  (shiphead_order_type = <? value('ordertype') ?>)"
                     "<? if exists('shiphead_id') ?>"
                     "  AND (shiphead_id=<? value('shiphead_id') ?>)"
                     "<? else ?>"
                     "  AND  (NOT shiphead_shipped) "
                     "<? endif ?>"
                     ") "
                     "ORDER BY shiphead_number "
                     "LIMIT 1;");
    XSqlQuery shipq = mql.toQuery(params);
    if (shipq.first())
    {
      if (shipq.value("shiphead_id").toInt() != _shipment->id())
        _shipment->setId(shipq.value("shiphead_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Shipment"),
                                  shipq, __FILE__, __LINE__))
      return;
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

  QString msg;
  bool    valid = false;
  MetaSQLQuery mql = MQLUtil::mqlLoad("packingList", "shipment", msg, &valid);
  XSqlQuery plq = mql.toQuery(params);
  if (plq.first())
  {
    _shipformid = plq.value("shiphead_shipform_id").toInt();
    _headtype = plq.value("shiphead_order_type").toString();
    int orderid = plq.value("shiphead_order_id").toInt();
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
                                       .arg(plq.value("number").toString()),
                                  QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
          _order->setId(plq.value("shiphead_order_id").toInt());
        else
        {
          _shipformid = -1;
          _shipment->clear();
        }
      }
      else if (_headtype == "TO")
      {
        if (QMessageBox::question(this, tr("Shipment for different Order"),
                                        tr("<p>Shipment %1 is for Transfer Order %2. "
                                           "Are you sure the Shipment Number is correct?")
                                       .arg(_shipment->number())
                                       .arg(plq.value("number").toString()),
                                  QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
          _order->setId(plq.value("shiphead_order_id").toInt());
        else
        {
          _shipformid = -1;
          _shipment->clear();
        }
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Packing List Data"),
                                plq, __FILE__, __LINE__))
    return;
}

void printPackingList::sHandleReprint()
{
  if (_reprint->isChecked())
  {
    _order->setAllowedStatuses(OrderLineEdit::AnyStatus);
    _shipment->setStatus(ShipmentClusterLineEdit::AllStatus);
  }
  else
  {
    _order->setAllowedStatuses(OrderLineEdit::Open);
    _shipment->setStatus(ShipmentClusterLineEdit::Unshipped);
  }
}
