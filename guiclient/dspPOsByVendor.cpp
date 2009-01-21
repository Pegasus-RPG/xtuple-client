/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPOsByVendor.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>

#include "mqlutil.h"
#include "purchaseOrder.h"

dspPOsByVendor::dspPOsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight,  true,  "pohead_number"  );
  _poitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehousecode" );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter, true,  "poitemstatus" );
  _poitem->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft,   true,  "vend_number"   );
  _poitem->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter, true,  "sortDate" );

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);

  _descrip->setEnabled(_searchDescrip->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPOsByVendor::~dspPOsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPOsByVendor::languageChange()
{
  retranslateUi(this);
}

bool dspPOsByVendor::setParams(ParameterList &params)
{
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  if(_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());

  if(_showClosed->isChecked())
    params.append("showClosed");

  if(_byReceiptDate->isChecked())
    params.append("byReceiptDate");
  else if(_byDueDate->isChecked())
    params.append("byDueDate");
  else //if(_byOrderDate->isChecked())
    params.append("byOrderDate");

  if(_searchDescrip->isChecked())
    params.append("descrip_pattern", _descrip->text());

  params.append("closed",	tr("Closed"));
  params.append("unposted",	tr("Unposted"));
  params.append("partial",	tr("Partial"));
  params.append("received",	tr("Received"));
  params.append("open",		tr("Open"));

  return true;
}

void dspPOsByVendor::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("POsByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPOsByVendor::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (pSelected->text(2) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privileges->check("MaintainPurchaseOrders")) && (!_privileges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPOsByVendor::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByVendor::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByVendor::sFillList()
{
  MetaSQLQuery mql = mqlLoad("purchaseOrders", "detail");

  ParameterList params;
  setParams(params);

  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _poitem->populate(q, true);
}
