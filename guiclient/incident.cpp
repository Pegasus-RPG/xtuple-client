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

#include "incident.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QMenu>

#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "returnAuthorization.h"
#include "arOpenItem.h"
#include "deliverEmail.h"

/*
 *  Constructs a incident as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
incident::incident(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  _statusCodes << "N" << "F" << "C" << "A" << "R" << "L";
  setupUi(this);

  _incdtid = -1;
  _commentAdded = false;
  _updated = false;
  _ediprofileid = -1;
  _saved = false;
  _aropenid = -1;
  
  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  // signals and slots connections
  connect(_cancel,        SIGNAL(clicked()),        this,	SLOT(sCancel()));
  connect(_crmacct,       SIGNAL(newId(int)),       this,	SLOT(sCRMAcctChanged(int)));
  connect(_deleteTodoItem, SIGNAL(clicked()),       this,	SLOT(sDeleteTodoItem()));
  connect(_editTodoItem,  SIGNAL(clicked()),        this,	SLOT(sEditTodoItem()));
  connect(_item,          SIGNAL(newId(int)),     _lotserial,   SLOT(setItemId(int)));
  connect(_newTodoItem,   SIGNAL(clicked()),        this,	SLOT(sNewTodoItem()));
  connect(_save,          SIGNAL(clicked()),        this,	SLOT(sSave()));
  connect(_todoList,      SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  connect(_todoList,      SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)),
	    this,         SLOT(sPopulateTodoMenu(QMenu*)));
  connect(_todoList,      SIGNAL(valid(bool)),      this, SLOT(sHandleTodoPrivs()));
  connect(_viewTodoItem,  SIGNAL(clicked()),        this,	SLOT(sViewTodoItem()));
  //connect(_return,      SIGNAL(clicked()),        this, SLOT(sReturn()));
  connect(_viewAR,        SIGNAL(clicked()),        this, SLOT(sViewAR()));
  connect(_cntct,         SIGNAL(changed()),        this, SLOT(sContactChanged()));
  connect(_assignedTo,    SIGNAL(newId(int)),       this, SLOT(sAssigned()));
  connect(_comments,      SIGNAL(commentAdded()),   this, SLOT(sCommentAdded()));
  
  //Note changes for e-mail notification
  connect(_category,      SIGNAL(currentIndexChanged(int)), this, SLOT(sUpdateEdiProfile()));
  connect(_category,      SIGNAL(currentIndexChanged(int)), this, SLOT(sChanged()));
  connect(_description,   SIGNAL(textChanged(QString)), this, SLOT(sChanged()));
  connect(_crmacct,       SIGNAL(newId(int))          , this, SLOT(sChanged()));
  connect(_owner,         SIGNAL(newId(int))          , this, SLOT(sChanged()));
  connect(_assignedTo,    SIGNAL(newId(int))          , this, SLOT(sChanged()));
  connect(_severity,      SIGNAL(currentIndexChanged(int)), this, SLOT(sChanged()));
  connect(_status,        SIGNAL(currentIndexChanged(int)), this, SLOT(sChanged()));
  connect(_resolution,    SIGNAL(currentIndexChanged(int)), this, SLOT(sChanged()));
  connect(_priority,      SIGNAL(currentIndexChanged(int)), this, SLOT(sChanged()));
  connect(_resolution,    SIGNAL(currentIndexChanged(int)), this, SLOT(sChanged()));
  connect(_cntct,         SIGNAL(changed())           , this, SLOT(sChanged()));
  connect(_notes,         SIGNAL(textChanged())       , this, SLOT(sChanged()));
  connect(_item,          SIGNAL(newId(int))          , this, SLOT(sChanged()));
  connect(_lotserial,     SIGNAL(newId(int))          , this, SLOT(sChanged()));

  _severity->setType(XComboBox::IncidentSeverity);
  _priority->setType(XComboBox::IncidentPriority);
  _resolution->setType(XComboBox::IncidentResolution);
  _category->setType(XComboBox::IncidentCategory);
  _lotserial->setStrict(false);

  _incdthist->addColumn(tr("Username"),     _userColumn, Qt::AlignLeft, true, "incdthist_username");
  _incdthist->addColumn(tr("Date/Time"),_timeDateColumn, Qt::AlignLeft, true, "incdthist_timestamp");
  _incdthist->addColumn(tr("Description"),           -1, Qt::AlignLeft, true, "incdthist_descrip");

  _todoList->addColumn(tr("Priority"),      _userColumn, Qt::AlignRight, true, "incdtpriority_name");
  _todoList->addColumn(tr("User"),          _userColumn, Qt::AlignLeft,  true, "usr_username");
  _todoList->addColumn(tr("Name"),                  100, Qt::AlignLeft,  true, "todoitem_name");
  _todoList->addColumn(tr("Description"),            -1, Qt::AlignLeft,  true, "todoitem_description");
  _todoList->addColumn(tr("Status"),      _statusColumn, Qt::AlignLeft,  true, "todoitem_status");
  _todoList->addColumn(tr("Due Date"),      _dateColumn, Qt::AlignLeft,  true, "todoitem_due_date");

  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _myUsrId = q.value("usr_id").toInt();
    _owner->setId(_myUsrId);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }
  
  if (_metrics->boolean("LotSerialControl"))
  {
    connect(_item, SIGNAL(valid(bool)), _lotserial, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(newId(int)),  _lotserial, SLOT(setItemId(int)));
  }
  else
    _lotserial->setVisible(false);

  // because this causes a pop-behind situation we are hiding for now.
  //_return->hide();
  
  sUpdateEdiProfile();
}

/*
 *  Destroys the object and frees any allocated resources
 */
