/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUnusedPurchasedItems.h"

#include <qvariant.h>
//#include <qstatusbar.h>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspUnusedPurchasedItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUnusedPurchasedItems::dspUnusedPurchasedItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUnusedPurchasedItems::~dspUnusedPurchasedItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUnusedPurchasedItems::languageChange()
{
    retranslateUi(this);
}


void dspUnusedPurchasedItems::init()
{
//  statusBar()->hide();
  
  _classCode->setType(ParameterGroup::ClassCode);

  _item->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"  );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"  );
  _item->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "uom_name"  );
  _item->addColumn(tr("Total QOH"),   _qtyColumn,  Qt::AlignRight,  true,  "qoh" );
  _item->addColumn(tr("Last Cnt'd"),  _dateColumn, Qt::AlignRight,  true,  "lastcount" );
  _item->addColumn(tr("Last Used"),   _dateColumn, Qt::AlignRight,  true,  "lastused" );
}

void dspUnusedPurchasedItems::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);

  if(_includeUncontrolled->isChecked())
    params.append("includeUncontrolledItems");

  orReport report("UnusedPurchasedItems", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUnusedPurchasedItems::sFillList()
{
  QString sql( "SELECT DISTINCT item_id, item_number,"
               "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
               "                SUM(itemsite_qtyonhand) AS qoh,"
               "                MAX(itemsite_datelastcount) AS lastcount,"
               "                MAX(itemsite_datelastused) AS lastused,"
               "                'qty' AS qoh_xtnumericrole,"
               "                CASE WHEN (COALESCE(MAX(itemsite_datelastcount), startOfTime()) = startOfTime()) THEN 'Never' END AS lastcount_qtdisplayrole,"
               "                CASE WHEN (COALESCE(MAX(itemsite_datelastused), startOfTime()) = startOfTime()) THEN 'Never' END AS lastused_qtdisplayrole "
               "FROM item, itemsite, uom "
               "WHERE ((itemsite_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (item_id NOT IN (SELECT DISTINCT bomitem_item_id FROM bomitem))"
               " AND (NOT item_sold)"
               " AND (item_type IN ('P', 'O'))" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (!_includeUncontrolled->isChecked())
    sql += " AND (itemsite_controlmethod <> 'N')";

  sql += ") "
         "GROUP BY item_id, item_number, uom_name, item_descrip1, item_descrip2 "
         "ORDER BY item_number;";

  q.prepare(sql);
  _classCode->bindValue(q);
  q.exec();
  _item->populate(q);
}
