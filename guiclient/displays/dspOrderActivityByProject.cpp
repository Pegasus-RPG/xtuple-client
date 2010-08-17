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

#include "guiclient.h"
#include "salesOrder.h"
#include "invoice.h"
#include "purchaseOrderItem.h"

dspOrderActivityByProject::dspOrderActivityByProject(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspOrderActivityByProject", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Order Activity by Project"));
  setListLabel(tr("Orders"));
  setReportName("OrderActivityByProject");
  setMetaSQLOptions("orderActivityByProject", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Type"),        _orderColumn, Qt::AlignLeft,   true,  "type"   );
  list()->addColumn(tr("Order #"),     -1,           Qt::AlignLeft,   true,  "ordernumber"   );
  list()->addColumn(tr("Status"),      _orderColumn, Qt::AlignCenter, true,  "status" );
  list()->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _showPo->setChecked(true);
    _showSo->setChecked(true);
    _showWo->setChecked(true);
  }
}

void dspOrderActivityByProject::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspOrderActivityByProject::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if(list()->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(list()->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Edit Quote..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes"));
  }

  if(list()->altId() == 3)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(list()->altId() == 5)
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
  if(list()->altId() == 1)
  {
    salesOrder::editSalesOrder(list()->id(), false);
  }
  else if(list()->altId() == 2)
  {
    ParameterList params;
    params.append("mode", "editQuote");
    params.append("quhead_id", list()->id());

    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 3)
  {
    invoice::editInvoice(list()->id());
  }
  else if(list()->altId() == 5)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("poitem_id", list()->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sView()
{
  if(list()->altId() == 1)
  {
    salesOrder::viewSalesOrder(list()->id());
  }
  else if(list()->altId() == 2)
  {
    ParameterList params;
    params.append("mode", "viewQuote");
    params.append("quhead_id", list()->id());

    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 3)
  {
    invoice::viewInvoice(list()->id());
  }
  else if(list()->altId() == 5)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("poitem_id", list()->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

bool dspOrderActivityByProject::setParams(ParameterList &params)
{
  if(_project->id() == -1)
  {
    QMessageBox::warning(this, tr("Project Required"),
      tr("You must specify a Project."));
    return false;
  }

  if( (!_showSo->isChecked()) && (!_showWo->isChecked()) && (!_showPo->isChecked()) )
  {
    QMessageBox::warning(this, tr("Incomplete Options"),
      tr("You must select at least one of the Show options."));
    return false;
  }

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
