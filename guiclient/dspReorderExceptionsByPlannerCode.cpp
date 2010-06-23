/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspReorderExceptionsByPlannerCode.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "dspRunningAvailability.h"
#include "workOrder.h"
#include "mqlutil.h"

dspReorderExceptionsByPlannerCode::dspReorderExceptionsByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_exception, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _exception->addColumn(tr("Site"),           _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _exception->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _exception->addColumn(tr("Description"),    -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _exception->addColumn(tr("Exception Date"), _dateColumn, Qt::AlignCenter, true,  "reorderdate" );
  _exception->addColumn(tr("Reorder Level"),  _qtyColumn,  Qt::AlignRight,  true,  "reorderlevel"  );
  _exception->addColumn(tr("Proj. Avail."),   _qtyColumn,  Qt::AlignRight,  true,  "projavail"  );
}

dspReorderExceptionsByPlannerCode::~dspReorderExceptionsByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspReorderExceptionsByPlannerCode::languageChange()
{
  retranslateUi(this);
}

bool dspReorderExceptionsByPlannerCode::setParams(ParameterList &params)
{
  params.append("lookAheadDays", _days->value());

  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_includePlanned->isChecked())
    params.append("includePlannedOrders");

  return true;
}

void dspReorderExceptionsByPlannerCode::sFillList()
{
  ParameterList params;
  setParams(params);

  MetaSQLQuery mql = mqlLoad("reorderExceptionsByPlannerCode", "detail");

  q = mql.toQuery(params);
  _exception->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspReorderExceptionsByPlannerCode::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ReorderExceptionsByPlannerCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspReorderExceptionsByPlannerCode::sPopulateMenu( QMenu *pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Running Availability..."), this, SLOT(sRunningAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (_exception->altId() == 1)
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Create Work Order..."), this, SLOT(sCreateWorkOrder()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspReorderExceptionsByPlannerCode::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _exception->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspReorderExceptionsByPlannerCode::sCreateWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _exception->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
