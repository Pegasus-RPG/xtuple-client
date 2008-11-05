/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "customCommand.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "customCommandArgument.h"

customCommand::customCommand(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _mode = cNew;
  _cmdid = -1;
  
  _arguments->addColumn(tr("Order"), _whsColumn, Qt::AlignCenter,true, "cmdarg_order");
  _arguments->addColumn(tr("Argument"),      -1, Qt::AlignLeft,  true, "cmdarg_arg");
  
  _module->clear();
  _module->addItem("Products");
  _module->addItem("Inventory");
  if (_metrics->value("Application") == "OpenMFG")
    _module->addItem("Schedule");
  _module->addItem("Purchase");
  _module->addItem("Manufacture");
  _module->addItem("CRM");
  _module->addItem("Sales");
  _module->addItem("Accounting");
  _module->addItem("System");
}

customCommand::~customCommand()
{
  // no need to delete child widgets, Qt does it all for us
}

void customCommand::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customCommand::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      setMode(cNew);
    else if("edit" == mode)
      setMode(cEdit);
    else if("view" == mode)
      setMode(cView);
  }

  // after setMode because populate() may change mode
  param = pParams.value("cmd_id", &valid);
  if(valid)
  {
    _cmdid = param.toInt();
    populate();
  }
  
  return NoError;
}

void customCommand::setMode(const int pmode)
{
  switch (pmode)
  {
    case cNew:
      q.prepare("SELECT nextval('cmd_cmd_id_seq') AS result;");
      q.exec();
      if(q.first())
        _cmdid = q.value("result").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      /* fallthru */

    case cEdit:
      _module->setEnabled(true);
      _title->setEnabled(true);
      _privname->setEnabled(true);
      _name->setEnabled(true);
      _description->setEnabled(true);
      _executable->setEnabled(true);
      _accept->show();
      _close->setText(tr("&Cancel"));

      if (pmode == cNew)
        _module->setFocus();
      else
        _accept->setFocus();
      break;

    case cView:
    default:
      _module->setEnabled(false);
      _title->setEnabled(false);
      _privname->setEnabled(false);
      _name->setEnabled(false);
      _description->setEnabled(false);
      _executable->setEnabled(false);
      _accept->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
      break;
  }
  _mode = pmode;
}

void customCommand::sSave()
{
  if(_title->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save"),
      tr("You must enter in a Menu Label for this command.") );
    return;
  }

  if(_executable->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save"),
      tr("You must enter in a program to execute.") );
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO cmd"
              "      (cmd_id, cmd_module, cmd_title, cmd_privname,"
              "       cmd_name, cmd_descrip, cmd_executable) "
              "VALUES(:cmd_id, :cmd_module, :cmd_title, :cmd_privname,"
              "       :cmd_name, :cmd_descrip, :cmd_executable);");
  else if(cEdit == _mode)
    q.prepare("UPDATE cmd"
              "   SET cmd_module=:cmd_module,"
              "       cmd_title=:cmd_title,"
              "       cmd_privname=:cmd_privname,"
              "       cmd_name=:cmd_name,"
              "       cmd_descrip=:cmd_descrip,"
              "       cmd_executable=:cmd_executable "
              " WHERE (cmd_id=:cmd_id);");

  q.bindValue(":cmd_id", _cmdid);
  q.bindValue(":cmd_module", _module->currentText());
  q.bindValue(":cmd_title", _title->text());
  q.bindValue(":cmd_privname", _privname->text());
  if(!_name->text().isEmpty())
    q.bindValue(":cmd_name", _name->text());
  q.bindValue(":cmd_descrip", _description->toPlainText());
  q.bindValue(":cmd_executable", _executable->text());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // make sure the custom privs get updated
  q.exec("SELECT updateCustomPrivs();");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void customCommand::reject()
{
  if(cNew == _mode)
  {
    QSqlQuery query;
    query.prepare("DELETE FROM cmdarg WHERE (cmdarg_cmd_id=:cmd_id);");
    query.bindValue(":cmd_id", _cmdid);
    query.exec();
    if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  XDialog::reject();
}

void customCommand::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cmd_id", _cmdid);

  customCommandArgument newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void customCommand::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmdarg_id", _arguments->id());

  customCommandArgument newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void customCommand::sDelete()
{
  q.prepare("DELETE FROM cmdarg WHERE (cmdarg_id=:cmdarg_id);");
  q.bindValue(":cmdarg_id", _arguments->id());
  if(q.exec())
    sFillList();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customCommand::populate()
{
  q.prepare("SELECT cmd.*, relname ~* 'pkgcmd' AS inPackage"
            "  FROM cmd, pg_class"
            " WHERE ((cmd.tableoid=pg_class.oid)"
            "   AND  (cmd_id=:cmd_id));");
  q.bindValue(":cmd_id", _cmdid);
  q.exec();
  if(q.first())
  {
    _module->setCurrentText(q.value("cmd_module").toString());
    _title->setText(q.value("cmd_title").toString());
    _oldPrivname = q.value("cmd_privname").toString();
    _privname->setText(q.value("cmd_privname").toString());
    _name->setText(q.value("cmd_name").toString());
    _executable->setText(q.value("cmd_executable").toString());
    _description->setText(q.value("cmd_descrip").toString());
    if (q.value("inPackage").toBool())
      setMode(cView);

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customCommand::sFillList()
{
  q.prepare("SELECT cmdarg_id, cmdarg_order, cmdarg_arg"
            "  FROM cmdarg"
            " WHERE (cmdarg_cmd_id=:cmd_id)"
            " ORDER BY cmdarg_order, cmdarg_id;");
  q.bindValue(":cmd_id", _cmdid);
  q.exec();
  _arguments->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
