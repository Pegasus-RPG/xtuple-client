/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "deliverInvoice.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

deliverInvoice::deliverInvoice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_invoice, SIGNAL(newId(int)), this, SLOT(sHandleInvcheadid()));

  _captive = FALSE;

  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    _fromEmail->setText(q.value("usr_email"));
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

deliverInvoice::~deliverInvoice()
{
  // no need to delete child widgets, Qt does it all for us
}

void deliverInvoice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse deliverInvoice::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("invchead_id", &valid);
  if (valid)
  {
    _invoice->setId(param.toInt());
    _submit->setFocus();
  }

  return NoError;
}

void deliverInvoice::sSubmit()
{
  if (_email->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Queue Invoice for Delivery"),
                           tr("<pYou must enter a email address to which this "
                              "Invoice is to be delivered.") );
    _email->setFocus();
    return;
  }

  q.prepare( "SELECT invchead_invcnumber, findCustomerForm(invchead_cust_id, 'I') AS invoiceform "
             "FROM invchead "
             "WHERE (invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invoice->id());
  q.exec();
  if (q.first())
  {
    QString invoiceNumber = q.value("invchead_invcnumber").toString();
    QString reportName = q.value("invoiceform").toString();

    q.prepare( "SELECT submitReportToBatch( :reportname, :fromEmail, :emailAddress, :ccAddress, :subject,"
               "                            :emailBody, :fileName, CURRENT_TIMESTAMP, :emailHTML) AS batch_id;" );
    q.bindValue(":reportname", reportName);
    q.bindValue(":fromEmail", _fromEmail->text());
    q.bindValue(":emailAddress", _email->text());
    q.bindValue(":ccAddress", _cc->text());
    q.bindValue(":subject", _subject->text().replace("</docnumber>", invoiceNumber).replace("</doctype>", "Invc"));
    q.bindValue(":fileName", _fileName->text().replace("</docnumber>", invoiceNumber).replace("</doctype>", "Invc"));
    q.bindValue(":emailBody", _emailBody->toPlainText().replace("</docnumber>", invoiceNumber).replace("</doctype>", "Invc"));
    q.bindValue(":emailHTML", QVariant(_emailHTML->isChecked()));
    q.exec();
    if (q.first())
    {
      int batch_id = q.value("batch_id").toInt();

      q.prepare( "INSERT INTO batchparam "
                 "( batchparam_batch_id, batchparam_order,"
                 "  batchparam_name, batchparam_value ) "
                 "VALUES "
                 "( :batchparam_batch_id, :batchparam_order,"
                 "  :batchparam_name, :batchparam_value );" );

      q.bindValue(":batchparam_batch_id", batch_id);
      q.bindValue(":batchparam_order", 1);
      q.bindValue(":batchparam_name", "invchead_id");
      q.bindValue(":batchparam_value", _invoice->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }

      q.bindValue(":batchparam_batch_id", batch_id);
      q.bindValue(":batchparam_order", 2);
      q.bindValue(":batchparam_name", "title");
      q.bindValue(":batchparam_value", "Emailed Customer Copy");
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }

    if (_markPrinted->isChecked())
    {
      q.prepare( "UPDATE invchead "
                 "SET invchead_printed=TRUE "
                 "WHERE (invchead_id=:invchead_id);" );
      q.bindValue(":invchead_id", _invoice->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      omfgThis->sInvoicesUpdated(_invoice->id(), TRUE);
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _submit->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _email->clear();
    _cc->clear();
    _emailBody->clear();
    _subject->clear();
    _fileName->clear();
    _invoice->clear();
    _invoice->setFocus();
  }
}

void deliverInvoice::sHandleInvcheadid()
{
  q.prepare( "SELECT invchead_printed,"
             "       cust_emaildelivery, cust_ediemail, cust_ediemailbody, "
             "       cust_edisubject, cust_edifilename, cust_edicc, cust_ediemailhtml "
             "FROM invchead, custinfo "
             "WHERE ( (invchead_cust_id=cust_id)"
             " AND (invchead_id=:invchead_id) );");
  q.bindValue(":invchead_id", _invoice->id());
  q.exec();
  if (q.first())
  {
    if (q.value("cust_emaildelivery").toBool())
    {
      if (q.value("invchead_printed").toBool())
      {
        _markPrinted->setChecked(FALSE);
        _markPrinted->setEnabled(FALSE);
      }
      else
        _markPrinted->setEnabled(TRUE);

      _submit->setEnabled(TRUE);
      _email->setText(q.value("cust_ediemail").toString());
      _cc->setText(q.value("cust_edicc").toString());
      _emailBody->setPlainText(q.value("cust_ediemailbody").toString());
      _subject->setText(q.value("cust_edisubject").toString());
      _fileName->setText(q.value("cust_edifilename").toString());
      _emailHTML->setChecked(q.value("cust_ediemailhtml").toBool());
    }
    else
    {
      _markPrinted->setEnabled(TRUE);
      _submit->setEnabled(FALSE);
      _email->clear();
      _cc->clear();
      _emailBody->clear();
      _subject->clear();
      _fileName->clear();
      _emailHTML->setChecked(false);
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
