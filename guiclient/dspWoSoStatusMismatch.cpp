/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoSoStatusMismatch.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include <metasql.h>
#include "mqlutil.h"
#include "closeWo.h"
#include "dspWoMaterialsByWorkOrder.h"

/*
 *  Constructs a dspWoSoStatusMismatch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoSoStatusMismatch::dspWoSoStatusMismatch(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

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
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoSoStatusMismatch::~dspWoSoStatusMismatch()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoSoStatusMismatch::languageChange()
{
  retranslateUi(this);
}

void dspWoSoStatusMismatch::sPrint()
{
  ParameterList params;

  _warehouse->appendValue(params);

  orReport report("OpenWorkOrdersWithClosedParentSalesOrders", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoSoStatusMismatch::sCloseWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoSoStatusMismatch::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if ((pSelected->text(1) == "E") || (pSelected->text(1) == "I"))
    pMenu->insertItem(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatlreq()), 0);

  pMenu->insertItem(tr("Close W/O..."), this, SLOT(sCloseWo()), 0);
}

void dspWoSoStatusMismatch::sViewWomatlreq()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoSoStatusMismatch::sFillList()
{
  ParameterList params;

  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("workOrderSoStatus", "detail");
  q = mql.toQuery(params);
  _wo->populate(q);
}

bool dspWoSoStatusMismatch::setParams(ParameterList & params)
{
  params.append("woSoStatusMismatch");
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());
  return true;
}
