/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspOrderActivityByProject.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include <metasql.h>
#include "mqlutil.h"

#include "guiclient.h"
#include "salesOrder.h"
#include "invoice.h"
#include "purchaseOrderItem.h"

dspOrderActivityByProject::dspOrderActivityByProject(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_project, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_project, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_showPo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showSo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showWo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_orders, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _orders->addColumn(tr("Type"),        _orderColumn, Qt::AlignLeft,   true,  "type"   );
  _orders->addColumn(tr("Order #"),     -1,           Qt::AlignLeft,   true,  "ordernumber"   );
  _orders->addColumn(tr("Status"),      _orderColumn, Qt::AlignCenter, true,  "status" );
  _orders->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _showPo->setChecked(true);
    _showSo->setChecked(true);
    _showWo->setChecked(true);
  }
}

dspOrderActivityByProject::~dspOrderActivityByProject()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspOrderActivityByProject::languageChange()
{
  retranslateUi(this);
}

void dspOrderActivityByProject::sPopulateMenu( QMenu * pMenu )
{
  QAction *menuItem;

  if(_orders->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(_orders->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Edit Quote..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes"));
  }

  if(_orders->altId() == 3)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(_orders->altId() == 5)
  {
    menuItem = pMenu->addAction(tr("Edit P/O Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View P/O Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

}

void dspOrderActivityByProject::sEdit()
{
  if(_orders->altId() == 1)
  {
    salesOrder::editSalesOrder(_orders->id(), false);
  }
  else if(_orders->altId() == 2)
  {
    ParameterList params;
    params.append("mode", "editQuote");
    params.append("quhead_id", _orders->id());

    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_orders->altId() == 3)
  {
    invoice::editInvoice(_orders->id());
  }
  else if(_orders->altId() == 5)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("poitem_id", _orders->id());

    purchaseOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sView()
{
  if(_orders->altId() == 1)
  {
    salesOrder::viewSalesOrder(_orders->id());
  }
  else if(_orders->altId() == 2)
  {
    ParameterList params;
    params.append("mode", "viewQuote");
    params.append("quhead_id", _orders->id());

    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(_orders->altId() == 3)
  {
    invoice::viewInvoice(_orders->id());
  }
  else if(_orders->altId() == 5)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("poitem_id", _orders->id());

    purchaseOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sFillList()
{
  if(_project->id() == -1)
  {
    _orders->clear();
    return;
  }

  if( (!_showSo->isChecked()) && (!_showWo->isChecked()) && (!_showPo->isChecked()) )
  {
    _orders->clear();
    return;
  }

  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("orderActivityByProject", "detail");
  q = mql.toQuery(params);

  _orders->populate(q, true);
}

void dspOrderActivityByProject::sPrint()
{
  ParameterList params;

  params.append("prj_id", _project->id());

  if (_showSo->isChecked())
    params.append("showSo");

  if (_showWo->isChecked())
    params.append("showWo");

  if (_showPo->isChecked())
    params.append("showPo");

  orReport report("OrderActivityByProject", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

bool dspOrderActivityByProject::setParams(ParameterList &params)
{
  params.append("prj_id", _project->id());
  
  params.append("so", tr("S/O"));
  params.append("wo", tr("W/O"));
  params.append("po", tr("P/O"));
  params.append("pr", tr("P/R"));
  params.append("quote", tr("Quote"));
  params.append("invoice", tr("Invoice"));

  if(_showSo->isChecked())
    params.append("showSo");

  if(_showWo->isChecked())
    params.append("showWo");

  if(_showPo->isChecked())
    params.append("showPo");

  return true;
}
