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

#include "postCreditMemos.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "distributeInventory.h"

postCreditMemos::postCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  _post->setFocus();
}

postCreditMemos::~postCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCreditMemos::languageChange()
{
  retranslateUi(this);
}

void postCreditMemos::sPost()
{
  q.exec( "SELECT cmhead_printed, COUNT(*) AS number "
          "FROM cmhead "
          "WHERE ( (NOT cmhead_posted) "
          "  AND   (checkCreditMemoSitePrivs(cmhead_id)) ) "
          "GROUP BY cmhead_printed;" );
  if (q.first())
  {
    int printed   = 0;
    int unprinted = 0;

    do
    {
      if (q.value("cmhead_printed").toBool())
        printed = q.value("number").toInt();
      else
        unprinted = q.value("number").toInt();
    }
    while (q.next());

    if ( ( (unprinted) && (!printed) ) && (!_postUnprinted->isChecked()) )
    {
      QMessageBox::warning( this, tr("No Credit Memos to Post"),
                            tr( "Although there are unposted Credit Memos, there are no unposted Credit Memos that have been printed.\n"
                                "You must manually print these Credit Memos or select 'Post Unprinted Credit Memos' before these Credit Memos\n"
                                "may be posted." ) );
      _postUnprinted->setFocus();
      return;
    }
  }
  else
  {
    QMessageBox::warning( this, tr("No Credit Memos to Post"),
                          tr("There are no Credit Memos, printed or not, to post.\n" ) );
    _close->setFocus();
    return;
  }

  q.exec("SELECT fetchJournalNumber('AR-CM') AS result");
  if (!q.first())
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  int journalNumber = q.value("result").toInt();
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT postCreditMemos(:postUnprinted, :journalNumber) AS result;");
  q.bindValue(":postUnprinted", QVariant(_postUnprinted->isChecked(), 0));
  q.bindValue(":journalNumber", journalNumber);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result == -5)
    {
      rollback.exec();
      QMessageBox::critical( this, tr("Cannot Post one or more Credit Memos"),
                             tr( "The G/L Account Assignments for one or more of the Credit Memos that you are trying to post are not\n"
                                 "configured correctly.  Because of this, G/L Transactions cannot be posted for these Credit Memos.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post these Credit Memos." ) );
      return;
    }
    else if (result < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at postCreditMemos::%1, Error #%2.")
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Post Credit Memos"), tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");

    if (_printJournal->isChecked())
    {
      ParameterList params;
      params.append("journalNumber", journalNumber);

      orReport report("CreditMemoJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at postCreditMemos::%1.")
                       .arg(__LINE__) );
    return;
  }

  omfgThis->sCreditMemosUpdated();

  accept();
}
