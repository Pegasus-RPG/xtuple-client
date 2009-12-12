/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspMaterialUsageVarianceByBOMItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include <metasql.h>
#include "mqlutil.h"

/*
 *  Constructs a dspMaterialUsageVarianceByBOMItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspMaterialUsageVarianceByBOMItem::dspMaterialUsageVarianceByBOMItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_womatlvar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_componentItem, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateComponentItems(int)));

  _item->setType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _womatlvar->addColumn(tr("Post Date"),      _dateColumn,  Qt::AlignCenter, true,  "posted" );
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
dspMaterialUsageVarianceByBOMItem::~dspMaterialUsageVarianceByBOMItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspMaterialUsageVarianceByBOMItem::languageChange()
{
  retranslateUi(this);
}

void dspMaterialUsageVarianceByBOMItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  params.append("includeFormatted");
	
  orReport report("MaterialUsageVarianceByBOMItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspMaterialUsageVarianceByBOMItem::sPopulateMenu(QMenu *)
{
}

void dspMaterialUsageVarianceByBOMItem::sPopulateComponentItems(int pItemid)
{
  if (pItemid != -1)
  {
    q.prepare( "SELECT bomitem_id,"
               "       (bomitem_seqnumber || '-' || item_number || ' ' || item_descrip1 || ' ' || item_descrip2) "
               "FROM bomitem, item "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (bomitem_parent_item_id=:item_id) ) "
               "ORDER BY bomitem_seqnumber;" );
    q.bindValue(":item_id", pItemid);
    q.exec();
    _componentItem->populate(q);
  }
  else
    _componentItem->clear();
}

void dspMaterialUsageVarianceByBOMItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
	
  MetaSQLQuery mql = mqlLoad("workOrderVariance", "material");
  q = mql.toQuery(params);
  _womatlvar->populate(q);
}

bool dspMaterialUsageVarianceByBOMItem::setParams(ParameterList &params)
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

  if(!_componentItem->isValid())
  {
    QMessageBox::warning(this, tr("Invalid BOM Item"),
      tr("You must specify a BOM Item.") );
    return false;
  }

  params.append("item_id", _item->id());
  params.append("bomitem_id", _componentItem->id());
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  return true;
}


