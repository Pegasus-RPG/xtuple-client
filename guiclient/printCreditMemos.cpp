/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemos.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

printCreditMemos::printCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  if(!_privileges->check("PostARDocuments"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }
  
  _print->setFocus();
}

printCreditMemos::~printCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

void printCreditMemos::languageChange()
{
    retranslateUi(this);
}

void printCreditMemos::sPrint()
{
  XSqlQuery cmhead( "SELECT cmhead_id, cmhead_number, cmhead_posted,"
                    "       findCustomerForm(cmhead_cust_id, 'C') AS _reportname "
                    "FROM ( SELECT cmhead_id, cmhead_number, cmhead_cust_id, cmhead_posted "
                    "       FROM cmhead "
                    "       WHERE ( (NOT cmhead_hold)"
                    "         AND   (NOT COALESCE(cmhead_printed,false)) ) ) AS data "
                    "WHERE (checkCreditMemoSitePrivs(cmhead_id));" );
  if (cmhead.first())
  {
    QPrinter  printer(QPrinter::HighResolution);
    bool      setupPrinter  = TRUE;
    bool      userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }


    do
    {
      for (int i = 0; i < _creditMemoCopies->numCopies(); i++)
      {
        ParameterList params;
        params.append("cmhead_id", cmhead.value("cmhead_id").toInt());
        params.append("showcosts", (_creditMemoCopies->showCosts(i) ? "TRUE" : "FALSE"));
        params.append("watermark", _creditMemoCopies->watermark(i));

        orReport report(cmhead.value("_reportname").toString(), params);
        if (!report.isValid())
          QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
                                 tr("<p>The Credit Memo Form '%1' for Credit "
                                    "Memo #%2 cannot be found. This Credit "
                                    "Memo cannot be printed until Customer "
                                    "Form Assignments are updated to remove "
                                    "any references to this Credit Memo Form "
                                    "or this Credit Memo Form is created." )
                                 .arg(cmhead.value("_reportname").toString())
                                 .arg(cmhead.value("cmhead_number").toString()) );
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printCreditMemos::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }
        }
      }
      
      // if post after print was checked attempt to post it
      // if it hasn't already been posted
      if (_post->isChecked() && !cmhead.value("cmhead_posted").toInt())
      {
        
        q.exec("BEGIN;");
        //TO DO:  Replace this method with commit that doesn't require transaction
        //block that can lead to locking issues
        XSqlQuery rollback;
        rollback.prepare("ROLLBACK;");
      
        q.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");
        q.bindValue(":cmhead_id", cmhead.value("cmhead_id").toInt());
        q.exec();
        q.first();
        int result = q.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError( this, storedProcErrorLookup("postCreditMemo", result),
                __FILE__, __LINE__);
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          rollback.exec();
        }
        else
        {
          if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
          {
            rollback.exec();
            QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
          }
      
          q.exec("COMMIT;");
        }
      }
      emit finishedPrinting(cmhead.value("cmhead_id").toInt());
    }
    while (cmhead.next());
    orReport::endMultiPrint(&printer);

    if (QMessageBox::question(this, tr("Mark Credit Memos as Printed?"),
                              tr("<p>Did all of the Credit Memos print "
                                 "correctly?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      XSqlQuery().exec( "UPDATE cmhead "
                        "SET cmhead_printed=TRUE "
                        "WHERE (NOT COALESCE(cmhead_printed,false));" );

    omfgThis->sCreditMemosUpdated();
  }
  else
    QMessageBox::information( this, tr("No Credit Memos to Print"),
                              tr("There aren't any Credit Memos to print.") );

  accept();
}

