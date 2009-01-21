/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspLaborVarianceByWorkOrder.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "inputManager.h"
#include "mqlutil.h"

dspLaborVarianceByWorkOrder::dspLaborVarianceByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_woopervar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));

  _wo->setType(cWoClosed);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _woopervar->addColumn(tr("Post Date"),_dateColumn, Qt::AlignCenter,true, "woopervar_posted");
  _woopervar->addColumn(tr("Seq #"),     _seqColumn, Qt::AlignCenter,true, "woopervar_seqnumber");
  _woopervar->addColumn(tr("Work Center"),       -1, Qt::AlignLeft,  true, "wrkcnt_code");
  _woopervar->addColumn(tr("Proj Setup"),_qtyColumn, Qt::AlignRight, true, "woopervar_stdsutime");
  _woopervar->addColumn(tr("Proj. Run"), _qtyColumn, Qt::AlignRight, true, "woopervar_stdrntime");
  _woopervar->addColumn(tr("Act. Setup"),_qtyColumn, Qt::AlignRight, true, "woopervar_sutime");
  _woopervar->addColumn(tr("Act. Run"),  _qtyColumn, Qt::AlignRight, true, "woopervar_rntime");
  _woopervar->addColumn(tr("Setup Var."),_qtyColumn, Qt::AlignRight, true, "suvar");
  _woopervar->addColumn(tr("Run Var."),  _qtyColumn, Qt::AlignRight, true, "rnvar");
}

dspLaborVarianceByWorkOrder::~dspLaborVarianceByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspLaborVarianceByWorkOrder::languageChange()
{
  retranslateUi(this);
}

bool dspLaborVarianceByWorkOrder::setParams(ParameterList &params)
{
  if (! _wo->isValid())
  {
    _wo->setFocus();
    return false;
  }
  params.append("wo_id", _wo->id());
  return true;
}

void dspLaborVarianceByWorkOrder::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("LaborVarianceByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspLaborVarianceByWorkOrder::sPopulateMenu(QMenu *)
{
}

void dspLaborVarianceByWorkOrder::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("manufacture", "laborvariance");
  q = mql.toQuery(params);
  _woopervar->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
