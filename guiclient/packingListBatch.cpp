/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "packingListBatch.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "printPackingList.h"
#include "salesOrder.h"
#include "salesOrderList.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"
#include "transferOrderList.h"
#include "errorReporter.h"

packingListBatch::packingListBatch(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_add,		           SIGNAL(clicked()),     this, SLOT(sAddSO()));
  connect(_addTO,	           SIGNAL(clicked()),     this, SLOT(sAddTO()));
  connect(_autoUpdate,	     SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_delete,	         SIGNAL(clicked()),     this, SLOT(sDelete()));
  connect(_deletePrinted,    SIGNAL(clicked()),     this, SLOT(sClearPrinted()));
  connect(_pack, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_printBatch,       SIGNAL(clicked()),     this, SLOT(sPrintBatch()));
  connect(_printEditList,    SIGNAL(clicked()),     this, SLOT(sPrintEditList()));
  connect(_printPackingList, SIGNAL(clicked()),     this, SLOT(sPrintPackingList()));
  connect(_warehouse,        SIGNAL(updated()),     this, SLOT(sFillList()));
  connect(_creditHold,       SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_packHold,         SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_shipHold,         SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  setAcceptDrops(true);

  _pack->addColumn(tr("Order #"),       80,           Qt::AlignCenter, true, "order_number" );
  _pack->addColumn(tr("Type"),		40,           Qt::AlignCenter, true, "pack_head_type" );
  _pack->addColumn(tr("Shipment #"),    80,           Qt::AlignCenter, true, "shipment_number" );
  _pack->addColumn(tr("Customer #"),    _itemColumn,  Qt::AlignLeft,   true, "number"   );
  _pack->addColumn(tr("Customer Name"), -1,           Qt::AlignLeft,   true, "name"   );
  _pack->addColumn(tr("Ship Via"),      80,           Qt::AlignLeft,   true, "shipvia");
  _pack->addColumn(tr("Pack Date"),     80,           Qt::AlignLeft,   true, "packdate");
  _pack->addColumn(tr("Hold Type"),     _dateColumn,  Qt::AlignCenter, true, "f_holdtype" );
  _pack->addColumn(tr("Printed"),       _dateColumn,  Qt::AlignCenter, true, "pack_printed" );

  if (_privileges->check("MaintainPackingListBatch"))
  {
    _add->setEnabled(true);
    _addTO->setEnabled(true);
    _deletePrinted->setEnabled(true);
    connect(_pack, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  }

  if (_privileges->check("PrintPackingLists"))
  {
    _printBatch->setEnabled(true);
    connect(_pack, SIGNAL(valid(bool)), _printPackingList, SLOT(setEnabled(bool)));
  }

  _addTO->setVisible(_metrics->boolean("MultiWhs"));

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

packingListBatch::~packingListBatch()
{
    // no need to delete child widgets, Qt does it all for us
}

void packingListBatch::languageChange()
{
    retranslateUi(this);
}

void packingListBatch::sPrintBatch()
{
  XSqlQuery packingPrintBatch;
  XSqlQuery updateq;
  updateq.prepare("UPDATE pack "
		  "SET pack_printed=true "
		  "WHERE (pack_id=:packid);" );

  ParameterList params;
  _warehouse->appendValue(params);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery mql = mqlLoad("packingListBatch", "print");
  packingPrintBatch = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Printing Packing List Batch"),
                                packingPrintBatch, __FILE__, __LINE__))
  {
    return;
  }

  QPrinter printer(QPrinter::HighResolution);
  bool     setupPrinter = true;
  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Could not initialize printing system "
                            "for multiple reports").arg(windowTitle()),__FILE__,__LINE__);
    return;
  }

  while (packingPrintBatch.next())
  {
    int osmiscid = packingPrintBatch.value("pack_shiphead_id").toInt();
    bool usePickForm;
    if (_printPick->isChecked())
      usePickForm = true;
    else if (_printPack->isChecked())
      usePickForm = false;
    else if (osmiscid > 0)
      usePickForm = false;
    else
      usePickForm = true;

    // skip when PackForm and no shiphead_id
    if (!usePickForm && osmiscid <= 0)
        continue;

    // set sohead_id, tohead_id, and shiphead_id for customer and 3rd-party use
    ParameterList params;
    params.append("head_id",   packingPrintBatch.value("pack_head_id").toInt());
    params.append("head_type", packingPrintBatch.value("pack_head_type").toString());
    if (packingPrintBatch.value("pack_head_type").toString() == "SO")
      params.append("sohead_id", packingPrintBatch.value("pack_head_id").toInt());
    else if (packingPrintBatch.value("pack_head_type").toString() == "TO")
      params.append("tohead_id", packingPrintBatch.value("pack_head_id").toInt());
    if (osmiscid > 0)
    {
      params.append("shiphead_id",  osmiscid);
    }
    _warehouse->appendValue(params);
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");

    orReport report(packingPrintBatch.value(usePickForm ? "pickform" : "packform").toString(), params);
    if (! report.isValid())
    {
      report.reportError(this);
    }
    else if (report.print(&printer, setupPrinter))
    {
      setupPrinter = false;
      updateq.bindValue(":packid", packingPrintBatch.value("pack_id").toInt());
      updateq.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Printing Packing List Batch"),
                                    updateq, __FILE__, __LINE__))
      {
        orReport::endMultiPrint(&printer);
        return;
      }
      emit finishedPrinting(packingPrintBatch.value("pack_head_id").toInt(),
                            packingPrintBatch.value("pack_head_type").toString(),
                            packingPrintBatch.value("pack_shiphead_id").toInt());
    }
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }
  orReport::endMultiPrint(&printer);

  if (setupPrinter)
  {
    QMessageBox::warning(this, tr("Nothing to Print"),
			 tr("<p>All of the Packing Lists appear to have been "
			    "printed already."));
  }
  else
  {
    sFillList();
  }
}

