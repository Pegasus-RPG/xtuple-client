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

#include <QVariant>
#include <QWorkspace>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"
#include "salesOrder.h"
#include "invoice.h"
#include "purchaseOrderItem.h"

/*
 *  Constructs a dspOrderActivityByProject as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspOrderActivityByProject::dspOrderActivityByProject(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_project, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_project, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_showPo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showSo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showWo, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_orders, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _orders->addColumn(tr("Type"),        _orderColumn, Qt::AlignLeft,   true,  "type"   );
  _orders->addColumn(tr("Order #"),     _itemColumn,  Qt::AlignLeft,   true,  "ordernumber"   );
  _orders->addColumn(tr("Status"),      _orderColumn, Qt::AlignCenter, true,  "status" );
  _orders->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _showPo->setChecked(true);
    _showSo->setChecked(true);
    _showWo->setChecked(true);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspOrderActivityByProject::~dspOrderActivityByProject()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspOrderActivityByProject::languageChange()
{
  retranslateUi(this);
}

void dspOrderActivityByProject::sPopulateMenu( QMenu * pMenu )
{
  int menuItem;

  if(_orders->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainSalesOrders") && !_privileges->check("ViewSalesOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if(_orders->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Edit Quote..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainQuotes"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Quote..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainQuotes") && !_privileges->check("ViewQuotes"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if(_orders->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("Edit Invoice..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Invoice..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainMiscInvoices") && !_privileges->check("ViewMiscInvoices"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if(_orders->altId() == 5)
  {
    menuItem = pMenu->insertItem(tr("Edit P/O Item..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View P/O Item..."), this, SLOT(sView()), 0);
    if (!_privileges->check("MaintainPurchaseOrders") && !_privileges->check("ViewPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
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

  QString sql;

  if(_showSo->isChecked())
  {
    sql += "SELECT cohead_id AS id, 1 AS typeid,"
           "       text(:so) AS type, text(cohead_number) AS ordernumber,"
           "       text('') AS status,"
           "       0 AS qty,"
           "       'qty' AS qty_xtnumericrole "
           "  FROM cohead "
           " WHERE (cohead_prj_id=:prj_id) ";

    sql += " UNION "
           "SELECT quhead_id AS id, 2 AS typeid,"
           "       text(:quote) AS type, text(quhead_number) AS ordernumber,"
           "       text('') AS status,"
           "       0 AS qty,"
           "       'qty' AS qty_xtnumericrole "
           "  FROM quhead "
           " WHERE (quhead_prj_id=:prj_id) ";

    sql += " UNION "
           "SELECT invchead_id AS id, 3 AS typeid,"
           "       text(:invoice) AS type, text(invchead_invcnumber) AS ordernumber,"
           "       text('') AS status,"
           "       0 AS qty,"
           "       'qty' AS qty_xtnumericrole "
           "  FROM invchead "
           " WHERE (invchead_prj_id=:prj_id) ";
  }

  if(_showWo->isChecked())
  {
    if(_showSo->isChecked())
      sql += " UNION ";

    sql += "SELECT wo_id AS id, 4 AS typeid,"
           "       text(:wo) AS type, formatWoNumber(wo_id) AS ordernumber,"
           "       wo_status AS status,"
           "       wo_qtyord AS qty,"
           "       'qty' AS qty_xtnumericrole "
           "  FROM wo "
           " WHERE (wo_prj_id=:prj_id) ";
  }

  if(_showPo->isChecked())
  {
    if(_showSo->isChecked() || _showWo->isChecked())
      sql += " UNION ";

    sql += "SELECT poitem_id AS id, 5 AS typeid,"
           "       text(:po) AS type, (text(pohead_number) || '-' || text(poitem_linenumber)) AS ordernumber,"
           "       poitem_status AS status,"
           "       poitem_qty_ordered AS qty,"
           "       'qty' AS qty_xtnumericrole "
           "  FROM pohead, poitem "
           " WHERE ((poitem_pohead_id=pohead_id) "
           "   AND  (poitem_prj_id=:prj_id)) ";

    sql += " UNION "
           "SELECT pr_id AS id, 6 AS typeid,"
           "       text(:pr) AS type, text(pr_number) AS ordernumber,"
           "       pr_status AS status,"
           "       pr_qtyreq AS qty,"
           "       'qty' AS qty_xtnumericrole "
           "  FROM pr "
           " WHERE (pr_prj_id=:prj_id) ";
  }

  sql += " ORDER BY ordernumber; ";

  q.prepare(sql);
  q.bindValue(":prj_id", _project->id());
  q.bindValue(":so", tr("S/O"));
  q.bindValue(":wo", tr("W/O"));
  q.bindValue(":po", tr("P/O"));
  q.bindValue(":pr", tr("P/R"));
  q.bindValue(":quote", tr("Quote"));
  q.bindValue(":invoice", tr("Invoice"));
  q.exec();

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