incident::~incident()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void incident::languageChange()
{
  retranslateUi(this);
}

enum SetResponse incident::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("incdt_id", &valid);
  if (valid)
  {
    _incdtid = param.toInt();
    populate();
    _lotserial->setItemId(_item->id());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.exec("SELECT nextval('incdt_incdt_id_seq') AS incdt_id, "
             "fetchIncidentNumber() AS number;");
      if(q.first())
      {
        _incdtid=q.value("incdt_id").toInt();
        _number->setText(q.value("number").toString());
        _comments->setId(_incdtid);
        _documents->setId(_incdtid);
        _alarms->setId(_incdtid);
      }
      else
      {
        QMessageBox::critical( omfgThis, tr("Database Error"),
                               tr( "A Database Error occured in incident::New.\n"
                                   "Contact your Systems Administrator." ));
        reject();
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _crmacct->setEnabled(true);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _crmacct->setEnabled(false);
      _cntct->setEnabled(false);
      _assignedTo->setEnabled(false);
      _category->setEnabled(false);
      _status->setEnabled(false);
      _resolution->setEnabled(false);
      _severity->setEnabled(false);
      _priority->setEnabled(false);
      _item->setReadOnly(true);
      _lotserial->setEnabled(false);
      _description->setEnabled(false);
      _notes->setEnabled(false);
      _deleteTodoItem->setEnabled(false);
      _editTodoItem->setEnabled(false);
      _newTodoItem->setEnabled(false);
      _owner->setEnabled(false);
  
      _save->hide();
      _cancel->setText(tr("&Close"));
      _cancel->setFocus();
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      _alarms->setReadOnly(true);
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _cntct->setId(param.toInt());
  }

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    q.prepare("SELECT aropen_doctype, aropen_docnumber, "
              "       CASE WHEN (aropen_doctype='C') THEN :creditMemo"
              "            WHEN (aropen_doctype='D') THEN :debitMemo"
              "            WHEN (aropen_doctype='I') THEN :invoice"
              "            WHEN (aropen_doctype='R') THEN :cashdeposit"
              "            ELSE '' END AS docType "
              "FROM aropen "
              "WHERE (aropen_id=:aropen_id);");
    q.bindValue(":aropen_id", _aropenid);
    q.bindValue(":creditMemo", tr("Credit Memo"));
    q.bindValue(":debitMemo", tr("Debit Memo"));
    q.bindValue(":invoice", tr("Invoice"));
    q.bindValue(":cashdeposit", tr("Customer Deposit"));    
    q.exec();
    if (q.first())
    { 
      if (_metrics->value("DefaultARIncidentStatus").toInt())
        _category->setId(_metrics->value("DefaultARIncidentStatus").toInt());
      _ardoctype=q.value("aropen_doctype").toString();
      _docType->setText(q.value("docType").toString());
      _docNumber->setText(q.value("aropen_docnumber").toString());
      _description->setText(QString("%1 #%2").arg(q.value("docType").toString()).arg(q.value("aropen_docnumber").toString()));
    }
  }

  sHandleTodoPrivs();
  return NoError;
}

