/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoHistory.h"

#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "copyPurchaseOrder.h"

dspPoHistory::dspPoHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_copy,	SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_po, SIGNAL(newId(int)), this, SLOT(sFillList()));

  _po->setType((cPOOpen | cPOClosed));

  _poitem->addColumn(tr("#"),            _whsColumn,  Qt::AlignCenter, true, "poitem_linenumber");
  _poitem->addColumn(tr("Item/Doc. #"),  _itemColumn, Qt::AlignLeft,   true, "itemnumber");
  _poitem->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true, "itemdescription");
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true, "uomname");
  _poitem->addColumn(tr("Due/Recvd."),   _dateColumn, Qt::AlignCenter, true, "poitem_duedate");
  _poitem->addColumn(tr("Vend. Item #"), -1,          Qt::AlignLeft,   true, "poitem_vend_item_number");
  _poitem->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true, "poitem_vend_uom");
  _poitem->addColumn(tr("Ordered"),      _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_ordered");
  _poitem->addColumn(tr("Received"),     _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_received");
  _poitem->addColumn(tr("Returned"),     _qtyColumn,  Qt::AlignRight,  true, "poitem_qty_returned");
}

dspPoHistory::~dspPoHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoHistory::languageChange()
{
  retranslateUi(this);
}

void dspPoHistory::sFillList()
{
  if (_po->isValid())
  {
    q.prepare( "SELECT poitem_id, poitem.*,"
               "       COALESCE(item_number, :nonInventory) AS itemnumber,"
               "       COALESCE(uom_name, :na) AS uomname,"
               "       CASE WHEN (LENGTH(TRIM(BOTH '    ' FROM poitem_vend_item_descrip)) <= 0) THEN "
               "                 (item_descrip1 || ' ' || item_descrip2) "
               "            ELSE poitem_vend_item_descrip "
               "       END AS itemdescription, "
               "      'qty' AS poitem_qty_ordered_xtnumericrole,"
               "      'qty' AS poitem_qty_received_xtnumericrole,"
               "      'qty' AS poitem_qty_returned_xtnumericrole "
               "FROM poitem LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id))"
               "     ON (poitem_itemsite_id=itemsite_id) "
               "WHERE (poitem_pohead_id=:pohead_id) "
               "ORDER BY poitem_linenumber;" );
    q.bindValue(":nonInventory", tr("Non-Inventory"));
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    _poitem->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _poitem->clear();
}

void dspPoHistory::sPrint()
{
  ParameterList params;
  params.append("pohead_id", _po->id());

  orReport report("POHistory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoHistory::sCopy()
{
  ParameterList params;
  params.append("pohead_id", _po->id());

  copyPurchaseOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
