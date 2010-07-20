/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryHistoryByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "adjustmentTrans.h"
#include "countTag.h"
#include "expenseTrans.h"
#include "inputManager.h"
#include "materialReceiptTrans.h"
#include "mqlutil.h"
#include "scrapTrans.h"
#include "transactionInformation.h"
#include "transferTrans.h"
#include "workOrder.h"

dspInventoryHistoryByItem::dspInventoryHistoryByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _invhist->setRootIsDecorated(TRUE);
  _invhist->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft, true, "invhist_transdate");
  _invhist->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft, false, "invhist_created");
  _invhist->addColumn(tr("Type"),        _transColumn,        Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Site"),        _whsColumn,          Qt::AlignCenter,true, "warehous_code");
  _invhist->addColumn(tr("Order #/Detail"), -1,Qt::AlignLeft,  true, "orderlocation");
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

  _orderType->append(0, tr("All Orders"),        "" );
  _orderType->append(1, tr("Sales Orders"),      "SO" );
  _orderType->append(2, tr("Purchase Orders"),   "PO" );
  _orderType->append(3, tr("Work Orders"),       "WO");

  if (_metrics->boolean("MultiWhs"))
  {
    _transType->append(cTransTransfers, tr("Transfers") );
    _orderType->append(4, tr("Transfer Orders"), "TO" );
  }

  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);
  _orderType->setCurrentIndex(0);

  _item->setFocus();
}

dspInventoryHistoryByItem::~dspInventoryHistoryByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryHistoryByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryHistoryByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setEnabled(FALSE);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);

    _dates->setFocus();
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("transtype", &valid);
  if (valid)
  {
    QString transtype = param.toString();

    if (transtype == "R")
      _transType->setCurrentIndex(1);
    else if (transtype == "I")
      _transType->setCurrentIndex(2);
    else if (transtype == "S")
      _transType->setCurrentIndex(3);
    else if (transtype == "A")
      _transType->setCurrentIndex(4);
    else if (transtype == "T")
      _transType->setCurrentIndex(5);
    else if (transtype == "SC")
      _transType->setCurrentIndex(6);
  }

  param = pParams.value("ordertype", &valid);
  if (valid)
  {
    QString ordertype = param.toString();

    if (ordertype == "SO")
      _orderType->setCurrentIndex(1);
    else if (ordertype == "PO")
      _orderType->setCurrentIndex(2);
    else if (ordertype == "WO")
      _orderType->setCurrentIndex(3);
    else if (ordertype == "TO")
      _orderType->setCurrentIndex(4);
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInventoryHistoryByItem::setParams(ParameterList & params)
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

  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("transType", _transType->id());

  if (_orderType->currentIndex())
    params.append("orderType", _orderType->code());

  params.append("average", tr("Average"));
  params.append("standard", tr("Standard"));
  params.append("job", tr("Job"));
  params.append("none", tr("None"));
  params.append("unknown", tr("Unknown"));
}

void dspInventoryHistoryByItem::sPrint()
{
  ParameterList params;
  setParams(params);
  if (!params.count())
    return;

  orReport report("InventoryHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryHistoryByItem::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(_invhist->column("invhist_transtype")));

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
  else
  {
    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspInventoryHistoryByItem::sEditTransInfo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invhist_id", _invhist->id());

  transactionInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryHistoryByItem::sViewWOInfo()
{
  QString orderNumber = _invhist->currentItem()->text(_invhist->column("ordernumber"));
  int sep1            = orderNumber.indexOf('-');
  int sep2            = orderNumber.indexOf('-', (sep1 + 1));
  int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
  int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

  q.prepare( "SELECT wo_id "
             "FROM wo "
             "WHERE ( (wo_number=:wo_number)"
             " AND (wo_subnumber=:wo_subnumber) );" );
  q.bindValue(":wo_number", mainNumber);
  q.bindValue(":wo_subnumber", subNumber);
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", q.value("wo_id"));

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspInventoryHistoryByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()));
  menuItem = pMenu->addAction(tr("Edit Transaction Information..."), this, SLOT(sEditTransInfo()));

  if ( (pItem->text(_invhist->column("warehous_code")).length()) &&
       ( (pItem->text(_invhist->column("invhist_transtype")) == "RM") || (pItem->text(_invhist->column("invhist_transtype")) == "IM") ) )
  {
    QString orderNumber = _invhist->currentItem()->text(_invhist->column("ordernumber"));
    int sep1            = orderNumber.indexOf('-');
    int sep2            = orderNumber.indexOf('-', (sep1 + 1));
    int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
    int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

    if ( (mainNumber) && (subNumber) )
    {
      menuItem = pMenu->addAction(tr("View Work Order Information..."), this, SLOT(sViewWOInfo()));
      if ((!_privileges->check("MaintainWorkOrders")) && (!_privileges->check("ViewWorkOrders")))
        menuItem->setEnabled(false);
    }
  }
}

void dspInventoryHistoryByItem::sFillList()
{
  _invhist->clear();

  ParameterList params;
  setParams(params);
  if (!params.count())
    return;

  params.append("includeFormatted");

  if (_item->isValid())
  {
    ParameterList params;
    setParams(params);
    if (!params.count())
      return;
    MetaSQLQuery mql = mqlLoad("inventoryHistory", "detail");
    q = mql.toQuery(params);
    if (q.first())
    {
      _invhist->populate(q, true);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
