/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoSoStatus.h"

#include <QAction>
#include <QMenu>
#include <QVariant>
#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "closeWo.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "mqlutil.h"

dspWoSoStatus::dspWoSoStatus(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  _wo->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft   , true, "wonumber" );
  _wo->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter , true, "wo_status");
  _wo->addColumn(tr("Item Number"), -1,            Qt::AlignLeft   , true, "item_number");
  _wo->addColumn(tr("UOM"),         _uomColumn,    Qt::AlignLeft   , true, "uom_name");
  _wo->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter , true, "warehous_code");
  _wo->addColumn(tr("S/O #"),       _orderColumn,  Qt::AlignLeft   , true, "cohead_number");
  _wo->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight  , true, "wo_qtyord");
  _wo->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight  , true, "wo_qtyrcv");
  _wo->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignCenter , true, "wo_startdate");
  _wo->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignCenter , true, "wo_duedate");

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

dspWoSoStatus::~dspWoSoStatus()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspWoSoStatus::languageChange()
{
  retranslateUi(this);
}

void dspWoSoStatus::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);

  orReport report("OpenWorkOrdersWithParentSalesOrders", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoSoStatus::sCloseWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoSoStatus::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if ((pSelected->text(1) == "E") || (pSelected->text(1) == "I"))
    pMenu->addAction(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatlreq()));

  pMenu->addAction(tr("Close W/O..."), this, SLOT(sCloseWo()));
}

void dspWoSoStatus::sViewWomatlreq()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSoStatus::sFillList()
{
  ParameterList params;

  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("workOrderSoStatus", "detail");
  q = mql.toQuery(params);
  _wo->populate(q);
}

bool dspWoSoStatus::setParams(ParameterList & params)
{
  params.append("woSoStatus");
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());
  return true;
}

void dspWoSoStatus::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
