/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspMaterialUsageVarianceByItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

/*
 *  Constructs a dspMaterialUsageVarianceByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspMaterialUsageVarianceByItem::dspMaterialUsageVarianceByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_womatlvar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemSites(int)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cJob);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cJob);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), 0);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), 0);

  _womatlvar->addColumn(tr("Post Date"),      _dateColumn,  Qt::AlignCenter, true,  "posted" );
  _womatlvar->addColumn(tr("Component Item"), _itemColumn,  Qt::AlignLeft,   true,  "componentitemnumber"   );
  _womatlvar->addColumn(tr("Description"),        -1,       Qt::AlignLeft,   true,  "componentdescrip");
  _womatlvar->addColumn(tr("Ordered"),        _qtyColumn,   Qt::AlignRight,  true,  "ordered"  );
  _womatlvar->addColumn(tr("Produced"),       _qtyColumn,   Qt::AlignRight,  true,  "received"  );
  _womatlvar->addColumn(tr("Proj. Req."),     _qtyColumn,   Qt::AlignRight,  true,  "projreq"  );
  _womatlvar->addColumn(tr("Proj. Qty. per"), _qtyColumn,   Qt::AlignRight,  true,  "projqtyper"  );
  _womatlvar->addColumn(tr("Act. Iss."),      _qtyColumn,   Qt::AlignRight,  true,  "actiss"  );
  _womatlvar->addColumn(tr("Act. Qty. per"),  _qtyColumn,   Qt::AlignRight,  true,  "actqtyper"  );
  _womatlvar->addColumn(tr("Qty. per Var."),  _qtyColumn,   Qt::AlignRight,  true,  "qtypervar"  );
  _womatlvar->addColumn(tr("%"),              _prcntColumn, Qt::AlignRight,  true,  "qtypervarpercent"  );
  _womatlvar->addColumn(tr("Notes"),              -1,       Qt::AlignLeft,   false, "womatlvar_notes");
  _womatlvar->addColumn(tr("Reference"), -1,       Qt::AlignLeft,   false, "womatlvar_ref");
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspMaterialUsageVarianceByItem::~dspMaterialUsageVarianceByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspMaterialUsageVarianceByItem::languageChange()
{
  retranslateUi(this);
}

void dspMaterialUsageVarianceByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  params.append("includeFormatted");
	
  orReport report("MaterialUsageVarianceByItem", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspMaterialUsageVarianceByItem::sPopulateMenu(QMenu *)
{
}

void dspMaterialUsageVarianceByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
	
  MetaSQLQuery mql = mqlLoad("workOrderVariance", "material");
  q = mql.toQuery(params);
  _womatlvar->populate(q);
}

bool dspMaterialUsageVarianceByItem::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range.") );
    return false;
  }

  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Item"),
      tr("You must specify an Item.") );
    return false;
  }

  params.append("item_id", _item->id());
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  return true;
}


