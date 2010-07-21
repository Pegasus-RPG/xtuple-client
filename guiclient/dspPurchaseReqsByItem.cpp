/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPurchaseReqsByItem.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "dspRunningAvailability.h"
#include "mqlutil.h"
#include "purchaseOrder.h"

dspPurchaseReqsByItem::dspPurchaseReqsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_pr, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _pr->addColumn(tr("P/R #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_number");
  _pr->addColumn(tr("Sub #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_subnumber");
  _pr->addColumn(tr("Status"),       _statusColumn, Qt::AlignCenter, true,  "pr_status" );
  _pr->addColumn(tr("Parent Order"), -1,            Qt::AlignLeft,   true,  "parent"   );
  _pr->addColumn(tr("Create Date"),        -1,    Qt::AlignLeft,  true,  "pr_createdate"  );
  _pr->addColumn(tr("Due Date"),     _dateColumn,   Qt::AlignCenter, true,  "pr_duedate" );
  _pr->addColumn(tr("Qty."),         _qtyColumn,    Qt::AlignRight,  true,  "pr_qtyreq"  );
  _pr->addColumn(tr("Notes"),        -1,    Qt::AlignLeft,  true,  "pr_releasenote"  );
  connect(omfgThis, SIGNAL(purchaseRequestsUpdated()), this, SLOT(sFillList()));
}

dspPurchaseReqsByItem::~dspPurchaseReqsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPurchaseReqsByItem::languageChange()
{
  retranslateUi(this);
}

bool dspPurchaseReqsByItem::setParams(ParameterList &params)
{
  if (! _item->isValid())
  {
    QMessageBox::information(this, tr("Item Required"),
                             tr("<p>Item is required."));
    _item->setFocus();
    return false;
  }

  _warehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("manual", tr("Manual"));
  params.append("other",  tr("Other"));
  return true;
}

void dspPurchaseReqsByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("PurchaseRequestsByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPurchaseReqsByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Release P/R..."), this, SLOT(sRelease()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Delete P/R..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));
}

void dspPurchaseReqsByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _pr->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPurchaseReqsByItem::sRelease()
{
  ParameterList params;
  params.append("mode", "releasePr");
  params.append("pr_id", _pr->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    delete newdlg;
}

void dspPurchaseReqsByItem::sDelete()
{
  q.prepare("SELECT deletePr(:pr_id) AS _result;");
  q.bindValue(":pr_id", _pr->id());
  q.exec();

  sFillList();
}

void dspPurchaseReqsByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("purchase", "purchaserequests");
  q = mql.toQuery(params);
  _pr->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
