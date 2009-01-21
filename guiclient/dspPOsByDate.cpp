/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
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
  QString sql( "SELECT pohead_id, pohead_number,"
               "       warehous_code AS warehousecode,"
               "       CASE WHEN(poitem_status='C') THEN <? value(\"closed\") ?>"
               "            WHEN(poitem_status='U') THEN <? value(\"unposted\") ?>"
               "            WHEN(poitem_status='O'"
               "                 AND (SUM(poitem_qty_received-poitem_qty_returned) > 0)"
               "                 AND (SUM(poitem_qty_ordered)>SUM(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"partial\") ?>"
               "            WHEN(poitem_status='O'"
               "                 AND (SUM(poitem_qty_received-poitem_qty_returned) > 0)"
               "                 AND (SUM(poitem_qty_ordered)<=SUM(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"received\") ?>"
               "            WHEN(poitem_status='O') THEN <? value(\"open\") ?>"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       vend_name,"
               "       MIN(poitem_duedate) AS minDueDate,"
               "       CASE WHEN (MIN(poitem_duedate) < CURRENT_DATE) THEN 'error'"
               "       END AS minDueDate_qtforegroundrole "
               "  FROM vend, poitem, pohead "
               "       LEFT OUTER JOIN warehous ON (pohead_warehous_id=warehous_id)"
               " WHERE ((poitem_pohead_id=pohead_id)"
               "   AND  (pohead_vend_id=vend_id)"
               "   AND  (poitem_duedate BETWEEN <? value(\"startDate\") ?>"
               "                            AND <? value(\"endDate\") ?>)"
               "<? if exists(\"warehous_id\") ?>"
               "   AND (pohead_warehous_id=<? value(\"warehous_id\") ?>) "
               "<? endif ?>"
               "<? if not exists(\"showClosed\") ?>"
               "   AND (poitem_status!='C')"
               "<? endif ?>"
               "<? if exists(\"agentUsername\") ?>"
               "   AND (pohead_agent_username=<? value(\"agentUsername\") ?>)"
               "<? endif ?>"
               ") "
               "GROUP BY pohead_id, pohead_number, warehous_code,"
               "         poitem_status, vend_name "
               "ORDER BY minDueDate;");

  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _poitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
