/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "project.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <openreports.h>
#include <comment.h>
#include "task.h"

project::project(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_printTasks, SIGNAL(clicked()), this, SLOT(sPrintTasks()));
  connect(_newTask, SIGNAL(clicked()), this, SLOT(sNewTask()));
  connect(_editTask, SIGNAL(clicked()), this, SLOT(sEditTask()));
  connect(_viewTask, SIGNAL(clicked()), this, SLOT(sViewTask()));
  connect(_deleteTask, SIGNAL(clicked()), this, SLOT(sDeleteTask()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sNumberChanged()));

  _prjtask->addColumn( tr("Number"),      _itemColumn, Qt::AlignRight, true, "prjtask_number" );
  _prjtask->addColumn( tr("Name"),        _itemColumn, Qt::AlignLeft,  true, "prjtask_name"  );
  _prjtask->addColumn( tr("Description"), -1,          Qt::AlignLeft,  true, "prjtask_descrip" ); 

  _owner->setUsername(omfgThis->username());
  _assignedTo->setUsername(omfgThis->username());

  populate();
}

/*
 *  Destroys the object and frees any allocated resources
 */
project::~project()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void project::languageChange()
{
    retranslateUi(this);
}

enum SetResponse project::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
    _assignedTo->setUsername(param.toString());

  param = pParams.value("prj_id", &valid);
  if (valid)
  {
    _prjid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      connect(_prjtask, SIGNAL(valid(bool)), _editTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(valid(bool)), _deleteTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(itemSelected(int)), _editTask, SLOT(animateClick()));

      q.exec("SELECT NEXTVAL('prj_prj_id_seq') AS prj_id;");
      if (q.first())
        _prjid = q.value("prj_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      _comments->setId(_prjid);

      _assignedTo->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
			      _privileges->check("ReassignTodoListItem"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _number->setEnabled(FALSE);

      connect(_prjtask, SIGNAL(valid(bool)), _editTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(valid(bool)), _deleteTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(itemSelected(int)), _editTask, SLOT(animateClick()));

      _assignedTo->setEnabled(_privileges->check("MaintainOtherTodoLists") ||
	                      _privileges->check("ReassignTodoListItem"));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _owner->setEnabled(FALSE);
      _number->setEnabled(FALSE);
      _status->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _so->setEnabled(FALSE);
      _wo->setEnabled(FALSE);
      _po->setEnabled(FALSE);
      _assignedTo->setEnabled(FALSE);
      _newTask->setEnabled(FALSE);
      connect(_prjtask, SIGNAL(itemSelected(int)), _viewTask, SLOT(animateClick()));
      _comments->setReadOnly(TRUE);
      _started->setEnabled(FALSE);
      _assigned->setEnabled(FALSE);
      _due->setEnabled(FALSE);
      _completed->setEnabled(FALSE);
    }
  }
    
  return NoError;
}

void project::populate()
{
  q.prepare( "SELECT prj_number, prj_name, prj_descrip,"
             "       prj_so, prj_wo, prj_po, prj_status, "
             "       prj_owner_username, prj_username, prj_start_date, "
             "       prj_assigned_date, prj_due_date, prj_completed_date "
             "FROM prj "
             "WHERE (prj_id=:prj_id);" );
  q.bindValue(":prj_id", _prjid);
  q.exec();
  if (q.first())
  {
    _owner->setUsername(q.value("prj_owner_username").toString());
    _number->setText(q.value("prj_number").toString());
    _name->setText(q.value("prj_name").toString());
    _descrip->setText(q.value("prj_descrip").toString());
    _so->setChecked(q.value("prj_so").toBool());
    _wo->setChecked(q.value("prj_wo").toBool());
    _po->setChecked(q.value("prj_po").toBool());
    _assignedTo->setUsername(q.value("prj_username").toString());
    _started->setDate(q.value("prj_start_date").toDate());
    _assigned->setDate(q.value("prj_assigned_date").toDate());
    _due->setDate(q.value("prj_due_date").toDate());
    _completed->setDate(q.value("prj_completed_date").toDate());
    QString status = q.value("prj_status").toString();
    if("P" == status)
      _status->setCurrentIndex(0);
    else if("O" == status)
      _status->setCurrentIndex(1);
    else if("C" == status)
      _status->setCurrentIndex(2);
  }

  sFillTaskList();
  _comments->setId(_prjid);
}

void project::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM prjtask "
               "WHERE (prjtask_prj_id=:prj_id);" );
    q.bindValue(":prj_id", _prjid);
    q.exec();
  }

  reject();
}

