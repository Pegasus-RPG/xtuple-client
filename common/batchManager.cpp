/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "batchManager.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "metasql.h"
#include "xsqlquery.h"

#include "batchItem.h"

batchManager::batchManager(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, fl)
{
  setupUi(this);
  if (name)
    setObjectName(name);

  QButtonGroup * buttonGroup = new QButtonGroup(this);
  buttonGroup->addButton(_currentUser);
  buttonGroup->addButton(_allUsers);
  buttonGroup->addButton(_selectedUser);

  connect(_autoUpdate,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_batch, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_batch, SIGNAL(itemSelectionChanged()),    this, SLOT(sHandleButtons()));
  connect(_cancel,       SIGNAL(clicked()),          this, SLOT(sCancel()));
  connect(_reschedule,   SIGNAL(clicked()),          this, SLOT(sReschedule()));
  connect(_showCompleted,SIGNAL(toggled(bool)),      this, SLOT(sFillList()));
  connect(_usr,          SIGNAL(newID(int)),         this, SLOT(sFillList()));
  connect(_view,         SIGNAL(clicked()),          this, SLOT(sView()));
  connect(buttonGroup,   SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));

  _db = QSqlDatabase();
  
  _timer = new QTimer(this);

  _batch->addColumn( tr("User"),          _dateColumn, Qt::AlignLeft, true, "batch_user");
  _batch->addColumn( tr("Action"),        _itemColumn, Qt::AlignLeft, true, "batch_action");
  _batch->addColumn( tr("Scheduled"), _timeDateColumn, Qt::AlignLeft, true, "batch_scheduled");
  _batch->addColumn( tr("Run Status"),_timeDateColumn, Qt::AlignLeft, true, "runstatus");
  _batch->addColumn( tr("Completed"), _timeDateColumn, Qt::AlignLeft, true, "batch_completed");
  _batch->addColumn( tr("Exit Status"),            -1, Qt::AlignLeft, true, "exitstatus" );

  setDatabase(_db);
}

batchManager::~batchManager()
{
  // no need to delete child widgets, Qt does it all for us
}

void batchManager::languageChange()
{
  retranslateUi(this);
}

void batchManager::sPopulateMenu(QMenu *)
{
}

void batchManager::sFillList()
{
  QString sql( "SELECT batch_id, *,"
               "       CASE WHEN (batch_started IS NULL)   THEN <? value(\"scheduled\") ?>"
               "            WHEN (batch_completed IS NULL) THEN <? value(\"inProcess\") ?>"
               "            ELSE <? value(\"completed\") ?>"
               "       END AS runstatus,"
               "       firstLine(batch_exitstatus) AS exitstatus "
               "  FROM xtbatch.batch "
               " WHERE((true) "
               "<? if not exists(\"showCompleted\") ?>"
               "   AND (batch_completed IS NULL)"
               "<? endif ?>"
               "<? if exists(\"username\") ?>"
               "   AND (batch_user=<? value(\"username\") ?>)"
               "<? elseif exists(\"current_user\") ?>"
               "   AND (batch_user=CURRENT_USER)"
               "<? endif ?>"
               ") "
               "ORDER BY batch_scheduled, batch_completed;" );

  ParameterList params;
  params.append("scheduled", tr("Scheduled"));
  params.append("inProcess", tr("In-Process"));
  params.append("completed", tr("Done"));
  if (_showCompleted->isChecked())
    params.append("showCompleted");
  if (_selectedUser->isChecked())
    params.append("username", _usr->currentText());
  else if(_currentUser->isChecked())
    params.append("current_user");

  XSqlQuery batch(_db);
  MetaSQLQuery mql(sql);
  batch = mql.toQuery(params, _db);
  _batch->populate(batch);
  if (batch.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this,
                          tr("System Error in %1::%2").arg(__FILE__, __LINE__),
                          batch.lastError().databaseText());
    return;
  }

  _timer->stop();
  if (_autoUpdate->isChecked())
    _timer->singleShot(60000, this, SLOT(sFillList()));
}

void batchManager::sHandleButtons()
{
  QList<XTreeWidgetItem*> list = _batch->selectedItems();
  _cancel->setEnabled(list.size() > 0);
  _reschedule->setEnabled(list.size() == 1);
  _view->setEnabled(list.size() == 1);
}

void batchManager::sReschedule()
{
  if (batchItem::Reschedule(_batch->id(), this, _db) != QDialog::Rejected)
    sFillList();
}

void batchManager::sView()
{
  if (batchItem::view(_batch->id(), this, _db) != QDialog::Rejected)
    sFillList();
}

void batchManager::sCancel()
{
  XSqlQuery cancel(_db);
  cancel.prepare("SELECT xtbatch.cancelBatchItem(:batch_id) AS result;");

  QList<XTreeWidgetItem*> list = _batch->selectedItems();
  if (list.size() > 1 &&
      QMessageBox::question(this, tr("Cancel?"),
                            tr("Are you sure you want to cancel %1 jobs?")
                            .arg(list.size()),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = 0; i < list.size(); i++)
  {
    cancel.bindValue(":batch_id", list.at(i)->id());
    cancel.exec();
  }

  sFillList();
}


void batchManager::setDatabase( QSqlDatabase db )
{
  //if(_db.isValid())
  //  disconnect(&_db, SIGNAL(destroyed()), this, SLOT(close()));

  _db = db;
  if(_db.isValid())
    setWindowTitle(tr("Batch Manager -- on %1/%2 AS %3")
               .arg(_db.hostName())
               .arg(_db.databaseName())
               .arg(_db.userName()));
  else
    setWindowTitle(tr("Batch Manager"));
  
  //if(_db.isValid())
  //  connect(&_db, SIGNAL(destroyed()), this, SLOT(close()));

  XSqlQuery users( "SELECT usr_id, usr_username, usr_username"
                   "  FROM usr "
                   " ORDER BY usr_username", _db );
  _usr->populate(users);
  
  sFillList();
}

void batchManager::setViewOtherEvents( bool viewOtherEvents )
{
  _allUsers->setEnabled(viewOtherEvents);
  _selectedUser->setEnabled(viewOtherEvents);
  if (!viewOtherEvents)
    _currentUser->setChecked(TRUE);
}
