/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "workOrderOperations.h"

#include <QMessageBox>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "woOperation.h"

workOrderOperations::workOrderOperations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_wooper, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewOperation()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditOperation()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sViewOperation()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDeleteOperation()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wooper->addColumn(tr("Seq #"),         _seqColumn,  Qt::AlignCenter, true,  "wooper_seqnumber" );
  _wooper->addColumn(tr("Work Center"),   _itemColumn, Qt::AlignLeft,   true,  "wrkcnt_code"   );
  _wooper->addColumn(tr("Std. Oper."),    _itemColumn, Qt::AlignLeft,   true,  "stdoperation"   );
  _wooper->addColumn(tr("Description"),   -1,          Qt::AlignLeft,   true,  "wooperdescrip"   );
  _wooper->addColumn(tr("Setup Remain."), _itemColumn, Qt::AlignRight,  true,  "setupremain"  );
  _wooper->addColumn(tr("Status"),        _ynColumn,   Qt::AlignRight,  true,  "setupcomplete"  );
  _wooper->addColumn(tr("Run Remain."),   _itemColumn, Qt::AlignRight,  true,  "runremain"  );
  _wooper->addColumn(tr("Status"),        _ynColumn,   Qt::AlignRight,  true,  "runcomplete"  );
  
  if (_privileges->check("MaintainWoOperations"))
  {
    connect(_wooper, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_wooper, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_wooper, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(workOrderOperationsUpdated(int, int, bool)), this, SLOT(sCatchOperationsUpdated(int, int, bool)));

  _wo->setFocus();
}

workOrderOperations::~workOrderOperations()
{
    // no need to delete child widgets, Qt does it all for us
}

void workOrderOperations::languageChange()
{
  retranslateUi(this);
}

enum SetResponse workOrderOperations::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wooper->setFocus();
  }

  return NoError;
}

void workOrderOperations::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if ((!_privileges->check("ViewWoOperations")) && (!_privileges->check("MaintainWoOperations")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Delete Operation..."), this, SLOT(sDeleteOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    menu->setItemEnabled(menuItem, FALSE);
}

void workOrderOperations::sNewOperation()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("wo_id", _wo->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderOperations::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderOperations::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderOperations::sDeleteOperation()
{
  if (QMessageBox::question( this, tr("Delete W/O Operation"),
                             tr("<p>If you Delete the selected W/O Operation "
				"you will not be able to post Labor to this "
				"Operation to this W/O. Are you sure that you "
				"want to delete the selected W/O Operation?" ),
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM wooper "
               "WHERE (wooper_id=:wooper_id);" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if (q.first())
      omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooper->id(), TRUE);
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

void workOrderOperations::sMoveUp()
{
  q.prepare("SELECT moveWooperUp(:wooper_id) AS result;");
  q.bindValue(":wooper_id", _wooper->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("moveWooperUp", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooper->id(), TRUE);
}

void workOrderOperations::sMoveDown()
{
  q.prepare("SELECT moveWooperDown(:wooper_id) AS result;");
  q.bindValue(":wooper_id", _wooper->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("moveWooperDown", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooper->id(), TRUE);
}

void workOrderOperations::sCatchOperationsUpdated(int pWoid, int, bool)
{
  if (pWoid == _wo->id())
    sFillList();
}

void workOrderOperations::sFillList()
{
  q.prepare( "SELECT wooper_id, wooper_seqnumber, wrkcnt_code,"
             "       COALESCE(stdopn_number, :none) AS stdoperation,"
             "       (wooper_descrip1 || ' ' || wooper_descrip2) AS wooperdescrip,"
             "       noNeg(wooper_sutime - wooper_suconsumed) AS setupremain,"
             "       CASE WHEN (wooper_sucomplete) THEN :complete"
             "       END AS setupcomplete,"
             "       noNeg(wooper_rntime - wooper_rnconsumed) AS runremain,"
             "       CASE WHEN (wooper_rncomplete) THEN :complete"
             "       END AS runcomplete,"
             "       '1' AS setupremain_xtnumericrole,"
             "       '1' AS runremain_xtnumericrole "
             "FROM wrkcnt,"
             "     wooper LEFT OUTER JOIN stdopn ON (wooper_stdopn_id=stdopn_id) "
             "WHERE ( (wooper_wrkcnt_id=wrkcnt_id)"
             " AND (wooper_wo_id=:wo_id) ) "
             "ORDER BY wooper_seqnumber;" );
  q.bindValue(":none", tr("None"));
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  _wooper->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
