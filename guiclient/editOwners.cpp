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

#include "editOwners.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

editOwners::editOwners(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_query,	SIGNAL(clicked()),	this,	SLOT(sFillList()));
  connect(_modify,  	SIGNAL(clicked()),	this,	SLOT(sModify()));
  connect(_modifyAll,   SIGNAL(clicked()),      this,   SLOT(sModifyAll()));
  connect(_list, 	SIGNAL(itemClicked(QTreeWidgetItem*, int)),	this, 	SLOT(sItemClicked()));

  _list->addColumn(tr("Type"),    _statusColumn,  Qt::AlignCenter, true, "type_name");
  _list->addColumn(tr("Name"),              100,  Qt::AlignLeft,   true, "name");
  _list->addColumn(tr("Description"),        -1,  Qt::AlignLeft,   true, "description");
  _list->addColumn(tr("Owner"),     _userColumn,  Qt::AlignLeft,   true, "owner_username");

  _modify->setEnabled(false);
  _modifyAll->setEnabled(false);

  _first = true;

  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _owner->setId(q.value("usr_id").toInt());
  }  
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
}

void editOwners::sFillList()
{

  if(_todo->isChecked())
  {
    _queryString += "SELECT todoitem_id AS id, "
                    "       todoitem_owner_username AS owner_username, "
                    "       todoitem_name AS name, "
                    "       todoitem_description AS description, "
                    "       'To Do' AS type_name, "
                    "       'todoitem' AS table "
                    "FROM todoitem "
                    "WHERE todoitem_owner_username = :owner ";
    _first = false;
  }
  if(_project->isChecked())
  {
    if(!_first) _queryString += "UNION ALL ";
    _queryString += "SELECT prj_id AS id, "
                    "       prj_owner_username AS owner_username, "
                    "       prj_name AS name, "
                    "       prj_descrip AS description, "
                    "       'Project' AS type_name, "
                    "       'prj' AS table "
                    "FROM prj "
                    "WHERE prj_owner_username = :owner ";
    _first = false;
  }
  if(_contact->isChecked())
  {
    if(!_first) _queryString += "UNION ALL ";
    _queryString += "SELECT cntct_id AS id, "
                    "       cntct_owner_username AS owner_username, "
                    "       cntct_first_name || ' ' || cntct_last_name AS name, "
                    "       '' AS description, "
                    "       'Contact' AS type_name, "
                    "       'cntct' AS table "
                    "FROM cntct "
                    "WHERE cntct_owner_username = :owner ";
    _first = false;
  }
  if(_incident->isChecked())
  {
    if(!_first) _queryString += "UNION ALL ";
    _queryString += "SELECT incdt_id AS id, "
                    "       incdt_owner_username AS owner_username, "
                    "       incdt_summary AS name, "
                    "       incdt_descrip AS description, "
                    "       'Incident' AS type_name, "
                    "       'incdt' AS table "
                    "FROM incdt "
                    "WHERE incdt_owner_username = :owner ";
    _first = false;
  }
  if(_account->isChecked())
  {
    if(!_first) _queryString += "UNION ALL ";
    _queryString += "SELECT crmacct_id AS id, "
                    "       crmacct_owner_username AS owner_username, "
                    "       crmacct_name AS name, "
                    "       crmacct_notes AS description, "
                    "       'Account' AS type_name, "
                    "       'crmacct' AS table "
                    "FROM crmacct "
                    "WHERE crmacct_owner_username = :owner ";
    _first = false;
  }
  if(_oppourtunity->isChecked())
  {
    if(!_first) _queryString += "UNION ALL ";
    _queryString += "SELECT ophead_id AS id, "
                    "       ophead_owner_username AS owner_username, "
                    "       ophead_name AS name, "
                    "       ophead_notes AS description, "
                    "       'Opportunity' AS type_name, "
                    "       'ophead' AS table "
                    "FROM ophead "
                    "WHERE ophead_owner_username = :owner ";
    _first = false;
  }

  if(_queryString == "")
    _list->clear();

  q.prepare(_queryString);
  q.bindValue(":owner", _owner->username());
  q.exec();
  if(q.first())
  {
    _list->populate(q);
    _modifyAll->setEnabled(true);
  }
  else
    _list->clear();

  _first = true;
  _queryString = "";
}

void editOwners::sClose()
{
  close();
}

void editOwners::sModify()
{
  QString table;

  if(!_newOwner->isValid())
  {
    QMessageBox::critical( this, tr("No New Owner"),
      tr("A new owner must be selected before you can continue."));
    _newOwner->setFocus();
    return;
  }
  int ret = QMessageBox::warning(this, tr("Confirm Ownership Modification"),
                   tr("Are you sure that you want to change\n"
                      "the new owner to '"+_newOwner->username()+"' for the\n"
                      "selected records?"),
                   QMessageBox::Yes | QMessageBox::No,
                   QMessageBox::Yes);

  if(ret == QMessageBox::Yes)
  {
  if (modifyOne(_list->currentItem()))
    sFillList();
  }
}

void editOwners::sModifyAll()
{
  if(!_newOwner->isValid())
  {
    QMessageBox::critical( this, tr("No New Owner"),
      tr("A new owner must be selected before you can continue."));
    _newOwner->setFocus();
    return;
  }

  int ret = QMessageBox::warning(this, tr("Confirm Ownership Modification"),
                   tr("Are you sure that you want to change\n"
                      "the new owner to '"+_newOwner->username()+"' for the\n"
                      "selected records?"),
                   QMessageBox::Yes | QMessageBox::No,
                   QMessageBox::Yes);
  
  if(ret == QMessageBox::Yes)
  {

    QList<QTreeWidgetItem*> all = _list->findItems("", Qt::MatchContains);

    for (int i = 0; i < all.size(); i++)
    {
      XTreeWidgetItem *currentItem = static_cast<XTreeWidgetItem*>(all[i]);
      if (currentItem->rawValue("type_name").toString() != "")
        modifyOne(currentItem);
    }
    sFillList();
  }
}

bool editOwners::modifyOne(XTreeWidgetItem * currentItem)
{
  QString table;

  if(currentItem->rawValue("type_name").toString() == "To Do") table = "todoitem";
  if(currentItem->rawValue("type_name").toString() == "Project") table = "prj";
  if(currentItem->rawValue("type_name").toString() == "Contact") table = "cntct";
  if(currentItem->rawValue("type_name").toString() == "Incident") table = "incdt";
  if(currentItem->rawValue("type_name").toString() == "Account") table = "crmacct";
  if(currentItem->rawValue("type_name").toString() == "Opportunity") table = "ophead";

  q.prepare("UPDATE "+table+" "
            "SET "+table+"_owner_username = :new_owner_username "
            "WHERE "+table+"_id = :id ");
  q.bindValue(":new_owner_username", _newOwner->username());
  q.bindValue(":id", currentItem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  return true;
}

void editOwners::sItemClicked()
{
  _modify->setEnabled(true);
}

