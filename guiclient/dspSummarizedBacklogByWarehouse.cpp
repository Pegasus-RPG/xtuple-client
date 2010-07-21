/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBacklogByWarehouse.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "dspInventoryAvailabilityBySalesOrder.h"
#include "salesOrder.h"
#include "printPackingList.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"

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

  _so->addColumn(tr("Order#/Shipment#"),         _itemColumn, Qt::AlignLeft, true, "cohead_number");
  _so->addColumn(tr("Customer/Ship Via"),            -1, Qt::AlignLeft,  true, "cust_name");
  _so->addColumn(tr("Hold Type/Shipped"),   _orderColumn*2, Qt::AlignRight, true, "f_holdtype");
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
  _so->addColumn(tr("Sales Rep"),           _itemColumn, Qt::AlignRight, true, "salesrep_name");
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

  if(_showPrices->isChecked())
    params.append("showPrices");

  sHandlePrices(_showPrices->isChecked());

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("pack",   tr("Pack"));
  params.append("return", tr("Return"));
  params.append("ship",   tr("Ship"));
  params.append("other",  tr("Other"));
  params.append("yes", tr("Yes"));
  params.append("no", tr("No"));

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
  QAction *menuItem;

  if (_so->id() != -1)
  {
    menuItem = pMenu->addAction(tr("Inventory Availability by Sales Order..."), this, SLOT(sInventoryAvailabilityBySalesOrder()));
    if (!_privileges->check("ViewInventoryAvailability"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();
  
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    if (!_privileges->check("MaintainSalesOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
    if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
    if (!_privileges->check("MaintainSalesOrders"))
      menuItem->setEnabled(false);

  }

  if (_so->altId() > -1 ||
      (_so->id() != -1 && _so->currentItem()->text(2) != tr("Pack") &&
       _so->currentItem()->text(2) != tr("Credit")))
  {
    if (_so->id() != -1)
      pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
    if (!_privileges->check("PrintPackingLists"))
      menuItem->setEnabled(false);
  }
}

void dspSummarizedBacklogByWarehouse::sFillList()
{

  ParameterList params;
  if (setParams(params))
  {
    MetaSQLQuery mql = mqlLoad("summarizedBacklogByWarehouse", "detail");
    q = mql.toQuery(params);
    _so->populate(q);
    if (q.first())
    {
   
      MetaSQLQuery totm = mqlLoad("summarizedBacklogByWarehouse", "totals");
      q = totm.toQuery(params);
      if (q.first())
        _totalSalesOrders->setText(q.value("totalorders").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      MetaSQLQuery cntm = mqlLoad("summarizedBacklogByWarehouse", "counts");
      q = cntm.toQuery(params);
      if (q.first())
        _totalLineItems->setText(q.value("totalitems").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      MetaSQLQuery qtym = mqlLoad("summarizedBacklogByWarehouse", "qtys");
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