int incident::saveContact(ContactCluster* pContact)
{
  pContact->setAccount(_crmacct->id());

  int answer = 2;	// Cancel
  int saveResult = pContact->save(AddressCluster::CHECK);

  if (-1 == saveResult)
    systemError(this, tr("There was an error saving a Contact (%1, %2).\n"
			 "Check the database server log for errors.")
		      .arg(pContact->label()).arg(saveResult),
		__FILE__, __LINE__);
  else if (-2 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple Contacts sharing this address (%1).\n"
		       "What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving %1").arg(pContact->label()),
		    tr("Would you like to update the existing Contact or create a new one?"),
		    tr("Create New"),
		    tr("Change Existing"),
		    tr("Cancel"),
		    2, 2);
  if (0 == answer)
    return pContact->save(AddressCluster::CHANGEONE);
  else if (1 == answer)
    return pContact->save(AddressCluster::CHANGEALL);

  return saveResult;
}

void incident::sCancel()
{
  if (cNew == _mode)
  {
    q.prepare("SELECT releaseNumber('IncidentNumber', :number) AS result;");
    q.bindValue(":number", _number->text());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("releaseNumber", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare("SELECT deleteIncident(:incdt_id) AS result;");
    q.bindValue(":incdt_id", _incdtid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteIncident", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void incident::sSave()
{
  if (! save(false)) // if error
    return;

  accept();
}

bool incident::save(bool partial)
{
  if (! partial)
  {
    if(_crmacct->id() == -1)
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify the Account that this incident is for.") );
      return false;
    }

    if(_cntct->id() <= 0 && _cntct->name().simplified().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify a Contact for this Incident.") );
      return false;
    }

    if(_description->text().trimmed().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify a description for this incident report.") );
      _description->setFocus();
      return false;
    }
    
    if (_status->currentIndex() == 3 && _assignedTo->username().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify an assignee when the status is assigned.") );
      _description->setFocus();
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

  if (! partial)
  {
    if (saveContact(_cntct) < 0)
    {
      rollback.exec();
      _cntct->setFocus();
      return false;
    }
  }

  if (cNew == _mode && !_saved)
    q.prepare("INSERT INTO incdt"
              "      (incdt_id, incdt_number, incdt_crmacct_id, incdt_cntct_id,"
              "       incdt_summary, incdt_descrip, incdt_item_id,"
              "       incdt_status, incdt_assigned_username,"
              "       incdt_incdtcat_id, incdt_incdtseverity_id,"
              "       incdt_incdtpriority_id, incdt_incdtresolution_id,"
              "       incdt_ls_id, incdt_aropen_id, incdt_owner_username) "
              "VALUES(:incdt_id, :incdt_number, :incdt_crmacct_id, :incdt_cntct_id,"
              "       :incdt_description, :incdt_notes, :incdt_item_id,"
              "       :incdt_status, :incdt_assigned_username,"
              "       :incdt_incdtcat_id, :incdt_incdtseverity_id,"
              "       :incdt_incdtpriority_id, :incdt_incdtresolution_id,"
              "       :incdt_ls_id, :incdt_aropen_id, :incdt_owner_username);" );
  else if (cEdit == _mode || _saved)
    q.prepare("UPDATE incdt"
              "   SET incdt_cntct_id=:incdt_cntct_id,"
              "       incdt_crmacct_id=:incdt_crmacct_id,"
              "       incdt_summary=:incdt_description,"
              "       incdt_descrip=:incdt_notes,"
              "       incdt_item_id=:incdt_item_id,"
              "       incdt_status=:incdt_status,"
              "       incdt_assigned_username=:incdt_assigned_username,"
              "       incdt_incdtcat_id=:incdt_incdtcat_id,"
              "       incdt_incdtpriority_id=:incdt_incdtpriority_id,"
              "       incdt_incdtseverity_id=:incdt_incdtseverity_id,"
              "       incdt_incdtresolution_id=:incdt_incdtresolution_id,"
              "       incdt_ls_id=:incdt_ls_id,"
              "       incdt_owner_username=:incdt_owner_username "
              " WHERE (incdt_id=:incdt_id); ");

  q.bindValue(":incdt_id", _incdtid);
  q.bindValue(":incdt_number", _number->text());
  if(_owner->isValid())
    q.bindValue(":incdt_owner_username", _owner->username());
  if (_crmacct->id() > 0)
    q.bindValue(":incdt_crmacct_id", _crmacct->id());
  if (_cntct->id() > 0)
    q.bindValue(":incdt_cntct_id", _cntct->id());
  q.bindValue(":incdt_description", _description->text().trimmed());
  q.bindValue(":incdt_notes", _notes->toPlainText().trimmed());
  if(-1 != _item->id())
    q.bindValue(":incdt_item_id", _item->id());
  if(_assignedTo->isValid())
    q.bindValue(":incdt_assigned_username", _assignedTo->username());
  q.bindValue(":incdt_status", _statusCodes.at(_status->currentIndex()));
  if(_category->isValid())
    q.bindValue(":incdt_incdtcat_id", _category->id());
  if(_severity->isValid())
    q.bindValue(":incdt_incdtseverity_id", _severity->id());
  if(_priority->isValid())
    q.bindValue(":incdt_incdtpriority_id", _priority->id());
  if(_resolution->isValid())
    q.bindValue(":incdt_incdtresolution_id", _resolution->id());
  if ((_item->id() != -1) && (_lotserial->id() != -1))
    q.bindValue(":incdt_ls_id", _lotserial->id());
  if (_aropenid > 0)
    q.bindValue(":incdt_aropen_id", _aropenid);
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
  
  if (! partial)
    emit prepareMail();

  _saved = true;
  return true;
}

void incident::sFillHistoryList()
{
  q.prepare("SELECT * "
            "  FROM incdthist"
            " WHERE (incdthist_incdt_id=:incdt_id)"
            " ORDER BY incdthist_timestamp; ");
  q.bindValue(":incdt_id", _incdtid);
  q.bindValue(":new", tr("New Incident"));
  q.bindValue(":status", tr("Status"));
  q.bindValue(":category", tr("Category"));
  q.bindValue(":severity", tr("Severity"));
  q.bindValue(":priority", tr("Priority"));
  q.bindValue(":resolution", tr("Resolution"));
  q.bindValue(":assignedto", tr("Assigned To"));
  q.bindValue(":notes", tr("Comment"));
  q.bindValue(":contact", tr("Contact"));
  q.exec();
  _incdthist->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incident::populate()
{ 
  q.prepare("SELECT incdt_number,"
            "       incdt_crmacct_id,"
            "       COALESCE(incdt_cntct_id,-1) AS incdt_cntct_id,"
            "       (cntct_first_name || ' ' || cntct_last_name) AS cntct_name,"
            "       incdt_summary, incdt_descrip,"
            "       incdt_item_id, incdt_ls_id,"
            "       incdt_status, incdt_assigned_username,"
            "       incdt_incdtcat_id, incdt_incdtseverity_id,"
            "       incdt_incdtpriority_id, incdt_incdtresolution_id, incdt_owner_username, "
			"       COALESCE(incdt_aropen_id, -1) AS docId,"
			"       COALESCE(aropen_docnumber, '') AS docNumber,"
            "       CASE WHEN (aropen_doctype='C') THEN :creditMemo"
            "            WHEN (aropen_doctype='D') THEN :debitMemo"
            "            WHEN (aropen_doctype='I') THEN :invoice"
            "            WHEN (aropen_doctype='R') THEN :cashdeposit"
            "            ELSE ''"
            "       END AS docType, "
            "       aropen_doctype "
            "FROM incdt LEFT OUTER JOIN cntct ON (incdt_cntct_id=cntct_id)"
			"           LEFT OUTER JOIN aropen ON (incdt_aropen_id=aropen_id) "
            "WHERE (incdt_id=:incdt_id); ");
  q.bindValue(":incdt_id", _incdtid);
  q.bindValue(":creditMemo", tr("Credit Memo"));
  q.bindValue(":debitMemo", tr("Debit Memo"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":cashdeposit", tr("Customer Deposit"));
  q.exec();
  if(q.first())
  {
    _crmacct->setId(q.value("incdt_crmacct_id").toInt());
    _cntct->setId(q.value("incdt_cntct_id").toInt());
    _owner->setUsername(q.value("incdt_owner_username").toString());
    _number->setText(q.value("incdt_number").toString());
    _assignedTo->setUsername(q.value("incdt_assigned_username").toString());
    _category->setNull();
    if(!q.value("incdt_incdtcat_id").toString().isEmpty())
      _category->setId(q.value("incdt_incdtcat_id").toInt());
    _status->setCurrentIndex(_statusCodes.indexOf(q.value("incdt_status").toString()));
    _statusCache=_status->currentIndex();
    _severity->setNull();
    if(!q.value("incdt_incdtseverity_id").toString().isEmpty())
      _severity->setId(q.value("incdt_incdtseverity_id").toInt());
    _priority->setNull();
    if(!q.value("incdt_incdtpriority_id").toString().isEmpty())
      _priority->setId(q.value("incdt_incdtpriority_id").toInt());
    _resolution->setNull();
    if(!q.value("incdt_incdtresolution_id").toString().isEmpty())
      _resolution->setId(q.value("incdt_incdtresolution_id").toInt());
    if(!q.value("incdt_item_id").toString().isEmpty())
      _item->setId(q.value("incdt_item_id").toInt());
    else
      _item->setId(-1);
    if(!q.value("incdt_ls_id").toString().isEmpty())
      _lotserial->setId(q.value("incdt_ls_id").toInt());
    else
      _lotserial->setId(-1);
    _description->setText(q.value("incdt_summary").toString());
    _notes->setText(q.value("incdt_descrip").toString());

    _comments->setId(_incdtid);
    _documents->setId(_incdtid);
    _alarms->setId(_incdtid);
	
    _docType->setText(q.value("docType").toString());
    _docNumber->setText(q.value("docNumber").toString());
    _aropenid = q.value("docId").toInt();
    _ardoctype = q.value("aropen_doctype").toString();
    if (_aropenid > 0)
      _viewAR->setEnabled(true);

    sFillHistoryList();
    sFillTodoList();
    _updated=false;
  }
}

void incident::sCRMAcctChanged(const int newid)
{
  _cntct->setSearchAcct(newid);
}

void incident::sNewTodoItem()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("incdt_id", _incdtid);
  params.append("priority_id", _priority->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillTodoList();
}

void incident::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillTodoList();
}

void incident::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incident::sDeleteTodoItem()
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

void incident::sFillTodoList()
{
  q.prepare("SELECT todoitem_id, todoitem_usr_id, *, "
	    "       firstLine(todoitem_notes) AS todoitem_notes, "
            "       CASE WHEN (todoitem_status != 'C' AND"
            "                  todoitem_due_date < CURRENT_DATE) THEN 'expired'"
            "            WHEN (todoitem_status != 'C' AND"
            "                  todoitem_due_date > CURRENT_DATE) THEN 'future'"
            "       END AS todoitem_due_date_qtforegroundrole "
	    "FROM usr, todoitem "
      "LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
	    "WHERE ( (todoitem_incdt_id=:incdt_id) "
	    "  AND   (todoitem_usr_id=usr_id)"
	    "  AND   (todoitem_active) ) "
	    "ORDER BY todoitem_due_date, usr_username;");

  q.bindValue(":incdt_id", _incdtid);
  q.exec();
  _todoList->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incident::sPopulateTodoMenu(QMenu *pMenu)
{
  int menuItem;

  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (_myUsrId == _todoList->altId() && _privileges->check("MaintainPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("MaintainOtherTodoLists")) );

  bool viewPriv =
      (_myUsrId == _todoList->altId() && _privileges->check("ViewPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("ViewOtherTodoLists"));

  menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNewTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, newPriv);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sViewTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, viewPriv);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDeleteTodoItem()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);
}

void incident::sHandleTodoPrivs()
{
  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalTodoList") ||
       _privileges->check("MaintainOtherTodoLists") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (_myUsrId == _todoList->altId() && _privileges->check("MaintainPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("MaintainOtherTodoLists")) );

  bool viewPriv =
      (_myUsrId == _todoList->altId() && _privileges->check("ViewPersonalTodoList")) ||
      (_myUsrId != _todoList->altId() && _privileges->check("ViewOtherTodoLists"));

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

void incident::sReturn()
{
  if (! save(true))
    return;

  ParameterList params;
  q.prepare("SELECT rahead_id FROM rahead WHERE rahead_incdt_id=:incdt_id");
  q.bindValue(":incdt_id", _incdtid);
  q.exec();
  if(q.first())
  {
    params.append("mode", "edit");
    params.append("rahead_id", q.value("rahead_id").toInt());
  }
  else
  {
    params.append("mode", "new");
    params.append("incdt_id", _incdtid);
  }

  returnAuthorization * newdlg = new returnAuthorization();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    QMessageBox::critical(this, tr("Could Not Open Window"),
                          tr("The new Return Authorization could not be created"));
  
}


void incident::sViewAR()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropenid);

  arOpenItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incident::sContactChanged()
{
  XSqlQuery acctq;
  acctq.prepare("SELECT cntct_crmacct_id "
                "FROM cntct "
                "WHERE cntct_id = :contact_id");
  acctq.bindValue(":contact_id", _cntct->id());
  acctq.exec();
  if(acctq.first())
    _crmacct->setId(acctq.value("cntct_crmacct_id").toInt());
  else if (acctq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, acctq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incident::sPrepareMail()
{
  //Users can use scripting to disconnect the signal that calls this and implement their
  //own logic to build message tokens if they like
  ParameterList params;
  ParameterList rptParams;
  QString ownerEmail;
  QString assignedEmail;
  QString docbody;
  QString reason;
  
  //See if we need to take action
  if ( !_metrics->boolean("CRMIncidentEmailCreated") &&
       !_metrics->boolean("CRMIncidentEmailAssigned") &&
       !_metrics->boolean("CRMIncidentEmailStatus") &&
       !_metrics->boolean("CRMIncidentEmailUpdated") &&
       !_metrics->boolean("CRMIncidentEmailComments") ) 
    return;
    
  //Determine reason, if none exit
  if (_statusCache != 3 && 
      _status->currentIndex() == 3 &&
      _metrics->boolean("CRMIncidentEmailAssigned") );
    reason = tr("The following incident has been ASSIGNED.");
  if (cNew == _mode)
    if (_metrics->boolean("CRMIncidentEmailCreated"))
      reason = tr("The following incident has been CREATED.");
    else
      return;
  else if (_statusCache != _status->currentIndex() && 
      _metrics->boolean("CRMIncidentEmailStatus") )
    reason = tr("The status of the following incident has been changed to %1.").arg(_status->currentText().upper());
  else if (_updated && _metrics->boolean("CRMIncidentEmailUpdated"))
    reason = tr("The following incident has been UPDATED.");
  else if (_commentAdded && _metrics->boolean("CRMIncidentEmailUpdated"))
    reason = tr("A new COMMENT has been added to the following incident.");
  else
    return;
  
  //Fetch email
  q.prepare("SELECT usr_email FROM usr WHERE (usr_username=:user);");
  q.bindValue(":user", _owner->username());
  q.exec();
  if (q.first())
    ownerEmail=q.value("usr_email").toString();
    
  q.prepare("SELECT usr_email FROM usr WHERE (usr_username=:user);");
  q.bindValue(":user", _assignedTo->username());
  q.exec();
  if (q.first())
    assignedEmail=q.value("usr_email").toString();
    
  //Build body
  docbody= QString("%1\n\n"
                   "Incident Number: %2\n"
                   "Description: %3\n"
                   "=====================================================\n"
                   "Owner: %4\n"
                   "Assigned To: %5\n" 
                   "Contact: %6\n"
                   "=====================================================\n"
                   "Category: %7\n"
                   "Status: %8\n"
                   "Severity: %9\n"
                   "Priority: %10\n"
                   "Resolution: %11\n")
           .arg(reason)
           .arg(_number->text())
           .arg(_description->text())
           .arg(_owner->username())
           .arg(_assignedTo->username())
           .arg(_cntct->name())
           .arg(_category->currentText())
           .arg(_status->currentText())
           .arg(_severity->currentText())
           .arg(_priority->currentText())
           .arg(_resolution->currentText());
           
  if (_item->id() != -1)
    docbody += tr("Item Number: %1\n").arg(_item->itemNumber());
    
  if (_lotserial->id() != -1)
    docbody += tr("Lot/Serial Number: %1\n").arg(_lotserial->number());
    
  if (!_docType->text().isEmpty())
  {
    docbody += tr("Receivable Type: %1\n"
                  "Document Number: %2\n")
           .arg(_docType->text())
           .arg(_docNumber->text());
  }
  
  docbody += tr(  "=====================================================\n"
                  "Notes:\n"
                  "%1\n\n")
           .arg(_notes->text());
           
  //Build history detail if applicable
  docbody += tr("=====================================================\n\n"
               "Incident History\n"
               "Date            Username  Text\n"
               "=====================================================\n");
               
  q.prepare("SELECT incdthist_timestamp, incdthist_username, incdthist_descrip "
            "FROM incdthist "
            "WHERE ( incdthist_incdt_id=:incdt_id);");
  q.bindValue(":incdt_id", _incdtid);
  q.exec();
  while (q.next())
  {
    docbody += q.value("incdthist_timestamp").toString();
    docbody += "  ";
    docbody += q.value("incdthist_username").toString().left(10).leftJustified(10,' ');
    docbody += q.value("incdthist_descrip").toString();
    docbody += "\n";
  }
  
  docbody += "=====================================================\n";
  
  params.append("description" , _description->text());
  params.append("docid"       , _incdtid);
  params.append("docbody"     , docbody);
  params.append("docnumber"   , _number->text());
  params.append("doctype"     , "INCDT");
  params.append("email1"      , ownerEmail);
  params.append("email2"      , assignedEmail);
  params.append("email3"      , _cntct->emailAddress());
  params.append("preview"     , _metrics->boolean("CRMIncidentEmailPreview"));
  
  if (_ardoctype == "I")
  {
    if (QMessageBox::question(this, tr("Attach Invoice"),
                              tr("<p>Would you like to attach a copy of the invoice "
                              "to the email to be sent?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes)
    {
      q.prepare( "SELECT invchead_id, findCustomerForm(invchead_cust_id, 'I') AS invoiceform "
             "FROM invchead "
             "WHERE (invchead_invcnumber=:invcnumber);" );
      q.bindValue(":invcnumber", _docNumber->text());
      q.exec();
      if (q.first())
      {
        params.append("reportName", q.value("invoiceform").toString());
        params.append("fileName", tr("Invoice-%1").arg(_docNumber->text()));
        rptParams.append("invchead_id", q.value("invchead_id").toInt());
      }
    }
  }
  sSendMail(params, rptParams);
}

void incident::sSendMail(ParameterList & params, ParameterList & rptParams)
{
  deliverEmail::profileEmail(this, _ediprofileid, params, rptParams);
}

void incident::sAssigned()
{
  if (_status->currentIndex() < 3 && !_assignedTo->username().isEmpty())
    _status->setCurrentIndex(3);
}

void incident::sUpdateEdiProfile()
{
  disconnect(this,           SIGNAL(prepareMail()),    this, SLOT(sPrepareMail()));
  if (_category->id() != -1)
  {
    XSqlQuery edi;
    edi.prepare("SELECT COALESCE(incdtcat_ediprofile_id,fetchMetricValue('CRMIncidentEmailProfile')) AS ediprofile_id "
              "FROM incdtcat "
              "WHERE (incdtcat_id=:incdtcat_id);");
    edi.bindValue(":incdtcat_id", _category->id());
    edi.exec();
    if (edi.first())
    {
      connect(this,           SIGNAL(prepareMail()),    this, SLOT(sPrepareMail()));
      _ediprofileid=edi.value("ediprofile_id").toInt();
    }
    else if (edi.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      _ediprofileid=-1;
      return;
    }
    else
      _ediprofileid=-1;
  }
  else if (_metrics->value("CRMIncidentEmailProfile").toInt())
  {
    connect(this,           SIGNAL(prepareMail()),    this, SLOT(sPrepareMail()));
    _ediprofileid=_metrics->value("CRMIncidentEmailProfile").toInt();
  }
  else
    _ediprofileid=-1;
}
