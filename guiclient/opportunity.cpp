/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunity.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QMenu>

#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "characteristicAssignment.h"

/*
 *  Constructs a opportunity as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
opportunity::opportunity(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_deleteTodoItem, SIGNAL(clicked()), this, SLOT(sDeleteTodoItem()));
  connect(_editTodoItem, SIGNAL(clicked()), this, SLOT(sEditTodoItem()));
  connect(_newTodoItem, SIGNAL(clicked()), this, SLOT(sNewTodoItem()));
  connect(_todoList, SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  connect(_todoList, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateTodoMenu(QMenu*)));
  connect(_todoList, SIGNAL(valid(bool)), this, SLOT(sHandleTodoPrivs()));
  connect(_viewTodoItem, SIGNAL(clicked()), this, SLOT(sViewTodoItem()));
  connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
  connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));

  _probability->setValidator(0);
  
  _opheadid = -1;

  _todoList->addColumn(tr("Priority"),   _userColumn, Qt::AlignRight, true, "incdtpriority_name");
  _todoList->addColumn(tr("User"),       _userColumn, Qt::AlignLeft,  true, "todoitem_username" );
  _todoList->addColumn(tr("Name"),               100, Qt::AlignLeft,  true, "todoitem_name" );
  _todoList->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "todoitem_description" );
  _todoList->addColumn(tr("Status"),   _statusColumn, Qt::AlignLeft,  true, "todoitem_status" );
  _todoList->addColumn(tr("Due Date"),   _dateColumn, Qt::AlignLeft,  true, "todoitem_due_date" );

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft,  true, "char_name" );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft,  true, "charass_value" );
  _charass->addColumn(tr("Default"),        _ynColumn,   Qt::AlignCenter,true, "charass_default" );

  _owner->setUsername(omfgThis->username());

  _saved = false;
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunity::~opportunity()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunity::languageChange()
{
  retranslateUi(this);
}

enum SetResponse opportunity::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("ophead_id", &valid);
  if (valid)
  {
    _opheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    param = pParams.value("crmacct_id", &valid);
    if (valid)
      _crmacct->setId(param.toInt());

    if (param.toString() == "new")
    {
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _comments->setReadOnly(true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _crmacct->setEnabled(true);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _crmacct->setEnabled(false);
      _owner->setEnabled(false);
      _oppstage->setEnabled(false);
      _oppsource->setEnabled(false);
      _opptype->setEnabled(false);
      _notes->setReadOnly(true);
      _name->setEnabled(false);
      _targetDate->setEnabled(false);
      _actualDate->setEnabled(false);
      _amount->setEnabled(false);
      _probability->setEnabled(false);
      _deleteTodoItem->setEnabled(false);
      _editTodoItem->setEnabled(false);
      _newTodoItem->setEnabled(false);
      _newCharacteristic->setEnabled(FALSE);

      _save->hide();
      _cancel->setText(tr("&Close"));
      _cancel->setFocus();
      _comments->setReadOnly(true);
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  sHandleTodoPrivs();
  return NoError;
}

void opportunity::sCancel()
{
  if (_saved && cNew == _mode)
  {
    q.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");
    q.bindValue(":ophead_id", _opheadid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteOpportunity", result));
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  reject();
}

void opportunity::sSave()
{
  if (! save(false)) // if error
    return;

  accept();
}

bool opportunity::save(bool partial)
{
  if (! partial)
  {
    if(_crmacct->id() == -1)
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify the Account that this opportunity is for.") );
      return false;
    }

    if(_name->text().trimmed().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify a Name for this opportunity report.") );
      _name->setFocus();
      return false;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (!q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (cNew == _mode && !_saved)
    q.prepare("INSERT INTO ophead"
              "      (ophead_name, ophead_crmacct_id,"
              "       ophead_owner_username,"
              "       ophead_opstage_id, ophead_opsource_id,"
              "       ophead_optype_id, ophead_probability_prcnt,"
              "       ophead_amount, ophead_curr_id, ophead_target_date,"
              "       ophead_actual_date,"
              "       ophead_notes) "
              "VALUES(:ophead_name, :ophead_crmacct_id,"
              "       :ophead_owner_username,"
              "       :ophead_opstage_id, :ophead_opsource_id,"
              "       :ophead_optype_id, :ophead_probability_prcnt,"
              "       :ophead_amount, :ophead_curr_id, :ophead_target_date,"
              "       :ophead_actual_date,"
              "       :ophead_notes);" );
  else if (cEdit == _mode || _saved)
    q.prepare("UPDATE ophead"
              "   SET ophead_name=:ophead_name,"
              "       ophead_crmacct_id=:ophead_crmacct_id,"
              "       ophead_owner_username=:ophead_owner_username,"
              "       ophead_opstage_id=:ophead_opstage_id,"
              "       ophead_opsource_id=:ophead_opsource_id,"
              "       ophead_optype_id=:ophead_optype_id,"
              "       ophead_probability_prcnt=:ophead_probability_prcnt,"
              "       ophead_amount=:ophead_amount,"
              "       ophead_curr_id=:ophead_curr_id,"
              "       ophead_target_date=:ophead_target_date,"
              "       ophead_actual_date=:ophead_actual_date,"
              "       ophead_notes=:ophead_notes"
              " WHERE (ophead_id=:ophead_id); ");

  q.bindValue(":ophead_id", _opheadid);
  q.bindValue(":ophead_name", _name->text());
  if (_crmacct->id() > 0)
    q.bindValue(":ophead_crmacct_id", _crmacct->id());
  if(_owner->isValid())
    q.bindValue(":ophead_owner_username", _owner->username());
  if(_oppstage->isValid())
    q.bindValue(":ophead_opstage_id", _oppstage->id());
  if(_oppsource->isValid())
    q.bindValue(":ophead_opsource_id", _oppsource->id());
  if(_opptype->isValid())
    q.bindValue(":ophead_optype_id", _opptype->id());
  if(!_probability->text().isEmpty())
    q.bindValue(":ophead_probability_prcnt", _probability->text().toInt());
  if(!_amount->isZero())
  {
    q.bindValue(":ophead_amount", _amount->localValue());
    q.bindValue(":ophead_curr_id", _amount->id());
  }
  if(_targetDate->isValid())
    q.bindValue(":ophead_target_date", _targetDate->date());
  if(_actualDate->isValid())
    q.bindValue(":ophead_actual_date", _actualDate->date());
  q.bindValue(":ophead_notes", _notes->toPlainText());

  if(!q.exec() && q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  q.exec("COMMIT;");
  if(q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (cNew == _mode && ! _saved)
  {
    q.exec("SELECT CURRVAL('ophead_ophead_id_seq') AS result;");
    if (q.first())
      _opheadid = q.value("result").toInt();
    else if(q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  _saved = true;
  return true;
}

void opportunity::populate()
{ 
  q.prepare("SELECT ophead_name,"
            "       ophead_crmacct_id,"
            "       ophead_owner_username,"
            "       ophead_opstage_id, ophead_opsource_id,"
            "       ophead_optype_id,"
            "       ophead_probability_prcnt, ophead_amount,"
            "       COALESCE(ophead_curr_id, basecurrid()) AS curr_id,"
            "       ophead_target_date, ophead_actual_date,"
            "       ophead_notes"
            "  FROM ophead"
            " WHERE(ophead_id=:ophead_id); ");
  q.bindValue(":ophead_id", _opheadid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("ophead_name").toString());
    _crmacct->setId(q.value("ophead_crmacct_id").toInt());
    _owner->setUsername(q.value("ophead_owner_username").toString());

    _oppstage->setNull();
    if(!q.value("ophead_opstage_id").toString().isEmpty())
      _oppstage->setId(q.value("ophead_opstage_id").toInt());

    _oppsource->setNull();
    if(!q.value("ophead_opsource_id").toString().isEmpty())
      _oppsource->setId(q.value("ophead_opsource_id").toInt());

    _opptype->setNull();
    if(!q.value("ophead_optype_id").toString().isEmpty())
      _opptype->setId(q.value("ophead_optype_id").toInt());

    _probability->clear();
    if(!q.value("ophead_probability_prcnt").toString().isEmpty())
      _probability->setText(q.value("ophead_probability_prcnt").toDouble());

    _amount->clear();
    _amount->setId(q.value("curr_id").toInt());
    if(!q.value("ophead_amount").toString().isEmpty())
      _amount->setLocalValue(q.value("ophead_amount").toDouble());

    _targetDate->clear();
    if(!q.value("ophead_target_date").toString().isEmpty())
      _targetDate->setDate(q.value("ophead_target_date").toDate());

    _actualDate->clear();
    if(!q.value("ophead_actual_date").toString().isEmpty())
      _actualDate->setDate(q.value("ophead_actual_date").toDate());

    _notes->setText(q.value("ophead_notes").toString());

    _comments->setId(_opheadid);

    sFillTodoList();
    sFillCharList();
  }
}

void opportunity::sNewTodoItem()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("ophead_id", _opheadid);

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillTodoList();
}

void opportunity::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillTodoList();
}

void opportunity::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void opportunity::sDeleteTodoItem()
{
  q.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  q.bindValue(":todoitem_id", _todoList->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
    else
      sFillTodoList();
    }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void opportunity::sFillTodoList()
{
  q.prepare("SELECT todoitem_id, incdtpriority_name, incdtpriority_order, "
	    "       todoitem_username, todoitem_name, "
	    "       firstLine(todoitem_description) AS todoitem_description, "
	    "       todoitem_status, todoitem_due_date, "
            "       CASE "
            "         WHEN (todoitem_status != 'C' AND todoitem_due_date < current_date) THEN "
            "           'error' "
            "         WHEN (todoitem_status != 'C' AND todoitem_due_date > current_date) THEN "
            "           'altemphasis' "
            "       END AS qtforegroundrole "
	    "  FROM todoitem "
            "       LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
	    "WHERE ( (todoitem_ophead_id=:ophead_id) "
	    "  AND   (todoitem_active) ) "
	    "ORDER BY incdtpriority_order, todoitem_username;");

  q.bindValue(":ophead_id", _opheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _todoList->populate(q);
}

void opportunity::sPopulateTodoMenu(QMenu *pMenu)
{
  int menuItem;

  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainOtherTodoLists")) );

  bool viewPriv =
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewOtherTodoLists"));

  menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNewTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, newPriv);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sViewTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, viewPriv);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDeleteTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);
}

void opportunity::sHandleTodoPrivs()
{
  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = false;
  bool viewPriv = false;

  if(_todoList->currentItem())
  {
    editPriv = (cNew == _mode || cEdit == _mode) && (
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("MaintainOtherTodoLists")) );

    viewPriv =
      (omfgThis->username() == _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewPersonalTodoList")) ||
      (omfgThis->username() != _todoList->currentItem()->text("todoitem_username") && _privileges->check("ViewOtherTodoLists"));
  }

  _newTodoItem->setEnabled(newPriv);
  _editTodoItem->setEnabled(editPriv && _todoList->id() > 0);
  _viewTodoItem->setEnabled((editPriv || viewPriv) && _todoList->id() > 0);
  _deleteTodoItem->setEnabled(editPriv && _todoList->id() > 0);

  if (editPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_viewTodoItem, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_editTodoItem, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _viewTodoItem, SLOT(animateClick()));
  }
}

void opportunity::sNewCharacteristic()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("ophead_id", _opheadid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharList();
}

void opportunity::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharList();
}

void opportunity::sDeleteCharacteristic()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sFillCharList();
}

void opportunity::sFillCharList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value, charass_default "
             "FROM charass, char "
             "WHERE ( (charass_target_type='OPP')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:ophead_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":ophead_id", _opheadid);
  q.exec();
  _charass->populate(q);
}

