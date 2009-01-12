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

#include "profitCenter.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

profitCenter::profitCenter(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _number->setMaxLength(_metrics->value("GLProfitSize").toInt());
  _cachedNumber = "";
}

profitCenter::~profitCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void profitCenter::languageChange()
{
  retranslateUi(this);
}

enum SetResponse profitCenter::set(const ParameterList &pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("prftcntr_id", &valid);
  if (valid)
  {
    _prftcntrid = param.toInt();
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

void profitCenter::sSave()
{
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Profit Center"),
                            tr("You must enter a valid Number.") );
      return;
  }
  
  q.prepare("SELECT prftcntr_id"
            "  FROM prftcntr"
            " WHERE((prftcntr_id != :prftcntr_id)"
            "   AND (prftcntr_number=:prftcntr_number))");
  q.bindValue(":prftcntr_id", _prftcntrid);
  q.bindValue(":prftcntr_number", _number->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Profit Center Number"),
      tr("A Profit Center Number already exists for the one specified.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('prftcntr_prftcntr_id_seq') AS prftcntr_id;");
    if (q.first())
      _prftcntrid = q.value("prftcntr_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO prftcntr "
               "( prftcntr_id, prftcntr_number, prftcntr_descrip ) "
               "VALUES "
               "( :prftcntr_id, :prftcntr_number, :prftcntr_descrip );" );
  }
  else if (_mode == cEdit)
  {
    if (_number->text() != _cachedNumber &&
        QMessageBox::question(this, tr("Change All Accounts?"),
                              tr("<p>The old Profit Center Number might be "
                                 "used by existing Accounts. Would you like to "
                                 "change all accounts that use it to Profit "
                                 "Center Number %1?<p>If you answer 'No' then "
                                 "change the Number back to %2 and Save again.")
                                .arg(_number->text())
                                .arg(_cachedNumber),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    q.prepare( "UPDATE prftcntr "
               "SET prftcntr_number=:prftcntr_number,"
               "    prftcntr_descrip=:prftcntr_descrip "
               "WHERE (prftcntr_id=:prftcntr_id);" );
  }

  q.bindValue(":prftcntr_id", _prftcntrid);
  q.bindValue(":prftcntr_number", _number->text());
  q.bindValue(":prftcntr_descrip", _descrip->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "UPDATE accnt "
               "SET accnt_profit=:prftcntr_number "
               "WHERE (accnt_profit=:old_prftcntr_number);" );
  }

  q.bindValue(":prftcntr_number", _number->text());
  q.bindValue(":old_prftcntr_number", _cachedNumber);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_prftcntrid);
}

void profitCenter::populate()
{
  q.prepare( "SELECT * "
             "FROM prftcntr "
             "WHERE (prftcntr_id=:prftcntr_id);" );
  q.bindValue(":prftcntr_id", _prftcntrid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("prftcntr_number").toString());
    _descrip->setText(q.value("prftcntr_descrip").toString());

    _cachedNumber = q.value("prftcntr_number").toString();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
