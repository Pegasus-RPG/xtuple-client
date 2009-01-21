/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedAvailableCapacityByWorkCenter.h"

#include <QMenu>
#include <QMessageBox>

#include <datecluster.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "dspWoOperationsByWorkCenter.h"

dspTimePhasedAvailableCapacityByWorkCenter::dspTimePhasedAvailableCapacityByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_load, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _load->addColumn(tr("Work Center"), _itemColumn, Qt::AlignLeft, true, "wrkcnt_code");
}

dspTimePhasedAvailableCapacityByWorkCenter::~dspTimePhasedAvailableCapacityByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTimePhasedAvailableCapacityByWorkCenter::languageChange()
{
  retranslateUi(this);
}

void dspTimePhasedAvailableCapacityByWorkCenter::sPrint()
{
  if (_periods->isPeriodSelected())
  {
    ParameterList params;

    _warehouse->appendValue(params);

    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());

    params.append("period_id_list", periodList);

    orReport report("TimePhasedAvailableCapacityByWorkCenter", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      return;
    }
  }
  else
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>The criteria you specified are not complete. "
			     "Please make sure all fields are correctly filled "
			     "out before running the report." ) );
}

void dspTimePhasedAvailableCapacityByWorkCenter::sViewLoad()
{
  ParameterList params;
  params.append("wrkcnt_id", _load->id());
  params.append("startDate", _columnDates[_column - 1].startDate);
  params.append("endDate", _columnDates[_column - 1].endDate);
  params.append("run");

  dspWoOperationsByWorkCenter *newdlg = new dspWoOperationsByWorkCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedAvailableCapacityByWorkCenter::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;

  _column = pColumn;
  if (_column > 0)
  {
    menuItem = menu->insertItem(tr("View Load..."), this, SLOT(sViewLoad()), 0);
    if ( (!_privileges->check("MaintainWoOperations")) && (!_privileges->check("ViewWoOperations")) )
      menu->setItemEnabled(menuItem, FALSE);
  }
}

void dspTimePhasedAvailableCapacityByWorkCenter::sFillList()
{
  _columnDates.clear();
  _load->setColumnCount(1);

  QString sql("SELECT wrkcnt_id, wrkcnt_code ");

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", (workCenterCapacity(wrkcnt_id, %1) - loadByWorkCenter(wrkcnt_id, %2)) AS %3,"
                   "  '1' AS %4_xtnumericrole ")
	   .arg(cursor->id())
	   .arg(cursor->id())
	   .arg(bucketname)
	   .arg(bucketname);

    _load->addColumn(formatDate(cursor->startDate()), _timeColumn, Qt::AlignRight, true, bucketname);

    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  sql += " FROM wrkcnt ";

  if (_warehouse->isSelected())
    sql += "WHERE (wrkcnt_warehous_id=:warehous_id) ";

  sql += "ORDER BY wrkcnt_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();
  _load->populate(q);
}
