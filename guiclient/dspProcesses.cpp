/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspProcesses.h"

#include <QMenu>

#include "errorReporter.h"
#include "parameterwidget.h"
#include "xtreewidget.h"

dspProcesses::dspProcesses(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspProcesses", fl)
{
  setWindowTitle(tr("Process/Lock Manager"));
  setListLabel(tr("Database Processes And Locks"));
  setMetaSQLOptions("processes", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setQueryOnStartEnabled(true);
  setAutoUpdateEnabled(true);

  QString qryStatus = QString("SELECT 1, '%1' "
                              "UNION SELECT 2, '%2' "
                              "UNION SELECT 3, '%3' "
                              "UNION SELECT 4, '%4' "
                              "UNION SELECT 5, '%5' "
                              "UNION SELECT 6, '%6';")
                             .arg(tr("Active"))
                             .arg(tr("Idle"))
                             .arg(tr("Idle In Transaction"))
                             .arg(tr("Idle In Transaction (Aborted)"))
                             .arg(tr("Fastpath Function Call"))
                             .arg(tr("Disabled"));

  parameterWidget()->append(tr("User"),        "usename",          ParameterWidget::User);
  parameterWidget()->append(tr("Application"), "application_name", ParameterWidget::Text);
  parameterWidget()->append(tr("Start Date"),  "startdate",        ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"),    "enddate",          ParameterWidget::Date);
  parameterWidget()->append(tr("Status"),      "state",            ParameterWidget::Multiselect,
                            QVariant(), false, qryStatus);
  parameterWidget()->append(tr("Table"),       "table",            ParameterWidget::Text);

  list()->addColumn(tr("Type"),               _statusColumn,   Qt::AlignLeft, true, "type");
  list()->addColumn(tr("Current Connection"), _ynColumn,       Qt::AlignLeft, true, "current");
  list()->addColumn(tr("Client"),             _itemColumn,     Qt::AlignLeft, true, "client");
  list()->addColumn(tr("Port"),               _itemColumn,     Qt::AlignLeft, true, "port");
  list()->addColumn(tr("User"),               _userColumn,     Qt::AlignLeft, true, "usename");
  list()->addColumn(tr("Application"),        _itemColumn,     Qt::AlignLeft, true, "application_name");
  list()->addColumn(tr("Status"  ),           _statusColumn,   Qt::AlignLeft, true, "state");

  list()->addColumn(tr("Started"),            _timeDateColumn, Qt::AlignLeft, true, "backend_start");
  list()->addColumn(tr("Last Active"),        _timeDateColumn, Qt::AlignLeft, true, "state_change");
  list()->addColumn(tr("Table"),              _itemColumn,     Qt::AlignLeft, true, "tablename");
  list()->addColumn(tr("Record"),             _itemColumn,     Qt::AlignLeft, true, "record");
  list()->addColumn(tr("Orphaned"),           _ynColumn,       Qt::AlignLeft, true, "orphaned");

  XSqlQuery mobilized("SELECT EXISTS(SELECT 1 "
                      "                FROM pg_class "
                      "                JOIN pg_namespace ON relnamespace = pg_namespace.oid "
                      "               WHERE relname='lock' "
                      "                 AND nspname='xt') AS mobilized;");
  if (mobilized.first())
    _mobilized = mobilized.value("mobilized").toBool();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error checking if mobilized"),
                                mobilized, __FILE__, __LINE__);
}

bool dspProcesses::setParams(ParameterList &params)
{
  params.append("process",        tr("Process"));
  params.append("lock",           tr("Lock"));
  params.append("active",         tr("Active"));
  params.append("idle",           tr("Idle"));
  params.append("idletrans",      tr("Idle In Transaction"));
  params.append("idletransabort", tr("Idle In Transaction (Aborted)"));
  params.append("fastpath",       tr("Fastpath Function Call"));
  params.append("disabled",       tr("Disabled"));
  params.append("yes",            tr("Yes"));
  params.append("no",             tr("No"));

  if (_mobilized)
    params.append("mobilized");

  if (!display::setParams(params))
    return false;

  return true;
}

void dspProcesses::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  if (list()->altId()==0)
  {
    QAction *menuItem;
    menuItem = pMenu->addAction(tr("Kill Process"), this, SLOT(sKill()));
    menuItem->setEnabled(!list()->rawValue("current").toBool());
  }
  else
    pMenu->addAction(tr("Release Lock"), this, SLOT(sRelease()));
}

void dspProcesses::sKill()
{
  XSqlQuery locks;
  if (_mobilized)
    locks.prepare("DELETE FROM xt.lock "
                  " WHERE lock_pid=:pid;");
  else
    locks.prepare("SELECT pg_advisory_unlock(classid::INTEGER, objid::INTEGER) "
                  "  FROM pg_locks "
                  " WHERE pid=:pid;");
  locks.bindValue(":pid", list()->id());
  locks.exec();

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Fetching Locks"),
                           locks, __FILE__, __LINE__))
    return;

  XSqlQuery kill;
  kill.prepare("SELECT pg_terminate_backend(:pid);");
  kill.bindValue(":pid", list()->id());
  kill.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Killing Process"),
                           kill, __FILE__, __LINE__))
    return;

  sFillList();
}

void dspProcesses::sRelease()
{
  XSqlQuery release;
  if (_mobilized)
  {
    release.prepare("DELETE FROM xt.lock "
                    " WHERE lock_id=:id;");
    release.bindValue(":id", list()->id());
  }
  else
  {
    release.prepare("SELECT pg_advisory_unlock(oid::INTEGER, :record) "
                    "  FROM pg_class "
                    " WHERE relname=:table;");
    release.bindValue(":table", list()->rawValue("tablename").toString());
    release.bindValue(":record", list()->rawValue("record").toInt());
  }
  release.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Releasing Lock"),
                           release, __FILE__, __LINE__))
    return;

  sFillList();
}
