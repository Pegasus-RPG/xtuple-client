/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPartiallyShippedOrders.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "printPackingList.h"
#include "salesOrder.h"

#define AMOUNT_COL	7
#define AMOUNT_CURR_COL	8
#define BASEAMOUNT_COL	9

dspPartiallyShippedOrders::dspPartiallyShippedOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _so->addColumn(tr("Hold"),        0,           Qt::AlignCenter,true, "cohead_holdtype");
  _so->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignRight, true, "cohead_number");
  _so->addColumn(tr("Customer"),    -1,          Qt::AlignLeft,  true, "cust_name");
  _so->addColumn(tr("Hold Type"),   _dateColumn, Qt::AlignCenter,true, "f_holdtype");
  _so->addColumn(tr("Ordered"),     _dateColumn, Qt::AlignRight, true, "cohead_orderdate");
  _so->addColumn(tr("Scheduled"),   _dateColumn, Qt::AlignRight, true, "minscheddate");
  _so->addColumn(tr("Pack Date"),   _dateColumn, Qt::AlignRight, true, "cohead_packdate");
  _so->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "extprice");
  _so->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currAbbr");
  _so->addColumn(tr("Amount\n(%1)").arg(CurrDisplay::baseCurrAbbr()),
                                   _moneyColumn, Qt::AlignRight, true, "extprice_base");

  sHandlePrices(_showPrices->isChecked());

  if ( (!_privileges->check("ViewCustomerPrices")) && (!_privileges->check("MaintainCustomerPrices")) )
    _showPrices->setEnabled(FALSE);

  sFillList();
}

dspPartiallyShippedOrders::~dspPartiallyShippedOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPartiallyShippedOrders::languageChange()
{
  retranslateUi(this);
}

void dspPartiallyShippedOrders::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
  {
    _so->showColumn(AMOUNT_COL);
    if (!omfgThis->singleCurrency())
      _so->showColumn(AMOUNT_CURR_COL);
    if (!omfgThis->singleCurrency())
      _so->showColumn(BASEAMOUNT_COL);
  }
  else
  {
    _so->hideColumn(AMOUNT_COL);
    _so->hideColumn(AMOUNT_CURR_COL);
    _so->hideColumn(BASEAMOUNT_COL);
  }
}

bool dspPartiallyShippedOrders::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  if (_dates->allValid())
    _dates->appendValue(params);
  else
    return false;

  if(_showPrices->isChecked())
    params.append("showPrices");

  params.append("none",   tr("None"));
  params.append("credit", tr("Credit"));
  params.append("pack",   tr("Pack"));
  params.append("return", tr("Return"));
  params.append("ship",   tr("Ship"));
  params.append("other",  tr("Other"));

  if (omfgThis->singleCurrency())
    params.append("singlecurrency");

  return true;
}

void dspPartiallyShippedOrders::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("PartiallyShippedOrders", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPartiallyShippedOrders::sEditOrder()
{
  salesOrder::editSalesOrder(_so->altId(), false);
}

void dspPartiallyShippedOrders::sViewOrder()
{
  salesOrder::viewSalesOrder(_so->altId());
}

void dspPartiallyShippedOrders::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", _so->altId());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPartiallyShippedOrders::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
  menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                       _privileges->check("ViewSalesOrders"));

  pMenu->addSeparator();

  if ( (_so->currentItem()->text(0) != "P") && (_so->currentItem()->text(0) != "C") )
  {
    menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
    menuItem->setEnabled(_privileges->check("PrintPackingLists"));
  }
}

void dspPartiallyShippedOrders::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("partiallyShippedOrders", "detail");
  q = mql.toQuery(params);
  _so->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _so->setDragString("soheadid=");
}
