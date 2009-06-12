/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBacklogByWarehouse.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "dspInventoryAvailabilityBySalesOrder.h"
#include "salesOrder.h"
#include "rescheduleSoLineItems.h"
#include "printPackingList.h"
#include "storedProcErrorLookup.h"

dspSummarizedBacklogByWarehouse::dspSummarizedBacklogByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _customerType->setType(ParameterGroup::CustomerType);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _so->addColumn(tr("S/O #/Shipped"),       _itemColumn, Qt::AlignRight, true, "cohead_number");
  _so->addColumn(tr("Customer/Ship Via"),            -1, Qt::AlignLeft,  true, "cust_name");
  _so->addColumn(tr("Hold Type/Ship #"), _orderColumn*2, Qt::AlignRight, true, "f_holdtype");
  _so->addColumn(tr("Ordered/Shipped"),     _dateColumn, Qt::AlignRight, true, "cohead_orderdate");
  _so->addColumn(tr("Scheduled"),           _dateColumn, Qt::AlignRight, true, "scheddate");
  _so->addColumn(tr("Pack Date"),           _dateColumn, Qt::AlignRight, true, "cohead_packdate");
  if (_privileges->check("ViewCustomerPrices") ||
      _privileges->check("MaintainCustomerPrices"))
  {
    _so->addColumn(tr("Sales"),  _moneyColumn, Qt::AlignRight, true, "sales");
    _so->addColumn(tr("Cost"),   _moneyColumn, Qt::AlignRight, true, "cost");
    _so->addColumn(tr("Margin"), _moneyColumn, Qt::AlignRight, true, "margin");
  }
  _so->addColumn(tr("Time Received"),       _dateColumn, Qt::AlignRight, false, "cohead_created");
  _so->addColumn(tr("Pack List Batch"),     _dateColumn, Qt::AlignRight, false, "packed");

  _so->setRootIsDecorated(TRUE);
  _so->setDragString("soheadid=");

  if ( (!_privileges->check("ViewCustomerPrices")) && (!_privileges->check("MaintainCustomerPrices")) )
    _showPrices->setEnabled(FALSE);
  sHandlePrices(_showPrices->isChecked());

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

dspSummarizedBacklogByWarehouse::~dspSummarizedBacklogByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSummarizedBacklogByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedBacklogByWarehouse::sHandlePrices(bool pShowPrices)
{
  if(pShowPrices){
    _so->showColumn("sales");
    _so->showColumn("cost");
    _so->showColumn("margin");
  }
  else{
    _so->hideColumn("sales");
    _so->hideColumn("cost");
    _so->hideColumn("margin");
  }
}

bool dspSummarizedBacklogByWarehouse::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  _customerType->appendValue(params);

  if (_dates->allValid())
    _dates->appendValue(params);
  else
    return false;

  if (_shipDate->isChecked())
    params.append("orderByShipDate");
  else if (_packDate->isChecked())
    params.append("orderByPackDate");
  else if (_orderNumber->isChecked())
    params.append("orderByOrderNumber");

  if(_showPrices->isChecked())
    params.append("showPrices");

  sHandlePrices(_showPrices->isChecked());

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("pack",   tr("Pack"));
  params.append("return", tr("Return"));
  params.append("ship",   tr("Ship"));
  params.append("other",  tr("Other"));

  return true;
}

