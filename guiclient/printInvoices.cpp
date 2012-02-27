/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

printInvoices::printInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _shipvia->populate( "SELECT MIN(invchead_id), invchead_shipvia "
                      "  FROM invchead "
                      " WHERE ( (NOT invchead_printed)"
                      "   AND   (NOT invchead_posted) )"
                      " GROUP BY invchead_shipvia"
                      " ORDER BY invchead_shipvia;" );

  if(!_privileges->check("PostMiscInvoices"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }

  _print->setFocus();
}

printInvoices::~printInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void printInvoices::languageChange()
{
  retranslateUi(this);
}

void printInvoices::sPrint()
{
  MetaSQLQuery mql = mqlLoad("invoices", "print");

  ParameterList params;
  if (_selectedShipvia->isChecked())
    params.append("shipvia", _shipvia->currentText());

  XSqlQuery invoices = mql.toQuery(params);
  if (invoices.first())
  {
    XSqlQuery local;
    QPrinter  printer(QPrinter::HighResolution);
    bool      setupPrinter = TRUE;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      int invoiceNumber = invoices.value("invchead_invcnumber").toInt();
      if (invoiceNumber == 0)
      {
        local.prepare("SELECT fetchInvcNumber() AS invoicenumber;");
        local.exec();
        if (local.first())
        {
          invoiceNumber = local.value("invoicenumber").toInt();
          local.prepare( "UPDATE invchead "
                         "SET invchead_invcnumber=text(:invoicenumber) "
                         "WHERE (invchead_id=:invchead_id);" );
          local.bindValue(":invoicenumber", invoiceNumber);
          local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
          local.exec();
        }
        else
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
      }

      XSqlQuery xrate;
      xrate.prepare("SELECT curr_rate "
                   "FROM curr_rate, invchead "
                   "WHERE ((curr_id=invchead_curr_id)"
                   "  AND  (invchead_id=:invchead_id)"
                   "  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
      // if SUM becomes dependent on curr_id then move XRATE before it in the loop

      XSqlQuery sum;
      sum.prepare("SELECT invoiceTotal(:invchead_id) AS subtotal;");

      message( tr("Printing Invoice #%1...")
               .arg(invoiceNumber) );

      for (int i = 0; i < _invoiceCopies->numCopies(); i++ )
      {
        ParameterList params;
        params.append("invchead_id", invoices.value("invchead_id").toInt());
        params.append("showcosts", (_invoiceCopies->showCosts(i) ? "TRUE" : "FALSE"));
        params.append("watermark", _invoiceCopies->watermark(i));

        orReport report(invoices.value("reportname").toString(), params);

        message( tr("Printing Invoice #%1...")
                 .arg(invoiceNumber) );

        if (!report.isValid())
        {
          QMessageBox::critical( this, tr("Cannot Find Invoice Form"),
                                 tr( "The Invoice Form '%1' cannot be found for Invoice #%2.\n"
                                     "This Invoice cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                     "references to this Invoice Form or this Invoice Form is created." )
                                 .arg(invoices.value("reportname").toString())
                                 .arg(invoiceNumber) );
          
          resetMessage();
        }
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printInvoices::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }
        }
      }
      emit finishedPrinting(invoices.value("invchead_id").toInt());

      if (_post->isChecked())
      {
        int invcheadId = invoices.value("invchead_id").toInt();
        sum.bindValue(":invchead_id", invcheadId);
        if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
        {
          if (QMessageBox::question(this, tr("Invoice Has Value 0"),
                                        tr("Invoice #%1 has a total value of 0.\n"
                                           "Would you like to post it anyway?")
                                          .arg(invoiceNumber),
                                        QMessageBox::Yes,
                                        QMessageBox::No | QMessageBox::Default)
                    == QMessageBox::No)
            continue;
        }
        else if (sum.lastError().type() != QSqlError::NoError)
        {
          systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
          continue;
        }
        else if (sum.value("subtotal").toDouble() != 0)
        {
          xrate.bindValue(":invchead_id", invcheadId);
          xrate.exec();
          if (xrate.lastError().type() != QSqlError::NoError)
          {
            systemError(this, tr("System Error posting Invoice #%1\n%2")
                                     .arg(invoiceNumber)
                                     .arg(xrate.lastError().databaseText()),
                             __FILE__, __LINE__);
            continue;
          }
          else if (!xrate.first() || xrate.value("curr_rate").isNull())
          {
            systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
                                     .arg(invoiceNumber));
            continue;
          }
        }

        message( tr("Posting Invoice #%1...")
                     .arg(invoiceNumber) );

        local.exec("BEGIN;");
        //TO DO:  Replace this method with commit that doesn't require transaction
        //block that can lead to locking issues
        XSqlQuery rollback;
        rollback.prepare("ROLLBACK;");

        local.prepare("SELECT postInvoice(:invchead_id) AS result;");
        local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
        local.exec();
        if (local.first())
        {
          int result = local.value("result").toInt();
          if (result < 0)
          {
            rollback.exec();
            systemError( this, storedProcErrorLookup("postInvoice", result),
                      __FILE__, __LINE__);
            return;
          }
          else
          {
            if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
            {
              rollback.exec();
              QMessageBox::information( this, tr("Post Invoice"), tr("Transaction Canceled") );
              return;
            }
          }
        }
        else if (local.lastError().type() != QSqlError::NoError)
        {
          systemError(this, local.lastError().databaseText(), __FILE__, __LINE__);
          rollback.exec();
          return;
        }

        local.exec("COMMIT;");
      }

    }
    while (invoices.next());
    orReport::endMultiPrint(&printer);

    resetMessage();

    if ( QMessageBox::information( this, tr("Mark Invoices as Printed?"),
                                     tr("Did all of the Invoices print correctly?"),
                                     tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
    {
      XSqlQuery invoices = mql.toQuery(params);
      if (invoices.first())
      {
        do
        {
          local.prepare( "UPDATE invchead "
                         "SET invchead_printed=TRUE "
                         "WHERE (invchead_id=:invchead_id);");
          local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
          local.exec();
        }
        while (invoices.next());
      }
    }

    omfgThis->sInvoicesUpdated(-1, TRUE);
  }
  else if (invoices.lastError().type() != QSqlError::NoError)
    systemError(this, invoices.lastError().databaseText(), __FILE__, __LINE__);
  else
    QMessageBox::information( this, tr("No Invoices to Print"),
                              tr("There aren't any Invoices to print.") );

  accept();
}

