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

#include "subaccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

subaccount::subaccount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _number->setMaxLength(_metrics->value("GLSubaccountSize").toInt());
  _cachedNumber = "";
}

subaccount::~subaccount()
{
  // no need to delete child widgets, Qt does it all for us
}

void subaccount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse subaccount::set(const ParameterList &pParams )
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("subaccnt_id", &valid);
  if (valid)
  {
    _subaccntid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _number->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      
      _close->setFocus();
    }
  }
  return NoError;
}

void subaccount::sSave()
{
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Sub Account"),
                            tr("You must enter a valid Number.") );
      return;
  }
  
  q.prepare("SELECT subaccnt_id"
            "  FROM subaccnt"
            " WHERE((subaccnt_id != :subaccnt_id)"
            "   AND (subaccnt_number=:subaccnt_number))");
  q.bindValue(":subaccnt_id", _subaccntid);
  q.bindValue(":subaccnt_number", _number->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Sub Account Number"),
      tr("A Sub Account Number already exists for the one specified.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('subaccnt_subaccnt_id_seq') AS subaccnt_id;");
    if (q.first())
      _subaccntid = q.value("subaccnt_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    q.prepare( "INSERT INTO subaccnt "
               "( subaccnt_id, subaccnt_number, subaccnt_descrip ) "
               "VALUES "
               "( :subaccnt_id, :subaccnt_number, :subaccnt_descrip );" );
  }
  else if (_mode == cEdit)
  {
    if (_number->text() != _cachedNumber &&
        QMessageBox::question(this, tr("Change All Accounts?"),
                              tr("<p>The old Subaccount Number %1 might be "
                                 "used by existing Accounts. Would you like to "
                                 "change all accounts that use it to Subaccount"
                                 " Number %2?<p>If you answer 'No' then change "
                                 "the Number back to %3 and Save again.")
                                .arg(_cachedNumber)
                                .arg(_number->text())
                                .arg(_cachedNumber),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    q.prepare( "UPDATE subaccnt "
               "SET subaccnt_number=:subaccnt_number,"
               "    subaccnt_descrip=:subaccnt_descrip "
               "WHERE (subaccnt_id=:subaccnt_id);" );
  }
  
  q.bindValue(":subaccnt_id", _subaccntid);
  q.bindValue(":subaccnt_number", _number->text());
  q.bindValue(":subaccnt_descrip", _descrip->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  done(_subaccntid);
}

void subaccount::populate()
{
  q.prepare( "SELECT * "
             "FROM subaccnt "
             "WHERE (subaccnt_id=:subaccnt_id);" );
  q.bindValue(":subaccnt_id", _subaccntid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("subaccnt_number").toString());
    _descrip->setText(q.value("subaccnt_descrip").toString());
    _cachedNumber = q.value("subaccnt_number").toString();
  }
}
