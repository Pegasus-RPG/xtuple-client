/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspLaborVarianceByWorkCenter.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "mqlutil.h"

dspLaborVarianceByWorkCenter::dspLaborVarianceByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_woopervar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _wrkcnt->populate( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip) "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );

  _woopervar->addColumn(tr("Post Date"),_dateColumn, Qt::AlignCenter,true, "woopervar_posted");
  _woopervar->addColumn(tr("Parent Item"),       -1, Qt::AlignLeft,  true, "item_number");
  _woopervar->addColumn(tr("Seq #"),     _seqColumn, Qt::AlignCenter,true, "woopervar_seqnumber");
  _woopervar->addColumn(tr("Proj Setup"),_qtyColumn, Qt::AlignRight, true, "woopervar_stdsutime");
  _woopervar->addColumn(tr("Proj. Run"), _qtyColumn, Qt::AlignRight, true, "woopervar_stdrntime");
  _woopervar->addColumn(tr("Act. Setup"),_qtyColumn, Qt::AlignRight, true, "woopervar_sutime");
  _woopervar->addColumn(tr("Act. Run"),  _qtyColumn, Qt::AlignRight, true, "woopervar_rntime");
  _woopervar->addColumn(tr("Setup Var."),_qtyColumn, Qt::AlignRight, true, "suvar");
  _woopervar->addColumn(tr("Run Var."),  _qtyColumn, Qt::AlignRight, true, "rnvar");
}

dspLaborVarianceByWorkCenter::~dspLaborVarianceByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspLaborVarianceByWorkCenter::languageChange()
{
  retranslateUi(this);
}

bool dspLaborVarianceByWorkCenter::setParams(ParameterList &params)
{
  if (! _dates->allValid())
  {
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);
  params.append("wrkcnt_id", _wrkcnt->id());
  return true;
}

void dspLaborVarianceByWorkCenter::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("LaborVarianceByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspLaborVarianceByWorkCenter::sPopulateMenu(QMenu *)
{
}

void dspLaborVarianceByWorkCenter::sFillList()
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
