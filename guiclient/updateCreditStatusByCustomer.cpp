/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateCreditStatusByCustomer.h"

#include <qvariant.h>

/*
 *  Constructs a updateCreditStatusByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateCreditStatusByCustomer::updateCreditStatusByCustomer(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulate(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateCreditStatusByCustomer::~updateCreditStatusByCustomer()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateCreditStatusByCustomer::languageChange()
{
    retranslateUi(this);
}


void updateCreditStatusByCustomer::sUpdate()
{
  q.prepare( "UPDATE custinfo "
             "SET cust_creditstatus=:cust_creditstatus "
             "WHERE (cust_id=:cust_id);" );
  q.bindValue(":cust_id", _cust->id());

  if (_inGoodStanding->isChecked())
    q.bindValue(":cust_creditstatus", "G");
  else if (_onCreditWarning->isChecked())
    q.bindValue(":cust_creditstatus", "W");
  if (_onCreditHold->isChecked())
    q.bindValue(":cust_creditstatus", "H");

  q.exec();

  accept();
}


void updateCreditStatusByCustomer::sPopulate( int /*pCustid*/ )
{
  q.prepare( "SELECT cust_creditstatus "
             "FROM custinfo "
             "WHERE (cust_id=:cust_id);" );
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  if (q.first())
  {
    if (q.value("cust_creditstatus").toString() == "G")
    {
      _inGoodStanding->setChecked(TRUE);
      _currentStatus->setText(tr("In Good Standing"));
      _currentStatus->setPaletteForegroundColor(QColor("black"));
    }
    else if (q.value("cust_creditstatus").toString() == "W")
    {
      _onCreditWarning->setChecked(TRUE);
      _currentStatus->setText(tr("On Credit Warning"));
      _currentStatus->setPaletteForegroundColor(QColor("orange"));
    }
    else if (q.value("cust_creditstatus").toString() == "H")
    {
      _onCreditHold->setChecked(TRUE);
      _currentStatus->setText(tr("On Credit Hold"));
      _currentStatus->setPaletteForegroundColor(QColor("red"));
    }
  }
}
