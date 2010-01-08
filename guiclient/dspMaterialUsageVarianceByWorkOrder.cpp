/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspMaterialUsageVarianceByWorkOrder.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include <metasql.h>
#include "mqlutil.h"

#include "inputManager.h"

/*
 *  Constructs a dspMaterialUsageVarianceByWorkOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspMaterialUsageVarianceByWorkOrder::dspMaterialUsageVarianceByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_womatlvar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_wo, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _wo->setType(cWoClosed);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatlvar->addColumn(tr("Post Date"),          _dateColumn,  Qt::AlignCenter, true,  "posted" );
  _womatlvar->addColumn(tr("Component Item"),     -1,           Qt::AlignLeft,   true,  "componentitemnumber"   );
  _womatlvar->addColumn(tr("Description"),        -1,           Qt::AlignLeft,   true,  "componentdescrip");
  _womatlvar->addColumn(tr("Ordered"),            _qtyColumn,   Qt::AlignRight,  true,  "ordered"  );
  _womatlvar->addColumn(tr("Produced"),           _qtyColumn,   Qt::AlignRight,  true,  "received"  );
  _womatlvar->addColumn(tr("Proj. Req."),         _qtyColumn,   Qt::AlignRight,  true,  "projreq"  );
  _womatlvar->addColumn(tr("Proj. Qty. per"),     _qtyColumn,   Qt::AlignRight,  true,  "projqtyper"  );
  _womatlvar->addColumn(tr("Act. Iss."),          _qtyColumn,   Qt::AlignRight,  true,  "actiss"  );
  _womatlvar->addColumn(tr("Act. Qty. per"),      _qtyColumn,   Qt::AlignRight,  true,  "actqtyper"  );
  _womatlvar->addColumn(tr("Qty. per Var."),      _qtyColumn,   Qt::AlignRight,  true,  "qtypervar"  );
  _womatlvar->addColumn(tr("%"),                  _prcntColumn, Qt::AlignRight,  true,  "qtypervarpercent");
  _womatlvar->addColumn(tr("Notes"),              -1,           Qt::AlignLeft,   false, "womatlvar_notes");
  _womatlvar->addColumn(tr("Reference"), -1,           Qt::AlignLeft,   false, "womatlvar_ref");
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspMaterialUsageVarianceByWorkOrder::~dspMaterialUsageVarianceByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspMaterialUsageVarianceByWorkOrder::languageChange()
{
  retranslateUi(this);
}

void dspMaterialUsageVarianceByWorkOrder::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  params.append("includeFormatted");
	
  orReport report("MaterialUsageVarianceByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspMaterialUsageVarianceByWorkOrder::sPopulateMenu(QMenu *)
{
}

void dspMaterialUsageVarianceByWorkOrder::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
	
  MetaSQLQuery mql = mqlLoad("workOrderVariance", "material");
  q = mql.toQuery(params);
  _womatlvar->populate(q);
}

bool dspMaterialUsageVarianceByWorkOrder::setParams(ParameterList &params)
{
  if(!_wo->isValid())
  {
    QMessageBox::warning(this, tr("Invalid W/O"),
      tr("You must specify a Work Order.") );
    return false;
  }

  params.append("wo_id", _wo->id());

  return true;
}


