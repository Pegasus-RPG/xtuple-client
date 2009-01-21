/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postBillingSelections.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>

/*
 *  Constructs a postBillingSelections as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postBillingSelections::postBillingSelections(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _customerType->setType(ParameterGroup::CustomerType);
  _post->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postBillingSelections::~postBillingSelections()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postBillingSelections::languageChange()
{
  retranslateUi(this);
}

void postBillingSelections::sPost()
{
  QString sql("SELECT postBillingSelections(custtype_id, :consolidate) AS result"
              " FROM custtype" );
  if (_customerType->isSelected())
    sql += " WHERE (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " WHERE (custtype_code ~ :custtype_pattern)";
  q.prepare(sql);
  _customerType->bindValue(q);
  q.bindValue(":consolidate", _consolidate->isChecked());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() == -5)
      QMessageBox::critical( this, tr("Cannot Post one or more Invoices"),
                             tr( "The G/L Account Assignments for one or more of the Billing Selections that you are trying to post\n"
                                 "are not configured correctly.  Because of this, G/L Transactions cannot be posted for these.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post these Billing Selections." ) );
    else if (q.value("result").toInt() < 0)
      systemError( this, tr("A System Error occurred at postBillingSelections::%1, Error #%2.")
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );

    omfgThis->sInvoicesUpdated(-1, TRUE);
    omfgThis->sBillingSelectionUpdated(-1, TRUE);
    omfgThis->sSalesOrdersUpdated(-1);
  }
  else
    systemError( this, tr("A System Error occurred at postBillingSelections::%1.")
                       .arg(__LINE__) );

  accept();
}