void packingListBatch::setParams(ParameterList & params)
{
  _warehouse->appendValue(params);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  params.append("none",		tr("None"));
  params.append("credit",	tr("Credit"));
  params.append("ship",		tr("Ship"));
  params.append("pack",		tr("Pack"));
  params.append("return",	tr("Return"));
  params.append("tax",          tr("Tax"));
  params.append("other",	tr("Other"));

  if (_creditHold->isChecked())
    params.append("showCredit", true);
  if (_packHold->isChecked())
    params.append("showPack", true);
  if (_shipHold->isChecked())
    params.append("showShip", true);
}

void packingListBatch::sPrintEditList()
{
  ParameterList params;
  setParams(params);
  orReport report("PackingListBatchEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void packingListBatch::sClearPrinted()
{
  XSqlQuery packingClearPrinted;
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("packingListBatch", "clear");
  packingClearPrinted = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Clearing Packing List Batch"),
                                packingClearPrinted, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void packingListBatch::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  if (_pack->currentItem()->rawValue("pack_head_type") == "SO")
  {
    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                        _privileges->check("ViewSalesOrders"));
  }

  if (_pack->currentItem()->rawValue("pack_head_type") == "TO")
  {
    menuItem = pMenu->addAction(tr("View Transfer Order..."), this, SLOT(sViewTransferOrder()));
    menuItem->setEnabled(_privileges->check("MaintainTransferOrders") ||
                        _privileges->check("ViewTransferOrders"));
  }
}

void packingListBatch::sViewSalesOrder()
{
  salesOrder::viewSalesOrder(_pack->id());
}

void packingListBatch::sViewTransferOrder()
{
  transferOrder::viewTransferOrder(_pack->id());
}

void packingListBatch::sAddSO()
{
  XSqlQuery packingAddSO;
  ParameterList params;
  params.append("soType", cSoOpen);
  params.append("exclShipped", true);
  _warehouse->appendValue(params);

  salesOrderList newdlg(this, "", true);
  newdlg.set(params);

  int soid;
  if ((soid = newdlg.exec()) != QDialog::Rejected)
  {
    packingAddSO.prepare("SELECT addToPackingListBatch(:warehous_id, 'SO'::TEXT, :sohead_id) AS result;");
    packingAddSO.bindValue(":sohead_id", soid);
    _warehouse->bindValue(packingAddSO);
    packingAddSO.exec();
    if (packingAddSO.first())
      sFillList();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Adding SO To Packing List Batch"),
                                  packingAddSO, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void packingListBatch::sAddTO()
{
  XSqlQuery packingAddTO;
  ParameterList params;
  params.append("toType", cToOpen);
  _warehouse->appendValue(params);

  transferOrderList newdlg(this, "", true);
  newdlg.set(params);

  int toid;
  if ((toid = newdlg.exec()) != QSqlError::NoError)
  {
    packingAddTO.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
    packingAddTO.bindValue(":tohead_id", toid);
    packingAddTO.exec();
    if (packingAddTO.first())
      sFillList();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Adding To Packing List Batch"),
                                  packingAddTO, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void packingListBatch::sDelete()
{
  XSqlQuery packingDelete;
  QString sql( "DELETE FROM pack "
	       "WHERE ((pack_head_id=<? value(\"head_id\") ?>)"
	       "  AND  (pack_head_type=<? value(\"head_type\") ?>)"
	       " <? if exists(\"shiphead_id\") ?>"
	       "  AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
	       " <? else ?>"
	       "  AND  (pack_shiphead_id IS NULL)"
	       "<? endif ?>"
	       ");" );

  ParameterList params;
  params.append("head_id",   _pack->id());
  params.append("head_type", _pack->currentItem()->rawValue("pack_head_type"));
  if (_pack->altId() > 0)
    params.append("shiphead_id", _pack->altId());

  MetaSQLQuery mql(sql);
  packingDelete = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Packing List Batch Information"),
                                           packingDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void packingListBatch::sPrintPackingList()
{
  XSqlQuery packingPrintPackingList;
  if (_pack->altId() == -1)
  {
    QMessageBox::critical(this, tr("Shipment Number Required"),
			  tr("<p>Packing Lists may only be printed for "
			     "existing Shipments and there is no Shipment for "
			     "this Order. Issue Stock To Shipping to "
			     "create a Shipment."));
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  if (_pack->currentItem()->rawValue("pack_head_type") == "TO")
  {
    params.append("head_id",     _pack->id());
    params.append("shiphead_id", _pack->altId());
    params.append("head_type",   _pack->currentItem()->rawValue("pack_head_type"));
    params.append("print");
  }
  else
  {
    params.append("sohead_id",   _pack->id());
    params.append("shiphead_id", _pack->altId());
  }

  printPackingList newdlg(this, "", true);
  if (newdlg.set(params) == NoError_Print ||
      newdlg.exec() != XDialog::Rejected)
  {
    QString sql( "UPDATE pack "
		 "SET pack_printed=true "
		 "WHERE ((pack_head_id=<? value(\"head_id\") ?>)"
	       "  AND  (pack_head_type=<? value(\"head_type\") ?>)"
		 " <? if exists(\"shiphead_id\") ?>"
		 "  AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
		 " <? else ?>"
		 "  AND  (pack_shiphead_id IS NULL)"
		 "<? endif ?>"
		 ");" );

    ParameterList params;
    params.append("head_id",   _pack->id());
    params.append("head_type", _pack->currentItem()->rawValue("pack_head_type"));
    if (_pack->altId() > 0)
      params.append("shiphead_id", _pack->altId());

    MetaSQLQuery mql(sql);
    packingPrintPackingList = mql.toQuery(params);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Printing Packing List Batch"),
                                           packingPrintPackingList, __FILE__, __LINE__))
    {
      return;
    }

    sFillList();
  }
}

void packingListBatch::sFillList()
{
  XSqlQuery packingFillList;
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("packingListBatch", "detail");
  packingFillList = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Packing List Batch Information"),
                                           packingFillList, __FILE__, __LINE__))
  {
    return;
  }
  _pack->populate(packingFillList, true);
}

void packingListBatch::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

