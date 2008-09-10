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

#include "company.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

#include <dbtools.h>

#include "login2.h"

#define DEBUG true

company::company(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_extDB,     SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extPort,   SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extServer, SIGNAL(editingFinished()), this, SLOT(sHandleTest()));
  connect(_extTest,           SIGNAL(clicked()), this, SLOT(sTest()));
  connect(_save,              SIGNAL(clicked()), this, SLOT(sSave()));

  _number->setMaxLength(_metrics->value("GLCompanySize").toInt());
  _cachedNumber = "";

  _extGroup->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));
  _extGroup->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));
}

company::~company()
{
  // no need to delete child widgets, Qt does it all for us
}

void company::languageChange()
{
  retranslateUi(this);
}

enum SetResponse company::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("company_id", &valid);
  if (valid)
  {
    _companyid = param.toInt();
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
      _extGroup->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      
      _close->setFocus();
    }
  }
  return NoError;
}

void company::sSave()
{
  if (_number->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Company"),
                            tr("You must enter a valid Number.") );
      return;
  }
  
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _extGroup->isChecked() && _extServer->text().isEmpty(),
      tr("<p>You must enter a Server if this is an external Company."),
      _extServer
    },
    { _extGroup->isChecked() && _extPort->value() == 0,
      tr("<p>You must enter a Port if this is an external Company."),
      _extPort
    },
    { _extGroup->isChecked() && _extDB->text().isEmpty(),
      tr("<p>You must enter a Database if this is an external Company."),
      _extDB
    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Company"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  q.prepare("SELECT company_id"
            "  FROM company"
            " WHERE((company_id != :company_id)"
            "   AND (company_number=:company_number))");
  q.bindValue(":company_id",       _companyid);
  q.bindValue(":company_number",   _number->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Company Number"),
      tr("A Company Number already exists for the one specified.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('company_company_id_seq') AS company_id;");
    if (q.first())
      _companyid = q.value("company_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    q.prepare( "INSERT INTO company "
               "( company_id, company_number, company_descrip,"
               "  company_external, company_server, company_port,"
               "  company_database) "
               "VALUES "
               "( :company_id, :company_number, :company_descrip,"
               "  :company_external, :company_server, :company_port, "
               "  :company_database);" );
  }
  else if (_mode == cEdit)
  {
    if (_number->text() != _cachedNumber &&
        QMessageBox::question(this, tr("Change All Accounts?"),
                          _extGroup->isChecked() ?
                              tr("<p>The old Company Number %1 might be used "
                                 "by existing Accounts, including Accounts in "
                                 "the %2 child database. Would you like to "
                                 "change all accounts in the current database "
                                 "that use it to Company Number %3?<p>If you "
                                 "answer 'No' then either Cancel or change the "
                                 "Number back to %4 and Save again. If you "
                                 "answer 'Yes' then change the Company Number "
                                 "in the child database as well.")
                                .arg(_cachedNumber)
                                .arg(_extDB->text())
                                .arg(_number->text())
                                .arg(_cachedNumber)
                            :
                              tr("<p>The old Company Number %1 might be used "
                                 "by existing Accounts. Would you like to "
                                 "change all accounts that use it to Company "
                                 "Number %2?<p>If you answer 'No' then either "
                                 "Cancel or change "
                                 "the Number back to %3 and Save again.")
                                .arg(_cachedNumber)
                                .arg(_number->text())
                                .arg(_cachedNumber),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    q.prepare( "UPDATE company "
               "SET company_number=:company_number,"
               "    company_descrip=:company_descrip,"
               "    company_external=:company_external,"
               "    company_server=:company_server,"
               "    company_port=:company_port,"
               "    company_database=:company_database "
               "WHERE (company_id=:company_id);" );
  }
  
  q.bindValue(":company_id",       _companyid);
  q.bindValue(":company_number",   _number->text());
  q.bindValue(":company_descrip",  _descrip->text());
  q.bindValue(":company_external", _extGroup->isChecked());
  q.bindValue(":company_server",   _extServer->text());
  q.bindValue(":company_port",     _extPort->cleanText());
  q.bindValue(":company_database", _extDB->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  done(_companyid);
}

void company::populate()
{
  q.prepare( "SELECT * "
             "FROM company "
             "WHERE (company_id=:company_id);" );
  q.bindValue(":company_id", _companyid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("company_number").toString());
    _descrip->setText(q.value("company_descrip").toString());
    _extGroup->setChecked(q.value("company_external").toBool());
    _extServer->setText(q.value("company_server").toString());
    _extPort->setValue(q.value("company_port").toInt());
    _extDB->setText(q.value("company_database").toString());

    _cachedNumber = q.value("company_number").toString();

    q.prepare("SELECT COUNT(*) AS result "
              "FROM accnt "
              "WHERE (accnt_company=:number);");
    q.bindValue(":number", _cachedNumber);
    q.exec();
    if (q.first())
      _extGroup->setEnabled(q.value("result").toInt() == 0);
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sHandleTest();
}

void company::sHandleTest()
{
  _extTest->setEnabled(! _extServer->text().isEmpty() &&
                       _extPort->value() != 0 &&
                       ! _extDB->text().isEmpty());
}

void company::sTest()
{
  if (DEBUG)
    qDebug("company::sTest()");

  QString dbURL;
  QString protocol;
  QString host = _extServer->text();
  QString db   = _extDB->text();
  QString port = _extPort->cleanText();

  buildDatabaseURL(dbURL, protocol, host, db, port);
  if (DEBUG)
    qDebug("company::sTest() dbURL before login2 = %s", qPrintable(dbURL));

  ParameterList params;
  params.append("databaseURL", dbURL);
  params.append("multipleConnections");

  login2 newdlg(this, "testLogin", false);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Rejected)
    return;

  dbURL = newdlg._databaseURL;
  if (DEBUG)
    qDebug("company::sTest() dbURL after login2 = %s", qPrintable(dbURL));
  parseDatabaseURL(dbURL, protocol, host, db, port);

  QSqlDatabase testDB = QSqlDatabase::addDatabase("QPSQL7", db);
  testDB.setHostName(host);
  testDB.setDatabaseName(db);
  testDB.setUserName(newdlg.username());
  testDB.setPassword(newdlg.password());
  testDB.setPort(port.toInt());
  if (testDB.open())
  {
    if (DEBUG)
      qDebug("company::sTest() opened testDB!");

    XSqlQuery rmq(testDB);
    rmq.prepare("SELECT fetchMetricText('OpenMFGServerVersion') AS result;");
    rmq.exec();
    if (rmq.first())
    {
      if (rmq.value("result").toString() != _metrics->value("OpenMFGServerVersion"))
      {
        QMessageBox::warning(this, tr("Versions Incompatible"),
                             tr("<p>The version of the child database is not "
                                "the same as the version of the parent "
                                "database (%1 vs. %2). The data cannot safely "
                                "be synchronized.")
                             .arg(rmq.value("result").toString())
                             .arg(_metrics->value("OpenMFGServerVersion")));
        return;
      }
    }
    else if (rmq.lastError().type() != QSqlError::None)
    {
      systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    rmq.exec("SELECT * FROM curr_symbol WHERE curr_base;");
    q.exec("SELECT * FROM curr_symbol WHERE curr_base;");
    if (q.first() && rmq.first())
    {
      if (rmq.value("curr_name").toString() != q.value("curr_name").toString() &&
          rmq.value("curr_symbol").toString() != q.value("curr_symbol").toString() &&
          rmq.value("curr_abbr").toString() != q.value("curr_abbr").toString())
      {
        QMessageBox::warning(this, tr("Currencies Incompatible"),
                             tr("<p>The currency of the child database does "
                                "not match the currency of the parent database "
                                "(%1 %2 %3 vs. %4 %5 %6). The data cannot "
                                "safely be synchronized.")
                             .arg(rmq.value("curr_name").toString())
                             .arg(rmq.value("curr_symbol").toString())
                             .arg(rmq.value("curr_abbr").toString())
                             .arg(q.value("curr_name").toString())
                             .arg(q.value("curr_symbol").toString())
                             .arg(q.value("curr_abbr").toString()));
        return;
      }
    }
    else if (rmq.lastError().type() != QSqlError::None)
    {
      systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (!rmq.first())
    {
      QMessageBox::warning(this, tr("No Base Currency"),
                           tr("<p>The child database does not appear to have "
                              "a base currency defined. The data cannot safely "
                              "be synchronized."));
      return;
    }
    else if (!q.first())
    {
      QMessageBox::warning(this, tr("No Base Currency"),
                           tr("<p>The parent database does not appear to have "
                              "a base currency defined. The data cannot safely "
                              "be synchronized."));
      return;
    }

    rmq.prepare("SELECT * FROM company WHERE (company_number=:number);");
    rmq.bindValue(":number", _number->text());
    rmq.exec();
    if (rmq.first())
      ; // nothing to do
    else if (rmq.lastError().type() != QSqlError::None)
    {
      systemError(this, rmq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::warning(this, tr("No Corresponding Company"),
                           tr("<p>The child database does not appear to have "
                              "a Company %1 defined. The data cannot safely "
                              "be synchronized.").arg(_number->text()));
      return;
    }
  }
  else
  {
    QMessageBox::warning(this, tr("Could Not Connect"),
                         tr("<p>Could not connect to the child database "
                            "with these connection parameters."));
    return;
  }

}
