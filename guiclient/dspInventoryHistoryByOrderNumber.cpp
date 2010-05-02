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
#include "salesOrderList.h"
#include "transferTrans.h"

dspInventoryHistoryByOrderNumber::dspInventoryHistoryByOrderNumber(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSalesOrderList()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_orderNumber, SIGNAL(requestList()), this, SLOT(sSalesOrderList()));

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  _invhist->setRootIsDecorated(TRUE);
  _invhist->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft, true, "invhist_transdate");
  _invhist->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft, false, "invhist_created");
  _invhist->addColumn(tr("Type"),        _transColumn,        Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Site"),        _whsColumn,          Qt::AlignCenter,true, "warehous_code");
  _invhist->addColumn(tr("Order #/Detail"),                -1,Qt::AlignLeft,  true, "orderlocation");
  _invhist->addColumn(tr("UOM"),         _uomColumn,          Qt::AlignCenter,true, "invhist_invuom");
  _invhist->addColumn(tr("Qty"),                  _qtyColumn, Qt::AlignRight, true, "transqty");
  _invhist->addColumn(tr("Value"),              _moneyColumn, Qt::AlignRight, true, "transvalue");
  _invhist->addColumn(tr("From Area"),   _orderColumn,        Qt::AlignLeft,  true, "locfrom");
  _invhist->addColumn(tr("QOH Before"),  _qtyColumn,          Qt::AlignRight, false, "qohbefore");
  _invhist->addColumn(tr("To Area"),     _orderColumn,        Qt::AlignLeft,  true, "locto");
  _invhist->addColumn(tr("QOH After"),   _qtyColumn,          Qt::AlignRight, false, "qohafter");
  _invhist->addColumn(tr("Cost Method"), _qtyColumn,          Qt::AlignLeft,  false, "costmethod");
  _invhist->addColumn(tr("Value Before"),_qtyColumn,          Qt::AlignRight, false, "invhist_value_before");
  _invhist->addColumn(tr("Value After"), _qtyColumn,          Qt::AlignRight, false, "invhist_value_after");
  _invhist->addColumn(tr("User"),        _orderColumn,        Qt::AlignCenter,false, "invhist_user");

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  if (_metrics->boolean("MultiWhs"))
    _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
}

dspInventoryHistoryByOrderNumber::~dspInventoryHistoryByOrderNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryHistoryByOrderNumber::languageChange()
{
  retranslateUi(this);
}


void dspInventoryHistoryByOrderNumber::sSalesOrderList()
{
  ParameterList params;
  params.append("sohead_id", _orderNumber->id());
  params.append("soType", cSoOpen);

  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if(id != QDialog::Rejected)
    _orderNumber->setId(id);
}

void dspInventoryHistoryByOrderNumber::setParams(ParameterList & params)
{
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

  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("orderNumber", _orderNumber->text());
  params.append("transType", _transType->id());

  params.append("average", tr("Average"));
  params.append("standard", tr("Standard"));
  params.append("job", tr("Job"));
  params.append("none", tr("None"));
  params.append("unknown", tr("Unknown"));
}

void dspInventoryHistoryByOrderNumber::sPrint()
{
  ParameterList params;
  setParams(params);
  if (!params.count())
    return;

  orReport report("InventoryHistory", params);
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

  if (_orderNumber->text().trimmed().length())
  {
    ParameterList params;
    setParams(params);
    if (!params.count())
      return;
    MetaSQLQuery mql = mqlLoad("inventoryHistory", "detail");

    q = mql.toQuery(params);
    _invhist->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
