/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBreederDistributionVarianceByWarehouse.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"

dspBreederDistributionVarianceByWarehouse::dspBreederDistributionVarianceByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_brdvar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _brdvar->addColumn(tr("Post Date"),   _dateColumn, Qt::AlignCenter,true, "brdvar_postdate");
  _brdvar->addColumn(tr("Parent Item"), _itemColumn, Qt::AlignLeft,  true, "pitem_number");
  _brdvar->addColumn(tr("Component Item"),       -1, Qt::AlignLeft,  true, "citem_number");
  _brdvar->addColumn(tr("Std. Qty. per"),_qtyColumn, Qt::AlignRight, true, "brdvar_stdqtyper");
  _brdvar->addColumn(tr("Std. Qty."),    _qtyColumn, Qt::AlignRight, true, "stdqty");
  _brdvar->addColumn(tr("Act. Qty. per"),_qtyColumn, Qt::AlignRight, true, "brdvar_actqtyper");
  _brdvar->addColumn(tr("Act. Qty."),    _qtyColumn, Qt::AlignRight, true, "actqty");
  _brdvar->addColumn(tr("Qty per Var."), _qtyColumn, Qt::AlignRight, true, "qtypervariance");
  _brdvar->addColumn(tr("% Var."),     _prcntColumn, Qt::AlignRight, true, "percentvariance");
}

dspBreederDistributionVarianceByWarehouse::~dspBreederDistributionVarianceByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBreederDistributionVarianceByWarehouse::languageChange()
{
  retranslateUi(this);
}

bool dspBreederDistributionVarianceByWarehouse::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  return true;
}

void dspBreederDistributionVarianceByWarehouse::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BreederDistributionVarianceByWarehouse", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBreederDistributionVarianceByWarehouse::sPopulateMenu(QMenu *)
{
}

void dspBreederDistributionVarianceByWarehouse::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("breederDistributionVariance", "detail");
  q = mql.toQuery(params);
  _brdvar->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
