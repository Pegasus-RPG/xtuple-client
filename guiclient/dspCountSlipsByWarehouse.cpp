/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCountSlipsByWarehouse.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#define DEBUG true

dspCountSlipsByWarehouse::dspCountSlipsByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // before connect so we don't repeatedly trigger the query
  if (_preferences->boolean("XCheckBox/forgetful"))
    _showUnposted->setChecked(true);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_cntslip, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_showUnposted, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_numericSlips, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));

  _cntslip->addColumn(tr("Slip #"),     _itemColumn, Qt::AlignLeft,  true, "slipnumber");
  _cntslip->addColumn(tr("Tag #"),     _orderColumn, Qt::AlignLeft,  true, "invcnt_tagnumber");
  _cntslip->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _cntslip->addColumn(tr("Item"),       _itemColumn, Qt::AlignLeft,  true, "item_number");
  _cntslip->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "descrip");
  _cntslip->addColumn(tr("Entered"),    _dateColumn, Qt::AlignCenter,true, "cntslip_entered");
  _cntslip->addColumn(tr("By"),         _userColumn, Qt::AlignCenter,true, "user");
  _cntslip->addColumn(tr("Qty. Counted"),_qtyColumn, Qt::AlignRight, true, "cntslip_qty");
  _cntslip->addColumn(tr("Posted"),     _dateColumn, Qt::AlignCenter,true, "cntslip_posted");

  sFillList();
}

dspCountSlipsByWarehouse::~dspCountSlipsByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCountSlipsByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspCountSlipsByWarehouse::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if(_showUnposted->isChecked())
    params.append("showUnposted");

  if(_numericSlips->isChecked())
    params.append("asNumeric");

  orReport report("CountSlipsByWarehouse", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountSlipsByWarehouse::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspCountSlipsByWarehouse::sFillList()
{
  QString sql("SELECT cntslip_id, ");

  if (_numericSlips->isChecked())
    sql += "toNumeric(cntslip_number, 0) AS slipnumber,"
           "cntslip_number AS slipnumber_qtdisplayrole," ;
  else
    sql += "cntslip_number AS slipnumber, ";

  sql += " invcnt_tagnumber, warehous_code,"
         " item_number, (item_descrip1 || ' ' || item_descrip2) AS descrip,"
         " cntslip_entered, cntslip_username AS user,"
         " cntslip_qty, 'qty' AS cntslip_qty_xtnumericrole,"
         " cntslip_posted "
         "FROM cntslip, invcnt, itemsite, item, warehous "
         "WHERE ((cntslip_cnttag_id=invcnt_id)"
         " AND (invcnt_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (cntslip_entered BETWEEN :startDate AND :endDate)";

  if (!_showUnposted->isChecked())
    sql += " AND (cntslip_posted)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY cntslip_number";

  if (DEBUG)
    qDebug("dspCountSlipsByWarehouse::sFillList() about to populate _cntslip");

  q.prepare(sql);
  _dates->bindValue(q);
  _warehouse->bindValue(q);
  q.exec();
  _cntslip->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_numericSlips->isChecked() && _cntslip->topLevelItemCount() > 1)
  {
    if (DEBUG)
      qDebug("dspCountSlipsByWarehouse::sFillList() looking for slip # gaps "
             "in %d items", _cntslip->topLevelItemCount());
    XTreeWidgetItem *last =_cntslip->topLevelItem(0);
    int slipNumber = last->rawValue("slipnumber").toInt();
    for (int i = 1; i < _cntslip->topLevelItemCount(); i++)
    {
      XTreeWidgetItem *curr = _cntslip->topLevelItem(i);
      if (DEBUG)
        qDebug("row %d has slipNumber %d and current %d",
               i, slipNumber, curr->rawValue("slipnumber").toInt());
      if (slipNumber == (curr->rawValue("slipnumber").toInt() - 1) || slipNumber == -1)
        slipNumber = curr->rawValue("slipnumber").toInt();
      else if (slipNumber >= 0)
      {
        if (slipNumber == curr->rawValue("slipnumber").toInt() - 2)
          curr = new XTreeWidgetItem( _cntslip, last, -1,
                                    QVariant("----"), "----", "----", "----",
                                    tr("Missing Slip #%1").arg(slipNumber + 1),
                                    "----", "----", "----" );
        else
          curr = new XTreeWidgetItem( _cntslip, last, -1,
                                    QVariant("----"), "----", "----", "----",
                                    tr("Missing Slips #%1 to #%2").arg(slipNumber + 1).arg(curr->rawValue("slipnumber").toInt() - 1),
                                    "----", "----", "----" );

        curr->setTextColor(namedColor("error"));
        slipNumber = -1;
        //i++; // 'cause we just added an item!
      }
      last = curr;
    }
  }
}
