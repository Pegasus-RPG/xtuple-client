/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspDetailedInventoryHistoryByLocation.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"
#include "mqlutil.h"

dspDetailedInventoryHistoryByLocation::dspDetailedInventoryHistoryByLocation(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_location, SIGNAL(newID(int)), this, SLOT(sPopulateLocationInfo(int)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulateLocations()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  
  if (_metrics->value("Application") != "PostBooks")
    _transType->append(cTransTransfers, tr("Transfers")              );
  
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);

  _invhist->addColumn(tr("Date"), (_dateColumn + 30), Qt::AlignRight, true, "invhist_transdate");
  _invhist->addColumn(tr("Type"),       _transColumn, Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Order #"),    _orderColumn, Qt::AlignLeft,  true, "ordernumber");
  _invhist->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _invhist->addColumn(tr("Lot/Serial #"), -1,         Qt::AlignLeft,  true, "lotserial");
  _invhist->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "invhist_invuom");
  _invhist->addColumn(tr("Trans-Qty"),    _qtyColumn, Qt::AlignRight, true, "transqty");
  _invhist->addColumn(tr("Qty. Before"),  _qtyColumn, Qt::AlignRight, true, "qohbefore");
  _invhist->addColumn(tr("Qty. After"),   _qtyColumn, Qt::AlignRight, true, "qohafter");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
  
  sPopulateLocations();
}

dspDetailedInventoryHistoryByLocation::~dspDetailedInventoryHistoryByLocation()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspDetailedInventoryHistoryByLocation::languageChange()
{
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLocation::sPopulateLocations()
{
  if (_warehouse->isAll())
    _location->populate( "SELECT location_id,"
                         "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (warehous_code || '-' || formatLocationName(location_id) || '-' || location_descrip)"
                         "            ELSE (warehous_code || '-' || formatLocationName(location_id))"
                         "       END AS locationname "
                         "FROM location, warehous "
                         "WHERE (location_warehous_id=warehous_id) "
                         "ORDER BY locationname;" );
  else
  {
    q.prepare( "SELECT location_id, "
               "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
               "            ELSE formatLocationName(location_id)"
               "       END AS locationname "
               "FROM location "
               "WHERE (location_warehous_id=:warehous_id) "
               "ORDER BY locationname;" );
    _warehouse->bindValue(q);
    q.exec();
    _location->populate(q);
  }
}

void dspDetailedInventoryHistoryByLocation::sPopulateLocationInfo(int pLocationid)
{
  q.prepare( "SELECT formatBoolYN(location_netable) AS netable,"
             "       formatBoolYN(location_restrict) AS restricted "
             "FROM location, warehous "
             "WHERE ( (location_warehous_id=warehous_id)"
             " AND (location_id=:location_id) );" );
  q.bindValue(":location_id", pLocationid);
  q.exec();
  if (q.first())
  {
    _netable->setText(q.value("netable").toString());
    _restricted->setText(q.value("restricted").toString());
  }
}

void dspDetailedInventoryHistoryByLocation::sPrint()
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
  _dates->appendValue(params);
  params.append("location_id", _location->id());
  params.append("transType", _transType->id());

  orReport report("DetailedInventoryHistoryByLocation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspDetailedInventoryHistoryByLocation::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(1));

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

void dspDetailedInventoryHistoryByLocation::sPopulateMenu(QMenu *menuThis)
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(1));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspDetailedInventoryHistoryByLocation::sFillList()
{
  ParameterList params;

  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("detailedInventoryHistory", "detail");
  q = mql.toQuery(params);
  _invhist->populate(q, true);
}

bool dspDetailedInventoryHistoryByLocation::setParams(ParameterList &params)
{
  params.append("byLocation");
 
  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }
  else if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }
  else
    _dates->appendValue(params);

  params.append("location_id", _location->id());

  params.append("transType", _transType->id());

  return true;
}
