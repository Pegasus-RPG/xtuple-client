/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateCreditStatusByCustomer.h"

#include <QMessageBox>
#include <QVariant>

updateCreditStatusByCustomer::updateCreditStatusByCustomer(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulate(int)));
}

updateCreditStatusByCustomer::~updateCreditStatusByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateCreditStatusByCustomer::languageChange()
{
    retranslateUi(this);
}

void updateCreditStatusByCustomer::sUpdate()
{
  if (_privileges->check("UpdateCustomerCreditStatus"))
  {
    XSqlQuery updateUpdate;
    updateUpdate.prepare( "UPDATE custinfo "
               "SET cust_creditstatus=:cust_creditstatus "
               "WHERE (cust_id=:cust_id);" );
    updateUpdate.bindValue(":cust_id", _cust->id());

    if (_inGoodStanding->isChecked())
      updateUpdate.bindValue(":cust_creditstatus", "G");
    else if (_onCreditWarning->isChecked())
      updateUpdate.bindValue(":cust_creditstatus", "W");
    if (_onCreditHold->isChecked())
      updateUpdate.bindValue(":cust_creditstatus", "H");

    updateUpdate.exec();

    accept();
  }
  else
  {
    QMessageBox::critical( this, tr("Cannot Update Credit Status"),
                                 tr( "You don't have privileges to update customer credit status." ) );
  }
}


void updateCreditStatusByCustomer::sPopulate( int /*pCustid*/ )
{
  XSqlQuery updatePopulate;
  updatePopulate.prepare( "SELECT cust_creditstatus "
             "FROM custinfo "
             "WHERE (cust_id=:cust_id);" );
  updatePopulate.bindValue(":cust_id", _cust->id());
  updatePopulate.exec();
  if (updatePopulate.first())
  {
    QString stylesheet;
    if (updatePopulate.value("cust_creditstatus").toString() == "G")
    {
      _inGoodStanding->setChecked(TRUE);
      _currentStatus->setText(tr("In Good Standing"));
    }
    else if (updatePopulate.value("cust_creditstatus").toString() == "W")
    {
      _onCreditWarning->setChecked(TRUE);
      _currentStatus->setText(tr("On Credit Warning"));
      stylesheet = QString("* { color: %1; }").arg(namedColor("warning").name());
    }
    else if (updatePopulate.value("cust_creditstatus").toString() == "H")
    {
      _onCreditHold->setChecked(TRUE);
      _currentStatus->setText(tr("On Credit Hold"));
      stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    _currentStatus->setStyleSheet(stylesheet);
  }
}
