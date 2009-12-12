/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoMaterialsByItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <openreports.h>
#include <parameter.h>
#include "inputManager.h"

/*
 *  Constructs a dspWoMaterialsByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoMaterialsByItem::dspWoMaterialsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _womatl->addColumn(tr("W/O #"),         _orderColumn, Qt::AlignLeft,   true,  "wonumber"   );
  _womatl->addColumn(tr("Parent Item #"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _womatl->addColumn(tr("Oper. #"),       _dateColumn,  Qt::AlignCenter, true,  "wooperseq" );
  _womatl->addColumn(tr("Iss. Meth."),    _dateColumn,  Qt::AlignCenter, true,  "issuemethod" );
  _womatl->addColumn(tr("Iss. UOM"),      _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  _womatl->addColumn(tr("Fxd. Qty."),     _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyfxd"  );
  _womatl->addColumn(tr("Qty. Per"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyper"  );
  _womatl->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight,  true,  "womatl_scrap"  );
  _womatl->addColumn(tr("Required"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyreq"  );
  _womatl->addColumn(tr("Issued"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyiss"  );
  _womatl->addColumn(tr("Scrapped"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtywipscrap"  );
  _womatl->addColumn(tr("Balance"),       _qtyColumn,   Qt::AlignRight,  true,  "balance"  );
  _womatl->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter, true,  "womatl_duedate" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoMaterialsByItem::~dspWoMaterialsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoMaterialsByItem::languageChange()
{
  retranslateUi(this);
}

void dspWoMaterialsByItem::sPrint()
{
  ParameterList params;

  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  orReport report("WOMaterialRequirementsByComponentItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoMaterialsByItem::sFillList()
{
  if (!checkParameters())
    return;

  _womatl->clear();

  QString sql( "SELECT womatl.*,"
               "       formatWONumber(wo_id) AS wonumber,"
               "       item_number,"
               "       formatwooperseq(womatl_wooper_id) AS wooperseq, "
               "       CASE WHEN (womatl_issuemethod = 'S') THEN :push"
               "            WHEN (womatl_issuemethod = 'L') THEN :pull"
               "            WHEN (womatl_issuemethod = 'M') THEN :mixed"
               "            ELSE :error"
               "       END AS issuemethod,"
               "       uom_name,"
               "       noNeg(womatl_qtyreq - womatl_qtyiss) AS balance,"
               "       CASE WHEN (womatl_duedate <= CURRENT_DATE) THEN 'error' END AS womatl_duedate_qtforegroundrole,"
               "       'qty' AS womatl_qtyfxd_xtnumericrole,"
               "       'qtyper' AS womatl_qtyper_xtnumericrole,"
               "       'percent' AS womatl_scrap_xtnumericrole,"
               "       'qty' AS womatl_qtyreq_xtnumericrole,"
               "       'qty' AS womatl_qtyiss_xtnumericrole,"
               "       'qty' AS womatl_qtywipscrap_xtnumericrole,"
               "       'qty' AS balance_xtnumericrole,"
               "       0 AS womatl_qtyreq_xttotalrole,"
               "       0 AS womatl_qtyiss_xttotalrole,"
               "       0 AS womatl_qtywipscrap_xttotalrole,"
               "       0 AS balance_xttotalrole "
               "FROM wo, womatl, itemsite AS parentsite, itemsite AS componentsite, item, uom "
               "WHERE ((womatl_wo_id=wo_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (wo_status <> 'C')"
               " AND (wo_itemsite_id=parentsite.itemsite_id)"
               " AND (womatl_itemsite_id=componentsite.itemsite_id)"
               " AND (parentsite.itemsite_item_id=item_id)"
               " AND (componentsite.itemsite_item_id=:item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (componentsite.itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY wo_startdate, item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":push", tr("Push"));
  q.bindValue(":pull", tr("Pull"));
  q.bindValue(":mixed", tr("Mixed"));
  q.bindValue(":error", tr("Error"));
  q.bindValue(":item_id", _item->id());
  q.exec();
  _womatl->populate(q);
}

bool dspWoMaterialsByItem::checkParameters()
{
  return TRUE;
}

