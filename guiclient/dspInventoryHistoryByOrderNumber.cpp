/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryHistoryByOrderNumber.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "adjustmentTrans.h"
#include "countTag.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "mqlutil.h"
#include "scrapTrans.h"
#include "transactionInformation.h"
#include "transferTrans.h"

dspInventoryHistoryByOrderNumber::dspInventoryHistoryByOrderNumber(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _invhist->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft,  true, "invhist_transdate");
  _invhist->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft,  false, "invhist_created");
  _invhist->addColumn(tr("Type"),               _transColumn, Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Site."),                _whsColumn, Qt::AlignLeft,  true, "warehous_code");
  _invhist->addColumn(tr("Item Number"),                  -1, Qt::AlignLeft,  true, "item_number");
  _invhist->addColumn(tr("UOM"),                  _uomColumn, Qt::AlignCenter,true, "invhist_invuom");
  _invhist->addColumn(tr("Trans-Qty"),            _qtyColumn, Qt::AlignRight, true, "invhist_invqty" );
  _invhist->addColumn(tr("Order #"),             _itemColumn, Qt::AlignCenter,true, "ordernumber");
  _invhist->addColumn(tr("QOH Before"),           _qtyColumn, Qt::AlignRight, false, "invhist_qoh_before");
  _invhist->addColumn(tr("QOH After"),            _qtyColumn, Qt::AlignRight, false, "invhist_qoh_after");
  _invhist->addColumn(tr("Cost Method"),          _qtyColumn, Qt::AlignLeft,  false, "invhist_costmethod");
  _invhist->addColumn(tr("Value Before"),         _qtyColumn, Qt::AlignRight,false, "invhist_value_before");
  _invhist->addColumn(tr("Value After"),          _qtyColumn, Qt::AlignRight,false, "invhist_value_after");
  _invhist->addColumn(tr("User"),               _orderColumn, Qt::AlignCenter,true, "invhist_user");

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);
}

dspInventoryHistoryByOrderNumber::~dspInventoryHistoryByOrderNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryHistoryByOrderNumber::languageChange()
{
  retranslateUi(this);
}

void dspInventoryHistoryByOrderNumber::setParams(ParameterList & params)
{
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("orderNumber", _orderNumber->text());
  params.append("transType", _transType->id());
}

void dspInventoryHistoryByOrderNumber::sPrint()
{
  if ( (!_dates->allValid()) || (_orderNumber->text().trimmed().length() == 0) )
  {
    QMessageBox::warning( this, tr("Invalid Data"),
                          tr("You must enter an Order Number along with a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  params.append("orderNumber", _orderNumber->text().trimmed());
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("transType", _transType->id());

  orReport report("InventoryHistoryByOrderNumber", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryHistoryByOrderNumber::sViewTransInfo()
{
  QString transtype(((XTreeWidgetItem *)_invhist->currentItem())->text(_invhist->column("invhist_transtype")));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

  if (transtype == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "CC")
  {
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else
  {
    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspInventoryHistoryByOrderNumber::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *)
{
  menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspInventoryHistoryByOrderNumber::sFillList()
{
  _invhist->clear();

  if (_orderNumber->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Order Search Pattern"),
                           tr("You must enter a Order # pattern to search for." ) );
    _orderNumber->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter an End Date.") );
    _dates->setFocus();
    return;
  }

  if (_orderNumber->text().trimmed().length())
  {
    ParameterList params;
    setParams(params);
    MetaSQLQuery mql = mqlLoad("inventoryHistoryByOrderNumber", "detail");

    q = mql.toQuery(params);
    _invhist->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