void project::sSave()
{
  if (_number->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save Project"),
      tr("No Project Number was specified. You must specify a project number.") );
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO prj "
               "( prj_id, prj_number, prj_name, prj_descrip,"
               "  prj_so, prj_wo, prj_po, prj_status, prj_owner_username, "
               "  prj_start_date, prj_due_date, prj_assigned_date,"
               "  prj_completed_date, prj_username ) "
               "VALUES "
               "( :prj_id, :prj_number, :prj_name, :prj_descrip,"
               "  :prj_so, :prj_wo, :prj_po, :prj_status, :prj_owner_username,"
               "  :prj_start_date, :prj_due_date, :prj_assigned_date,"
               "  :prj_completed_date, :username  );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE prj "
               "SET prj_number=:prj_number, prj_name=:prj_name, prj_descrip=:prj_descrip,"
               "    prj_so=:prj_so, prj_wo=:prj_wo, prj_po=:prj_po, prj_status=:prj_status, "
               "    prj_owner_username=:prj_owner_username, prj_start_date=:prj_start_date, "
               "    prj_due_date=:prj_due_date, prj_assigned_date=:prj_assigned_date,"
               "    prj_completed_date=:prj_completed_date, prj_username=:username  "
               "WHERE (prj_id=:prj_id);" );

  q.bindValue(":prj_id", _prjid);
  q.bindValue(":prj_number", _number->text().trimmed().toUpper());
  q.bindValue(":prj_name", _name->text());
  q.bindValue(":prj_descrip", _descrip->toPlainText());
  q.bindValue(":prj_so", QVariant(_so->isChecked()));
  q.bindValue(":prj_wo", QVariant(_wo->isChecked()));
  q.bindValue(":prj_po", QVariant(_po->isChecked()));
  q.bindValue(":prj_owner_username", _owner->username());
  q.bindValue(":username",   _assignedTo->username());
  q.bindValue(":prj_start_date",	_started->date());
  q.bindValue(":prj_due_date",	_due->date());
  q.bindValue(":prj_assigned_date",	_assigned->date());
  q.bindValue(":prj_completed_date",	_completed->date());

  switch(_status->currentIndex())
  {
    case 0:
    default:
      q.bindValue(":prj_status", "P");
      break;
    case 1:
      q.bindValue(":prj_status", "O");
      break;
    case 2:
      q.bindValue(":prj_status", "C");
      break;
  }
  q.exec();

  done(_prjid);
}

void project::sPrintTasks()
{
  ParameterList params;

  params.append("prj_id", _prjid);

  orReport report("ProjectTaskList", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void project::sNewTask()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _prjid);
  params.append("prj_owner_username", _owner->username());
  params.append("prj_username",   _assignedTo->username());
  params.append("prj_start_date",	_started->date());
  params.append("prj_due_date",	_due->date());
  params.append("prj_assigned_date",	_assigned->date());
  params.append("prj_completed_date",	_completed->date());

  task newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTaskList();
}

void project::sEditTask()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtask_id", _prjtask->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTaskList();
}

void project::sViewTask()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", _prjtask->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void project::sDeleteTask()
{
  q.prepare("DELETE FROM prjtask"
            " WHERE (prjtask_id=:prjtask_id); ");
  q.bindValue(":prjtask_id", _prjtask->id());
  q.exec();
  sFillTaskList();
}

void project::sFillTaskList()
{
  q.prepare( "SELECT prjtask_id, prjtask_number, prjtask_name, "
             "firstLine(prjtask_descrip) AS prjtask_descrip "
             "FROM prjtask "
             "WHERE (prjtask_prj_id=:prj_id) "
             "ORDER BY prjtask_number;" );
  q.bindValue(":prj_id", _prjid);
  q.exec();
  _prjtask->populate(q);
}

void project::sNumberChanged()
{
  if((cNew == _mode) && (_number->text().length()))
  {
    _number->setText(_number->text().trimmed().toUpper());

    q.prepare( "SELECT prj_id"
               "  FROM prj"
               " WHERE (prj_number=:prj_number);" );
    q.bindValue(":prj_number", _number->text());
    q.exec();
    if(q.first())
    {
      _number->setEnabled(FALSE);
      _prjid = q.value("prj_id").toInt();
      _mode = cEdit;
      populate();
    }
    else
    {
      _number->setEnabled(FALSE);
      _mode = cNew;
    }
  }
}
