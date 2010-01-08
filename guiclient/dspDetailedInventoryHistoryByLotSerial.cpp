/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspDetailedInventoryHistoryByLotSerial.h"

#include <QMenu>
#include <QStack>
#include <QMessageBox>

#include <openreports.h>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"

dspDetailedInventoryHistoryByLotSerial::dspDetailedInventoryHistoryByLotSerial(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);

  _invhist->addColumn(tr("Site"),         _whsColumn,          Qt::AlignCenter, true,  "lshist_warehous_code" );
  _invhist->addColumn(tr("Date"),         (_dateColumn + 30),  Qt::AlignRight,  true,  "lshist_transdate"  );
  _invhist->addColumn(tr("Type"),         _transColumn,        Qt::AlignCenter, true,  "lshist_transtype" );
  _invhist->addColumn(tr("Order #"),      _orderColumn,        Qt::AlignLeft,   true,  "lshist_ordernumber"   );
  _invhist->addColumn(tr("Item Number"),  _itemColumn,         Qt::AlignLeft,   true,  "lshist_item_number"   );
  _invhist->addColumn(tr("Location"),     _dateColumn,         Qt::AlignLeft,   true,  "lshist_locationname"   );
  _invhist->addColumn(tr("Lot/Serial #"), -1,                  Qt::AlignLeft,   true,  "lshist_lotserial"   );
  _invhist->addColumn(tr("UOM"),          _uomColumn,          Qt::AlignCenter, true,  "lshist_invuom" );
  _invhist->addColumn(tr("Trans-Qty"),    _qtyColumn,          Qt::AlignRight,  true,  "lshist_transqty"  );
  _invhist->addColumn(tr("Qty. Before"),  _qtyColumn,          Qt::AlignRight,  true,  "lshist_qty_before"  );
  _invhist->addColumn(tr("Qty. After"),   _qtyColumn,          Qt::AlignRight,  true,  "lshist_qty_after"  );
}

dspDetailedInventoryHistoryByLotSerial::~dspDetailedInventoryHistoryByLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspDetailedInventoryHistoryByLotSerial::languageChange()
{
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLotSerial::sPrint()
{
  QString trace;

  if (_dateGroup->isChecked())
  {
    if (!_dates->startDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter Start Date"),
                             tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter End Date"),
                             tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return;
    }
  }


  if ( ((_selected->isChecked() && _lotSerial->number().trimmed().length() == 0)
     || (_pattern->isChecked() && _lotSerialPattern->text().trimmed().length() == 0))
     && (!_item->isValid()) )
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial or Item criteria to print Inventory "
			     "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return;
  }

  if (_traceGroup->isChecked())
  {
    if (_forward->isChecked())
      trace="F";
    else
      trace="B";
  }
  else
    trace="N";

  ParameterList params;
  _dates->appendValue(params);
  if (_item->isValid())
    params.append("itemid", _item->id());
  if (_warehouse->isSelected())
    params.append("warehouseid", _warehouse->id());
  params.append("transType", _transType->id());
  params.append("trace", trace);
  if (_selected->isChecked())
  {
    if (_lotSerial->number().trimmed().length() > 0)
      params.append("lotSerial", _lotSerial->number().trimmed());
  }
  else
  {
    params.append("pattern");
    params.append("lotSerial", _lotSerialPattern->text().trimmed());
  }

  orReport report("DetailedInventoryHistoryByLotSerial", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspDetailedInventoryHistoryByLotSerial::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(2));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

  if (transType == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "CC")
  {
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspDetailedInventoryHistoryByLotSerial::sPopulateMenu(QMenu *menuThis)
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(2));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspDetailedInventoryHistoryByLotSerial::sFillList()
{
  QString trace;
  _invhist->clear();

  if (_dateGroup->isChecked())
  {
    if (!_dates->startDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter Start Date"),
                             tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::critical( this, tr("Enter End Date"),
                             tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return;
    }
  }


  if ( ((_selected->isChecked() && _lotSerial->number().trimmed().length() == 0)
     || (_pattern->isChecked() && _lotSerialPattern->text().trimmed().length() == 0))
     && (!_item->isValid()) )
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial or Item criteria to view Inventory "
			     "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return;
  }


  if (_traceGroup->isChecked())
  {
    if (_forward->isChecked())
      trace="F";
    else
      trace="B";
  }
  else
    trace="N";

  q.prepare( "SELECT * FROM lshist(:itemid,:warehouseid,:lotserial,:pattern,:transType,:startDate,:endDate,:trace,1); ");
  if (_dateGroup->isChecked())
    _dates->bindValue(q);
  if (_item->isValid())
    q.bindValue(":itemid", _item->id());
  if (_warehouse->isSelected())
    q.bindValue(":warehouseid", _warehouse->id());
  if (_selected->isChecked())
  {
    if (_lotSerial->number().trimmed().length() > 0)
      q.bindValue(":lotserial", _lotSerial->number().trimmed());
    q.bindValue(":pattern", FALSE);
  }
  else
  {   
    q.bindValue(":lotserial", _lotSerialPattern->text().trimmed());
    q.bindValue(":pattern", TRUE);
  }
  q.bindValue(":transType", _transType->id());
  q.bindValue(":trace", trace);
  q.exec();
  _invhist->populate(q);
  _invhist->expandAll();
}

