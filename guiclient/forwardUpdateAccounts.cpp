/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "forwardUpdateAccounts.h"

#include <QVariant>
#include <QMessageBox>
#include "errorReporter.h"
#include "guiclient.h"

forwardUpdateAccounts::forwardUpdateAccounts(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(accept()));
  connect(_typeSelected, SIGNAL(toggled(bool)), _type, SLOT(setEnabled(bool)));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
}

forwardUpdateAccounts::~forwardUpdateAccounts()
{
  // no need to delete child widgets, Qt does it all for us
}

void forwardUpdateAccounts::languageChange()
{
  retranslateUi(this);
}

void forwardUpdateAccounts::sUpdate()
{
  XSqlQuery forwardUpdate;
  if(_accntSelected->isChecked() && _accnt->id() == -1)
  {
    QMessageBox::warning(this, tr("No Account Selected"),
      tr("You have choosen to use an account but no account was specified.") );
    return;
  }
  
  if(_accntSelected->isChecked())
  {
    forwardUpdate.prepare("SELECT forwardUpdateAccount(:accnt_id) AS result;");
    forwardUpdate.bindValue(":accnt_id", _accnt->id());
  }
  else if(_typeSelected->isChecked())
  {
    forwardUpdate.prepare("SELECT forwardUpdateAccount(accnt_id) AS result"
              "  FROM accnt LEFT OUTER JOIN company ON (company_number=accnt_company)"
              " WHERE((accnt_type=:accnt_type)"
              "   AND (NOT COALESCE(company_external,false)));");
    if (_type->currentIndex() == 0)
      forwardUpdate.bindValue(":accnt_type", "A");
    else if (_type->currentIndex() == 1)
      forwardUpdate.bindValue(":accnt_type", "L");
    else if (_type->currentIndex() == 2)
      forwardUpdate.bindValue(":accnt_type", "E");
    else if (_type->currentIndex() == 3)
      forwardUpdate.bindValue(":accnt_type", "R");
    else if (_type->currentIndex() == 4)
      forwardUpdate.bindValue(":accnt_type", "Q");
  }
  else
    forwardUpdate.prepare("SELECT forwardUpdateAccount(accnt_id) AS result"
              "  FROM accnt LEFT OUTER JOIN company ON (company_number=accnt_company)"
              " WHERE (NOT COALESCE(company_external,false));");
  
  forwardUpdate.exec();

  if (!ErrorReporter::error(QtCriticalMsg, this, tr("Error updating accounts"),
                            forwardUpdate, __FILE__, __LINE__))
    QMessageBox::information(this, tr("Accounts Updated"),
                             tr("All accounts were successfully updated."));
}

