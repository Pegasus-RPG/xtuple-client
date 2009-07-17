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
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range.") );
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("component_item_id", _componentItem->id());

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
  if ((_componentItem->id() != -1) && (_dates->allValid()))
  {
    QString sql( "SELECT womatlvar_id, posted,"
                 "       ordered, received,"
                 "       projreq, projqtyper,"
                 "       actiss, actqtyper,"
                 "       (actqtyper - projqtyper) AS qtypervar,"
                 "       CASE WHEN (actqtyper=projqtyper) THEN 0"
                 "            WHEN (projqtyper=0) THEN actqtyper"
                 "            ELSE ((1 - (actqtyper / projqtyper)) * -1)"
                 "       END AS qtypervarpercent,"
                 "       womatlvar_notes, womatlvar_ref,"
                 "       'qty' AS ordered_xtnumericrole,"
                 "       'qty' AS received_xtnumericrole,"
                 "       'qty' AS projreq_xtnumericrole,"
                 "       'qtyper' AS projqtyper_xtnumericrole,"
                 "       'qty' AS actiss_xtnumericrole,"
                 "       'qtyper' AS actqtyper_xtnumericrole,"
                 "       'qtyper' AS qtypervar_xtnumericrole,"
                 "       'percent' AS qtypervarpercent_xtnumericrole "
                 "FROM ( SELECT womatlvar_id, womatlvar_posted AS posted,"
                 "              womatlvar_notes, womatlvar_ref,"
                 "              womatlvar_qtyord AS ordered, womatlvar_qtyrcv AS received,"
                 "              (womatlvar_qtyrcv * (womatlvar_qtyper * (1 + womatlvar_scrap))) AS projreq,"
                 "              womatlvar_qtyper AS projqtyper,"
                 "              (womatlvar_qtyiss) AS actiss, (womatlvar_qtyiss / (womatlvar_qtyrcv * (1 + womatlvar_scrap))) AS actqtyper "
                 "       FROM womatlvar, itemsite AS component, itemsite AS parent "
                 "       WHERE ((womatlvar_parent_itemsite_id=parent.itemsite_id)"
                 "        AND (womatlvar_component_itemsite_id=component.itemsite_id)"
                 "        AND (womatlvar_bomitem_id=:bomitem_id)"
                 "        AND (womatlvar_posted BETWEEN :startDate AND :endDate)" );

    if (_warehouse->isSelected())
      sql += " AND (component.itemsite_warehous_id=:warehous_id)";

    sql += ") ) AS data "
           "ORDER BY posted";

    q.prepare(sql);
    _warehouse->bindValue(q);
    _dates->bindValue(q);
    q.bindValue(":bomitem_id", _componentItem->id());
    q.exec();
    _womatlvar->populate(q);
  }
}

