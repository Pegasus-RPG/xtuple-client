/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUsageStatisticsByWarehouse.h"

#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include "dspInventoryHistoryByItem.h"

dspUsageStatisticsByWarehouse::dspUsageStatisticsByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_usage, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _usage->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"  );
  _usage->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"  );
  _usage->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight,  true,  "received" );
  _usage->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight,  true,  "issued" );
  _usage->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight,  true,  "sold" );
  _usage->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight,  true,  "scrap" );
  _usage->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight,  true,  "adjust" );

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

dspUsageStatisticsByWarehouse::~dspUsageStatisticsByWarehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUsageStatisticsByWarehouse::languageChange()
{
  retranslateUi(this);
}

void dspUsageStatisticsByWarehouse::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  params.append("warehous_id", _warehouse->id());

  orReport report("UsageStatisticsByWarehouse", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUsageStatisticsByWarehouse::sViewTransactions()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("itemsite_id", _usage->altId());
  params.append("run");

  switch (_column)
  {
    case 2:
      params.append("transtype", "R");
      break;

    case 3:
      params.append("transtype", "I");
      break;

    case 4:
      params.append("transtype", "S");
      break;

    case 5:
      params.append("transtype", "SC");
      break;

    case 6:
      params.append("transtype", "A");
      break;
  }

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatisticsByWarehouse::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;

  _column = pColumn;

  intMenuItem = menuThis->insertItem(tr("View Transactions..."), this, SLOT(sViewTransactions()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void dspUsageStatisticsByWarehouse::sFillList()
{
  _usage->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  q.prepare( "SELECT item_id, itemsite_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
             "       summTransR(itemsite_id, :startDate, :endDate) AS received,"
             "       summTransI(itemsite_id, :startDate, :endDate) AS issued,"
             "       summTransS(itemsite_id, :startDate, :endDate) AS sold,"
             "       summTransC(itemsite_id, :startDate, :endDate) AS scrap,"
             "       summTransA(itemsite_id, :startDate, :endDate) AS adjust,"
             "       'qty' AS received_xtnumericrole,"
             "       'qty' AS issued_xtnumericrole,"
             "       'qty' AS sold_xtnumericrole,"
             "       'qty' AS scrap_xtnumericrole,"
             "       'qty' AS adjust_xtnumericrole "
             "FROM itemsite, item "
             "WHERE ( (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=:warehous_id) ) "
             "ORDER BY item_number;" );
  _dates->bindValue(q);
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _usage->populate(q, TRUE);
}

