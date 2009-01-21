/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "deliverSalesOrder.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a deliverSalesOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
deliverSalesOrder::deliverSalesOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sHandleSoheadid()));

  _captive = FALSE;

  _so->setType(cSoOpen | cSoClosed);
  _report->populate( "SELECT form_id, form_name "
                     "FROM form "
                     "WHERE (form_key='SO') "
                     "ORDER BY form_name;" );

  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    _fromEmail->setText(q.value("usr_email"));
//  ToDo
}

/*
 *  Destroys the object and frees any allocated resources
 */
deliverSalesOrder::~deliverSalesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void deliverSalesOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse deliverSalesOrder::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _so->setId(param.toInt());
    _submit->setFocus();
  }

  return NoError;
}

void deliverSalesOrder::sSubmit()
{
  if (_email->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Queue Sales Order for Delivery"),
                           tr("You must enter a email address to which this Sales Order is to be delivered.") );
    _email->setFocus();
    return;
  }

  q.prepare( "SELECT report_name "
             "  FROM report, form "
             " WHERE ( (form_id=:form_id) "
             "   AND (report_id=form_report_id) );");
  q.bindValue(":form_id", _report->id());
  q.exec();
  if (!q.first())
  {
    QMessageBox::warning( this, tr("Could not locate report"),
                          tr("Could not locate the report definition the form \"%1\"")
                          .arg(_report->currentText()) );
    return;
  }
  
  QString reportName = q.value("report_name").toString();

  q.prepare( "SELECT submitReportToBatch( :reportname, :fromEmail, :emailAddress, :ccAddress, :subject,"
             "                            :emailBody, :fileName, CURRENT_TIMESTAMP, :emailHTML) AS batch_id;" );
  q.bindValue(":reportname", reportName);
  q.bindValue(":fromEmail", _fromEmail->text());
  q.bindValue(":emailAddress", _email->text());
  q.bindValue(":ccAddress", _cc->text());
  q.bindValue(":subject", _subject->text());
  q.bindValue(":fileName", _fileName->text());
  q.bindValue(":emailBody", _emailBody->toPlainText());
  q.bindValue(":emailHTML", QVariant(_emailHTML->isChecked(), 0));
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
    q.bindValue(":batchparam_name", "sohead_id");
    q.bindValue(":batchparam_value", _so->id());
    q.exec();

    q.bindValue(":batchparam_batch_id", batch_id);
    q.bindValue(":batchparam_order", 2);
    q.bindValue(":batchparam_name", "title");
    q.bindValue(":batchparam_value", "Emailed Customer Copy");
    q.exec();
  }
  
//  ToDo

  if (_captive)
    accept();
  else
  {
    _so->setId(-1);
    _so->setFocus();
    _submit->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _email->clear();
    _cc->clear();
    _emailBody->clear();
    _subject->clear();
    _fileName->clear();
  }
}

void deliverSalesOrder::sHandleSoheadid()
{
  q.prepare( "SELECT cust_soemaildelivery, cust_soediemail, cust_soediemailbody, "
             "       cust_soedisubject, cust_soedifilename, cust_soedicc, cust_soediemailhtml, "
             "       cohead_number "
             "FROM cohead, custinfo "
             "WHERE ( (cohead_cust_id=cust_id)"
             " AND (cohead_id=:sohead_id) );" );
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    if (q.value("cust_soemaildelivery").toBool())
    {
      _email->setText(q.value("cust_soediemail").toString());
      _cc->setText(q.value("cust_soedicc").toString());
      _emailBody->setPlainText(q.value("cust_soediemailbody").toString().replace("</docnumber>", q.value("cohead_number").toString()).replace("</doctype>", "SO"));
      _subject->setText(q.value("cust_soedisubject").toString().replace("</docnumber>", q.value("cohead_number").toString()).replace("</doctype>", "SO"));
      _fileName->setText(q.value("cust_soedifilename").toString().replace("</docnumber>", q.value("cohead_number").toString()).replace("</doctype>", "SO"));
      _emailHTML->setChecked(q.value("cust_soediemailhtml").toBool());
    }
    else
    {
      _email->clear();
      _cc->clear();
      _emailBody->clear();
      _subject->clear();
      _fileName->clear();
     _emailHTML->setChecked(false);
    }
  }
}

