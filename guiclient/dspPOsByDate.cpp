/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPOsByDate.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include <metasql.h>

#include "mqlutil.h"
#include "purchaseOrder.h"

dspPOsByDate::dspPOsByDate(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Due Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Due Date:"));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight, true, "pohead_number");
  _poitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter,true, "warehousecode");
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter,true, "poitemstatus");
  _poitem->addColumn(tr("Vendor"),      _itemColumn,  Qt::AlignLeft,  true, "vend_name");
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "minDueDate");
}

dspPOsByDate::~dspPOsByDate()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPOsByDate::languageChange()
{
  retranslateUi(this);
}

bool dspPOsByDate::setParams(ParameterList &pParams)
{
  pParams.append("byDate");
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr( "Please enter a valid Start Date." ) );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr( "Please eneter a valid End Date." ) );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(pParams);
  _warehouse->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showClosed->isChecked())
    pParams.append("showClosed");

  pParams.append("closed",      tr("Closed"));
  pParams.append("unposted",    tr("Unposted"));
  pParams.append("partial",     tr("Partial"));
  pParams.append("received",    tr("Received"));
  pParams.append("open",        tr("Open"));

  return true;
}

void dspPOsByDate::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("POsByDate", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPOsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
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

void dspPOsByDate::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("purchaseOrders", "detail");
  q = mql.toQuery(params);
  _poitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