void dspSummarizedBacklogByWarehouse::sPrint()
{
  ParameterList params;
  if (setParams(params))
  {
    orReport report("SummarizedBacklogByWarehouse", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
}

void dspSummarizedBacklogByWarehouse::sInventoryAvailabilityBySalesOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _so->id());
      
  dspInventoryAvailabilityBySalesOrder *newdlg = new dspInventoryAvailabilityBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedBacklogByWarehouse::sEdit()
{
  salesOrder::editSalesOrder(_so->id(), false);
}

void dspSummarizedBacklogByWarehouse::sView()
{
  salesOrder::viewSalesOrder(_so->id());
}

void dspSummarizedBacklogByWarehouse::sReschedule()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
      
  rescheduleSoLineItems newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspSummarizedBacklogByWarehouse::sDelete()
{
  if ( QMessageBox::question(this, tr("Delete Sales Order?"),
                             tr("<p>Are you sure that you want to completely "
			     "delete the selected Sales Order?"),
			     QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteSo(:sohead_id) AS result;");
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      switch (result)
      {
        case 0:
          omfgThis->sSalesOrdersUpdated(-1);
          break;

        case -1:
          if (QMessageBox::question(this, tr("Cannot Delete Sales Order"),
				    storedProcErrorLookup("deleteSO", result) +
				       tr("<br>Would you like to Close the "
					  "selected Sales Order instead?" ),
				    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
          {
            q.prepare( "UPDATE coitem "
                       "SET coitem_status='C' "
                       "WHERE ((coitem_status<>'X')"
		       "   AND (coitem_cohead_id=:sohead_id));" );
            q.bindValue(":sohead_id", _so->id());
            q.exec();
	    if (q.lastError().type() != QSqlError::NoError)
	    {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
      
            sFillList();
          }

          break;

        default:
          systemError(this, storedProcErrorLookup("deleteSO", result),
		      __FILE__, __LINE__);
	  return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void dspSummarizedBacklogByWarehouse::sPrintPackingList()
{
  ParameterList params;
  if (_so->altId() > 0)
    params.append("cosmisc_id", _so->altId());
  else
    params.append("sohead_id", _so->id());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSummarizedBacklogByWarehouse::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_so->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Inventory Availability by Sales Order..."), this, SLOT(sInventoryAvailabilityBySalesOrder()), 0);
    if (!_privileges->check("ViewInventoryAvailability"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reschedule..."), this, SLOT(sReschedule()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

  }

  if (_so->altId() > -1 ||
      (_so->id() != -1 && _so->currentItem()->text(2) != tr("Pack") &&
       _so->currentItem()->text(2) != tr("Credit")))
  {
    if (_so->id() != -1)
      pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
    if (!_privileges->check("PrintPackingLists"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspSummarizedBacklogByWarehouse::sFillList()
{

  ParameterList params;
  if (setParams(params))
  {
    QString sql( "(SELECT cohead_id, 0 AS xtindentrole, salesrep_name, cohead_holdtype, cohead_number, "
   "CASE WHEN ((cosmisc_shipped) AND (COALESCE(cobmisc_cohead_id,0) > 0)       "
   "AND (SUM(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) <= 0)) THEN 'green'"
   "WHEN ( (COALESCE(cobmisc_cohead_id,0) > 0)       "
   "AND (SUM(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) > 0)       "
   ") THEN 'red' "
   "WHEN (NOT cosmisc_shipped) THEN 'blue' "
   "END AS qtforegroundrole, "
   "cust_name,       "
   "cohead_created, cohead_orderdate, cohead_packdate, pack_head_id,       "
   "       CASE WHEN (cohead_holdtype='N') THEN <? value(\"Pnone\") ?>"
   "            WHEN (cohead_holdtype='C') THEN <? value(\"Pcredit\") ?>"
   "            WHEN (cohead_holdtype='S') THEN <? value(\"Pship\") ?>"
   "            WHEN (cohead_holdtype='P') THEN <? value(\"Ppack\") ?>"
   "            WHEN (cohead_holdtype='R') THEN <? value(\"Preturn\") ?>"
   "            ELSE <? value(\"Pother\") ?>"
   "END AS f_holdtype,       "
   "MIN(coitem_scheddate) AS scheddate,       "
   "SUM((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *            "
   "            (currToBase(cohead_curr_id, coitem_price, cohead_orderdate) / coitem_price_invuomratio) ) AS sales, "
   "       SUM((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) * stdcost(item_id) ) AS cost,"
   "       SUM((noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
   "            ((currToBase(cohead_curr_id, coitem_price, cohead_orderdate) / coitem_price_invuomratio) - stdcost(item_id)) ) AS margin,"
   "       'curr' AS sales_xtnumericrole,"
   "       'curr' AS cost_xtnumericrole,"
   "       'curr' AS margin_xtnumericrole,"
   "       COALESCE(cosmisc_id, -1) AS cosmisc_id, "
   "       formatShipmentNumber(cosmisc_id) AS cosmisc_number, "
   "       CASE WHEN (cosmisc_shipped IS NULL) THEN 0"
   "            WHEN (cosmisc_shipped) THEN 1"
   "            WHEN (NOT cosmisc_shipped) THEN 2"
   "       END AS shipstatus,"
   "       COALESCE(cosmisc_shipvia, '') AS shipvia,"
   "       CASE WHEN (cosmisc_shipdate IS NULL) THEN ''"
   "            ELSE formatDate(cosmisc_shipdate)"
   "       END AS shipdate,"
   "		 formatboolyn(CASE WHEN (pack_head_id IS NOT NULL) THEN TRUE"
   "            ELSE FALSE"
   " END) AS packed "
   "<? if exists(\"showPrices\") ?> "
   " , 0 AS sales_xttotalrole, 0 AS cost_xttotalrole, 0 AS margin_xttotalrole "
   "<? endif ?>"
   "FROM coitem, itemsite, item, cust, cohead "
   "  LEFT OUTER JOIN cosmisc ON (cosmisc_cohead_id=cohead_id) "
   "  LEFT OUTER JOIN (SELECT DISTINCT cobmisc_cohead_id FROM cobmisc) AS cobmisc ON (cobmisc_cohead_id=cohead_id) "
   "  LEFT OUTER JOIN pack ON (cohead_id = pack_head_id), salesrep "
   "WHERE ( (coitem_cohead_id=cohead_id)"
   " AND (salesrep_id = cohead_salesrep_id) "
   " AND (cohead_cust_id=cust_id)"
   " AND (coitem_itemsite_id=itemsite_id)"
   " AND (itemsite_item_id=item_id)"
   " AND (coitem_status NOT IN ('C','X'))"
   " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
   "                           AND <? value(\"endDate\") ?>)"
   "<? if exists(\"warehous_id\") ?>"
   " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
   "<? endif ?>"
   "<? if exists(\"custtype_id\") ?>"
   " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
   "<? elseif exists(\"custtype_pattern\") ?>"
   " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
   "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
   "<? endif ?>"
   ") "
   "GROUP BY cohead_id, salesrep_number, salesrep_name, cohead_number, cust_name,"
   "         cohead_holdtype, cohead_orderdate, cohead_packdate,"
   "         cosmisc_shipped, cosmisc_shipvia, cosmisc_shipdate,"
   "         cosmisc_id, cobmisc_cohead_id, cohead_created, pack_head_id "
   " ORDER BY "
   "<? if exists(\"orderByShipDate\") ?>scheddate,"
   "<? elseif exists(\"orderByPackDate\") ?>cohead_packdate,"
   "<? endif ?>"
   "          cohead_number, cosmisc_shipped )"
   "UNION "
   "(SELECT cohead_id, 1 AS xtindentrole, NULL, NULL, "
   "CASE WHEN (cosmisc_shipped IS NULL) THEN ''            "
   "WHEN (cosmisc_shipped) THEN 'Yes'            "
   "WHEN (NOT cosmisc_shipped) THEN 'No'       "
   "END AS cohead_number, "
   "CASE WHEN (cosmisc_shipped IS NULL) THEN ''           "
   "WHEN (cosmisc_shipped) THEN 'green'            "
   "WHEN (NOT cosmisc_shipped) THEN 'blue' "
   "WHEN ( (COALESCE(cobmisc_cohead_id,0) > 0)       "
   "AND (SUM(noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) > 0)       "
   ") THEN 'red'       "
   "END AS qtforegroundrole, "
   "COALESCE(cosmisc_shipvia, ''), NULL, NULL, NULL, NULL, NULL,  MIN(coitem_scheddate), NULL, NULL, NULL, NULL, NULL, NULL, NULL,        "
   "formatShipmentNumber(cosmisc_id) AS cosmisc_number,        "
   "NULL,       "
   "COALESCE(cosmisc_shipvia, '') AS shipvia,       "
   "CASE WHEN (cosmisc_shipdate IS NULL) THEN ''            "
   "ELSE formatDate(cosmisc_shipdate)       "
   "END AS shipdate        "
   "<? if exists(\"showPrices\") ?> "
   " , NULL, NULL, NULL "
   "<? endif ?>"
   ", NULL "
   "FROM coitem, itemsite, item, cust, cohead   "
   "LEFT OUTER JOIN cosmisc ON (cosmisc_cohead_id=cohead_id)   "
   "LEFT OUTER JOIN (SELECT DISTINCT cobmisc_cohead_id FROM cobmisc) AS cobmisc ON (cobmisc_cohead_id=cohead_id)   "
   "LEFT OUTER JOIN pack ON (cohead_id = pack_head_id), salesrep "
   "WHERE ( (coitem_cohead_id=cohead_id)"
   " AND (salesrep_id = cohead_salesrep_id) "
   " AND (cohead_cust_id=cust_id)"
   " AND (coitem_itemsite_id=itemsite_id)"
   " AND (itemsite_item_id=item_id)"
   " AND (coitem_status NOT IN ('C','X'))"
   " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
   "                           AND <? value(\"endDate\") ?>)"
   "<? if exists(\"warehous_id\") ?>"
   " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
   "<? endif ?>"
   "<? if exists(\"custtype_id\") ?>"
   " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
   "<? elseif exists(\"custtype_pattern\") ?>"
   " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
   "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
   "<? endif ?>"
   "AND (cosmisc_shipped IS NOT NULL)) "
   "GROUP BY cohead_id, salesrep_number, salesrep_name, cohead_number, cust_name,         "
   "cohead_holdtype, cohead_orderdate, cohead_packdate,         "
   "cosmisc_shipped, cosmisc_shipvia, cosmisc_shipdate,         "
   "cosmisc_id, cobmisc_cohead_id, cohead_created, pack_head_id  ORDER BY           "
   "cohead_number, cosmisc_shipped)");

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.first())
    {
   
      QString tots = "SELECT COUNT(cohead_id) AS totalorders "
		     "FROM ( SELECT DISTINCT cohead_id "
		     "       FROM cohead, coitem, itemsite, cust "
		     "       WHERE ( (coitem_cohead_id=cohead_id)"
		     "        AND (coitem_itemsite_id=itemsite_id)"
		     "        AND (cohead_cust_id=cust_id)"
		     "        AND (coitem_status NOT IN ('C','X'))"
		     " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		     "                           AND <? value(\"endDate\") ?>)"
		     "<? if exists(\"warehous_id\") ?>"
		     " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		     "<? endif ?>"
		     "<? if exists(\"custtype_id\") ?>"
		     " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		     "<? elseif exists(\"custtype_pattern\") ?>"
		     " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		     "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		     "<? endif ?>"
		     ") ) AS data;";

      MetaSQLQuery totm(tots);
      q = totm.toQuery(params);
      if (q.first())
        _totalSalesOrders->setText(q.value("totalorders").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      QString cnts = "SELECT COUNT(coitem.*) AS totalitems "
		     "FROM cohead, coitem, itemsite, cust "
		     "WHERE ( (coitem_cohead_id=cohead_id)"
		     " AND (coitem_itemsite_id=itemsite_id)"
		     " AND (cohead_cust_id=cust_id)"
		     " AND (coitem_status NOT IN ('C','X'))"
		     " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		     "                           AND <? value(\"endDate\") ?>)"
		     "<? if exists(\"warehous_id\") ?>"
		     " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		     "<? endif ?>"
		     "<? if exists(\"custtype_id\") ?>"
		     " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		     "<? elseif exists(\"custtype_pattern\") ?>"
		     " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		     "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		     "<? endif ?>"
		     ");";

      MetaSQLQuery cntm(cnts);
      q = cntm.toQuery(params);
      if (q.first())
        _totalLineItems->setText(q.value("totalitems").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      QString qtys = "SELECT formatQty(SUM(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned)) AS f_totalqty "
		     "FROM cohead, coitem, itemsite, item, cust "
		     "WHERE ( (coitem_cohead_id=cohead_id)"
		     " AND (coitem_itemsite_id=itemsite_id)"
		     " AND (itemsite_item_id=item_id)"
		     " AND (cohead_cust_id=cust_id)"
		     " AND (coitem_status NOT IN ('C','X'))"
		     " AND (coitem_scheddate BETWEEN <? value(\"startDate\") ?>"
		     "                           AND <? value(\"endDate\") ?>)"
		     "<? if exists(\"warehous_id\") ?>"
		     " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
		     "<? endif ?>"
		     "<? if exists(\"custtype_id\") ?>"
		     " AND (cust_custtype_id=<? value(\"custtype_id\") ?>)"
		     "<? elseif exists(\"custtype_pattern\") ?>"
		     " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype"
		     "  WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>)))"
		     "<? endif ?>"
		     ");";

      MetaSQLQuery qtym(qtys);
      q = qtym.toQuery(params);
      if (q.first())
        _totalQty->setText(q.value("f_totalqty").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    else
    {
      _totalSalesOrders->clear();
      _totalLineItems->clear();
      _totalQty->clear();
    }
  }
}

