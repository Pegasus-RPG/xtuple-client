/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryHistoryByParameterList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "adjustmentTrans.h"
#include "countTag.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "mqlutil.h"
#include "scrapTrans.h"
#include "transactionInformation.h"
#include "transferTrans.h"
#include "workOrder.h"

dspInventoryHistoryByParameterList::dspInventoryHistoryByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _invhist->setRootIsDecorated(TRUE);
  _invhist->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft,  true, "invhist_transdate");
  _invhist->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft,  false, "invhist_created");
  _invhist->addColumn(tr("Site"),                 _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _invhist->addColumn(tr("Item Number"),                  -1, Qt::AlignLeft,  true, "item_number");
  _invhist->addColumn(tr("Type"),               _transColumn, Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Order #"),             _itemColumn, Qt::AlignCenter,true, "orderlocation");
  _invhist->addColumn(tr("UOM"),                  _uomColumn, Qt::AlignCenter,true, "invhist_invuom");
  _invhist->addColumn(tr("Trans-Qty"),            _qtyColumn, Qt::AlignRight, true, "transqty");
  _invhist->addColumn(tr("From Area"),          _orderColumn, Qt::AlignLeft,  true, "locfrom");
  _invhist->addColumn(tr("QOH Before"),           _qtyColumn, Qt::AlignRight, false, "qohbefore");
  _invhist->addColumn(tr("To Area"),            _orderColumn, Qt::AlignLeft,  true, "locto");
  _invhist->addColumn(tr("QOH After"),            _qtyColumn, Qt::AlignRight, false, "qohafter");
  _invhist->addColumn(tr("Cost Method"),          _qtyColumn, Qt::AlignLeft,  false, "costmethod");
  _invhist->addColumn(tr("Value Before"),         _qtyColumn, Qt::AlignRight, false, "invhist_value_before");
  _invhist->addColumn(tr("Value After"),          _qtyColumn, Qt::AlignRight, false, "invhist_value_after");
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

dspInventoryHistoryByParameterList::~dspInventoryHistoryByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryHistoryByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryHistoryByParameterList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ItemGroup);

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

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

  if (pParams.inList("run"))
    sFillList();

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Inventory History by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Inventory History by Planner Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Inventory History by Item Group"));
      break;

    default:
      break;
  }

  return NoError;
}

void dspInventoryHistoryByParameterList::setParams(ParameterList & params)
{
  _warehouse->appendValue(params);
  _parameter->appendValue(params);
  _dates->appendValue(params);
  params.append("transType", _transType->id());

  if (_parameter->type() == ParameterGroup::ItemGroup)
    params.append("itemgrp");
  else if(_parameter->type() == ParameterGroup::PlannerCode)
    params.append("plancode");
  else
    params.append("classcode");
}

void dspInventoryHistoryByParameterList::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _parameter->appendValue(params);
  _dates->appendValue(params);
  params.append("transType", _transType->id());

  if (_parameter->isAll())
  {
    switch(_parameter->type())
    {
    case ParameterGroup::ItemGroup:
      params.append("itemgrp");
      break;
    case ParameterGroup::ClassCode:
      params.append("classcode");
      break;
    case ParameterGroup::PlannerCode:
      params.append("plancode");
      break;
    default:
      break;
    }
  }

  orReport report("InventoryHistoryByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryHistoryByParameterList::sViewTransInfo()
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

void dspInventoryHistoryByParameterList::sEditTransInfo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invhist_id", _invhist->id());

  transactionInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryHistoryByParameterList::sViewWOInfo()
{
  QString orderNumber = _invhist->currentItem()->text(_invhist->column("ordernumber"));
  int sep1            = orderNumber.find('-');
  int sep2            = orderNumber.find('-', (sep1 + 1));
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
}

void dspInventoryHistoryByParameterList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
  menuItem = pMenu->insertItem(tr("Edit Transaction Information..."), this, SLOT(sEditTransInfo()), 0);

  if ( (pItem->text(_invhist->column("warehous_code")).length()) &&
       ( (pItem->text(_invhist->column("invhist_transtype")) == "RM") || (pItem->text(_invhist->column("invhist_transtype")) == "IM") ) )
  {
    QString orderNumber = _invhist->currentItem()->text(_invhist->column("ordernumber"));
    int sep1            = orderNumber.find('-');
    int sep2            = orderNumber.find('-', (sep1 + 1));
    int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
    int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

    if ( (mainNumber) && (subNumber) )
    {
      q.prepare( "SELECT wo_id "
                 "FROM wo "
                 "WHERE ( (wo_number=:wo_number)"
                 " AND (wo_subnumber=:wo_subnumber) );" );
      q.bindValue(":wo_number", mainNumber);
      q.bindValue(":wo_subnumber", subNumber);
      q.exec();
      if (q.first())
      {
          menuItem = pMenu->insertItem(tr("View Work Order Information..."), this, SLOT(sViewWOInfo()), 0);
          if ((!_privileges->check("MaintainWorkOrders")) && (!_privileges->check("ViewWorkOrders")))
            pMenu->setItemEnabled(menuItem, FALSE);
      }
    }
  }
}

void dspInventoryHistoryByParameterList::sFillList()
{
  _invhist->clear();

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

  ParameterList params;
  setParams(params);
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
