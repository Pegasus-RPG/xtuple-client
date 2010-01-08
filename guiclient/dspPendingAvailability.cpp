/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPendingAvailability.h"

#include <QVariant>
#include <QValidator>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspPendingAvailability as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPendingAvailability::dspPendingAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(privateIdChanged(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cJob | ItemLineEdit::cTooling);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cJob);

  _effective->setNullString(tr("Now"));
  _effective->setNullDate(QDate::currentDate());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();

  _buildDate->setNullString(tr("Latest"));
  _buildDate->setNullDate(omfgThis->endOfTime());
  _buildDate->setAllowNullDate(TRUE);
  _buildDate->setNull();

  _qtyToBuild->setValidator(omfgThis->qtyVal());
  _qtyToBuild->setText("1.0");

  _items->addColumn(tr("#"),            _seqColumn,  Qt::AlignCenter, true,  "bomitem_seqnumber" );
  _items->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _items->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "item_descrip"   );
  _items->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _items->addColumn(tr("Pend. Alloc."), _qtyColumn,  Qt::AlignRight,  true,  "pendalloc"  );
  _items->addColumn(tr("Total Alloc."), _qtyColumn,  Qt::AlignRight,  true,  "totalalloc"  );
  _items->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "qoh"  );
  _items->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight,  true,  "totalavail"  );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPendingAvailability::~dspPendingAvailability()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPendingAvailability::languageChange()
{
  retranslateUi(this);
}

void dspPendingAvailability::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("warehous_id", _warehouse->id());
  params.append("buildQty", _qtyToBuild->toDouble());
  params.append("effectiveDate", _effective->date());
  params.append("buildDate", _buildDate->date());

  if(_showShortages->isChecked())
    params.append("showShortages");

  orReport report("PendingWOMaterialAvailability", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPendingAvailability::sFillList()
{
  if (!checkParameters())
    return;

  _items->clear();

  QString sql( "SELECT itemsite_id, bomitem_seqnumber, item_number, item_descrip, uom_name,"
               "       pendalloc, (totalalloc + pendalloc) AS totalalloc,"
               "       qoh, (qoh + ordered - (totalalloc + pendalloc)) AS totalavail,"
               "       reorderlevel,"
               "       'qty' AS pendalloc_xtnumericrole,"
               "       'qty' AS totalalloc_xtnumericrole,"
               "       'qty' AS qoh_xtnumericrole,"
               "       'qty' AS totalavail_xtnumericrole,"
               "       CASE WHEN (qoh < pendalloc) THEN 'error' END AS qoh_qtforegroundrole,"
               "       CASE WHEN ((qoh + ordered - (totalalloc + pendalloc)) < reorderlevel) THEN 'error'"
               "            WHEN ((qoh + ordered - (totalalloc + pendalloc)) = reorderlevel) THEN 'warning'"
               "       END AS totalavail_qtforegroundrole "
               "FROM ( SELECT itemsite_id, bomitem_seqnumber, item_number,"
               "              (item_descrip1 || ' ' || item_descrip2) AS item_descrip, uom_name,"
               "              ((itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL,"
			   "                            (bomitem_qtyfxd + bomitem_qtyper * :buildQty) * (1 + bomitem_scrap)))) AS pendalloc,"
               "              qtyAllocated(itemsite_id, DATE(:buildDate)) AS totalalloc,"
               "              noNeg(itemsite_qtyonhand) AS qoh,"
               "              qtyOrdered(itemsite_id, DATE(:buildDate)) AS ordered,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel"
			   "       FROM itemsite, item, bomitem(:item_id), uom "
               "       WHERE ( (bomitem_item_id=itemsite_item_id)"
               "        AND (itemsite_item_id=item_id)"
               "        AND (item_inv_uom_id=uom_id)"
               "        AND (itemsite_warehous_id=:warehous_id)" );

  if (_effective->isNull())
    sql += " AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))) ) AS data ";
  else
    sql += " AND (:effective BETWEEN bomitem_effective AND (bomitem_expires-1))) ) AS data ";

  if (_showShortages->isChecked())
    sql += "WHERE ((qoh + ordered - (totalalloc + pendalloc)) < 0) ";
    
  sql += "ORDER BY bomitem_seqnumber";

  q.prepare(sql);
  q.bindValue(":buildQty", _qtyToBuild->toDouble());
  q.bindValue(":buildDate", _buildDate->date());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":effective", _effective->date());
  q.exec();
  _items->populate(q);
}

bool dspPendingAvailability::checkParameters()
{
  return TRUE;
}
