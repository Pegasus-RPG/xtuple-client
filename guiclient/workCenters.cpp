/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "workCenters.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"
#include "workCenter.h"

workCenters::workCenters(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_wrkcnt, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _wrkcnt->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _wrkcnt->addColumn(tr("Work Cntr."),  _itemColumn, Qt::AlignLeft,   true,  "wrkcnt_code"   );
  _wrkcnt->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "wrkcnt_descrip"   );

  connect(omfgThis, SIGNAL(workCentersUpdated()), SLOT(sFillList()));

  if (_privileges->check("MaintainWorkCenters"))
  {
    connect(_wrkcnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_wrkcnt, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_wrkcnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_wrkcnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_wrkcnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

workCenters::~workCenters()
{
  // no need to delete child widgets, Qt does it all for us
}

void workCenters::languageChange()
{
  retranslateUi(this);
}

void workCenters::sPrint()
{
  orReport report("WorkCentersMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void workCenters::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  workCenter *newdlg = new workCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void workCenters::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wrkcnt_id", _wrkcnt->id());

  workCenter *newdlg = new workCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void workCenters::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("wrkcnt_id", _wrkcnt->id());

  workCenter *newdlg = new workCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void workCenters::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wrkcnt_id", _wrkcnt->id());

  workCenter *newdlg = new workCenter();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void workCenters::sDelete()
{
  q.prepare("SELECT deleteWorkCenter(:wrkcnt_id) AS result;");
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteWorkCenter", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void workCenters::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Work Center"), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainWorkCenters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Work Center"), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Copy Work Center"), this, SLOT(sCopy()), 0);
  if (!_privileges->check("MaintainWorkCenters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Work Center"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainWorkCenters"))
    pMenu->setItemEnabled(menuItem, FALSE);

}

void workCenters::sFillList()
{
  _wrkcnt->populate( "SELECT wrkcnt_id, warehous_code,"
                     "       wrkcnt_code, wrkcnt_descrip "
                     "FROM wrkcnt, site() "
                     "WHERE (wrkcnt_warehous_id=warehous_id) "
                     "ORDER BY wrkcnt_code" );
}
