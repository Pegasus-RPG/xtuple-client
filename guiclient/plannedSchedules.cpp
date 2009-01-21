/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannedSchedules.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>

#include <openreports.h>

#include "plannedSchedule.h"
#include "copyPlannedSchedule.h"

plannedSchedules::plannedSchedules(QWidget * parent, const char * name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

//  statusBar()->hide();

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _list->addColumn(tr("Start Date"),   _dateColumn, Qt::AlignCenter, true,  "pschhead_start_date" );
  _list->addColumn(tr("End Date"),     _dateColumn, Qt::AlignCenter, true,  "pschhead_end_date" );
  _list->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _list->addColumn(tr("Schd. Number"), _itemColumn, Qt::AlignLeft,   true,  "pschhead_number"   );
  _list->addColumn(tr("Status"),       _whsColumn,  Qt::AlignCenter, true,  "pschhead_status" );
  _list->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "pschhead_descrip"   );

  sFillList();
}

plannedSchedules::~plannedSchedules()
{
}

void plannedSchedules::languageChange()
{
  retranslateUi(this);
}

void plannedSchedules::sPopulateMenu(QMenu* pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Release"), this, SLOT(sRelease()), 0);
}

void plannedSchedules::sRelease()
{
  q.prepare("UPDATE pschhead SET pschhead_status = 'R' WHERE (pschhead_id=:pschhead_id);");
  q.bindValue(":pschhead_id", _list->id());
  q.exec();
  sFillList();
}

void plannedSchedules::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  plannedSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedules::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pschhead_id", _list->id());

  plannedSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedules::sCopy()
{
  ParameterList params;
  params.append("pschhead_id", _list->id());

  copyPlannedSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedules::sPrint()
{
  orReport report("PlannedSchedulesMasterList");
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void plannedSchedules::sFillList()
{
  q.prepare("SELECT pschhead_id,"
            "       pschhead_start_date,"
            "       pschhead_end_date,"
            "       warehous_code,"
            "       pschhead_number,"
            "       pschhead_status,"
            "       pschhead_descrip "
            "  FROM pschhead JOIN site() ON (pschhead_warehous_id=warehous_id) "
            " ORDER BY pschhead_start_date, pschhead_end_date, warehous_code, pschhead_number; ");
  q.exec();
  _list->populate(q);
}

