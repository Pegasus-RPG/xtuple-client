/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "arOpenItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "storedProcErrorLookup.h"

arOpenItem::arOpenItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustInfo(int)));
  connect(_terms, SIGNAL(newID(int)), this, SLOT(sPopulateDueDate()));
  connect(_docDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateDueDate()));

  _last = -1;

  _arapply->addColumn(tr("Type"),            _dateColumn, Qt::AlignCenter,true, "doctype");
  _arapply->addColumn(tr("Doc. #"),                   -1, Qt::AlignLeft,  true, "docnumber");
  _arapply->addColumn(tr("Apply Date"),      _dateColumn, Qt::AlignCenter,true, "arapply_postdate");
  _arapply->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight, true, "arapply_applied");
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,  true, "currabbr");
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight, true, "baseapplied");

  _printOnPost->setVisible(false);

  if (omfgThis->singleCurrency())
      _arapply->hideColumn("currabbr");

  _terms->setType(XComboBox::ARTerms);
  _salesrep->setType(XComboBox::SalesReps);

  _altSalescatid->setType(XComboBox::SalesCategories);

  _rsnCode->setType(XComboBox::ReasonCodes);

  _journalNumber->setEnabled(FALSE);
  _commissionPaid->setEnabled(FALSE);
}

arOpenItem::~arOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void arOpenItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arOpenItem::set( const ParameterList &pParams )
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("docType", &valid);
  if (valid)
  {
    if (param.toString() == "creditMemo")
    {
      setWindowTitle(caption() + tr(" - Enter Misc. Credit Memo"));
      _docType->setCurrentIndex(0);
    }
    else if (param.toString() == "debitMemo")
    {
      setWindowTitle(caption() + tr(" - Enter Misc. Debit Memo"));
      _docType->setCurrentIndex(1);
    }
    else if (param.toString() == "invoice")
      _docType->setCurrentIndex(2);
    else if (param.toString() == "customerDeposit")
      _docType->setCurrentIndex(3);
    else
      return UndefinedError;
//  ToDo - better error return types

    _docType->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT fetchARMemoNumber() AS number;");
      if (q.first())
        _docNumber->setText(q.value("number").toString());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _paid->clear();
      _commissionPaid->clear();
      _save->setText(tr("Post"));
      _printOnPost->setVisible(true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _cust->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _orderNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _altPrepaid->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _cust->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _orderNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _terms->setType(XComboBox::Terms);
      _salesrep->setEnabled(FALSE);
      _commissionDue->setEnabled(FALSE);
      _commissionPaid->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _altPrepaid->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _save->hide();

      _close->setText(tr("&Close"));
    }
    else
      return UndefinedError;
  }
  
  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    populate();
  }

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    populate();
  }

  return NoError;
}

void arOpenItem::sSave()
{
  QString storedProc;

  if (_mode == cNew)
  {
    if (!_docDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                             tr("You must enter a date for this A/R Memo before you may save it") );
      _docDate->setFocus();
      return;
    }

    if (!_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                             tr("You must enter a date for this A/R Memo before you may save it") );
      _dueDate->setFocus();
      return;
    }

    if (_amount->isZero())
    {
      QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                             tr("You must enter an amount for this A/R Memo before you may save it") );
      _amount->setFocus();
      return;
    }

    if (_altPrepaid->isChecked())
    {
      if(_altSalescatidSelected->isChecked() && !_altSalescatid->isValid())
      {
        QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                               tr("You must choose a valid Alternate Sales Category for this A/R Memo before you may save it") );
        return;
      }

      if(_altAccntidSelected->isChecked() && !_altAccntid->isValid())
      {
        QMessageBox::critical( this, tr("Cannot Save A/R Memo"),
                               tr("You must choose a valid Alternate Prepaid Account Number for this A/R Memo before you may save it.") );
        return;
      }
    }

    if (_docType->currentIndex() == 0)
    {
      q.prepare( "SELECT createARCreditMemo( :cust_id, :aropen_docnumber, :aropen_ordernumber,"
                 "                           :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                           :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                           :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due,"
		 "                           :curr_id ) AS result;" );
      storedProc = "createARCreditMemo";
    }
    else if (_docType->currentIndex() == 1)
    {
      q.prepare( "SELECT createARDebitMemo( :cust_id, :aropen_docnumber, :aropen_ordernumber,"
                 "                          :aropen_docdate, :aropen_amount, :aropen_notes, :aropen_rsncode_id,"
                 "                           :aropen_salescat_id, :aropen_accnt_id, :aropen_duedate,"
                 "                           :aropen_terms_id, :aropen_salesrep_id, :aropen_commission_due, "
		 "                           :curr_id ) AS result;" );
      storedProc = "createARCreditMemo";
    }

    q.bindValue(":cust_id", _cust->id());
  }
  else if (_mode == cEdit)
  {
    if (_cAmount != _amount->localValue())
      if ( QMessageBox::warning( this, tr("A/R Open Amount Changed"),
                                 tr( "You are changing the open amount of this A/R Open Item.  If you do not post a G/L Transaction\n"
                                     "to distribute this change then the A/R Open Item total will be out of balance with the\n"
                                     "A/R Trial Balance(s).\n"
                                     "Are you sure that you want to save this change?" ),
                                 tr("Yes"), tr("No"), QString::null ) == 1 )
        return;

    q.prepare( "UPDATE aropen "
               "SET aropen_duedate=:aropen_duedate,"
               "    aropen_terms_id=:aropen_terms_id, aropen_salesrep_id=:aropen_salesrep_id,"
               "    aropen_amount=:aropen_amount,"
               "    aropen_commission_due=:aropen_commission_due, aropen_notes=:aropen_notes,"
               "    aropen_rsncode_id=:aropen_rsncode_id, "
	       "    aropen_curr_id=:curr_id "
               "WHERE (aropen_id=:aropen_id);" );
  }

  q.bindValue(":aropen_id", _aropenid);
  q.bindValue(":aropen_docdate", _docDate->date());
  q.bindValue(":aropen_duedate", _dueDate->date());
  q.bindValue(":aropen_docnumber", _docNumber->text());
  q.bindValue(":aropen_ordernumber", _orderNumber->text());
  q.bindValue(":aropen_terms_id", _terms->id());
  q.bindValue(":aropen_salesrep_id", _salesrep->id());
  q.bindValue(":aropen_amount", _amount->localValue());
  q.bindValue(":aropen_commission_due", _commissionDue->baseValue());
  q.bindValue(":aropen_notes",          _notes->toPlainText());
  q.bindValue(":aropen_rsncode_id", _rsnCode->id());
  q.bindValue(":curr_id", _amount->id());
  if(_altPrepaid->isChecked() && _altSalescatidSelected->isChecked())
    q.bindValue(":aropen_salescat_id", _altSalescatid->id());
  else
    q.bindValue(":aropen_salescat_id", -1);
  if(_altPrepaid->isChecked() && _altAccntidSelected->isChecked())
    q.bindValue(":aropen_accnt_id", _altAccntid->id());
  else
    q.bindValue(":aropen_accnt_id", -1);

  switch (_docType->currentIndex())
  {
    case 0:
      q.bindValue(":aropen_doctype", "C");
      break;

    case 1:
      q.bindValue(":aropen_doctype", "D");
      break;

    case 2:
      q.bindValue(":aropen_doctype", "I");
      break;

    case 3:
      q.bindValue(":aropen_doctype", "R");
      break;
  }

  if (q.exec())
  {
    if (_mode == cEdit)
      done(_aropenid);
    else
    {
      q.first();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      _last = q.value("result").toInt();
      if (_last < 0)
      {
	systemError(this, storedProc.isEmpty() ?
			    tr("Saving Credit Memo Failed: %1").arg(_last) :
			    storedProcErrorLookup(storedProc, _last),
		    __FILE__, __LINE__);
	return;
      }
      reset();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("SELECT MAX(aropen_id) AS max_aropen_id FROM aropen;");
  if (q.first())
  {
    if(_printOnPost->isChecked())
    {
      sPrintOnPost(q.value("max_aropen_id").toInt()); 
    }
  }
}

void arOpenItem::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT releaseARMemoNumber(:docNumber);");
    q.bindValue(":docNumber", _docNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if(_last != -1)
    {
      done(_last);
      return;
    }
  }

  reject();
}

void arOpenItem::sPopulateCustInfo(int pCustid)
{
  if ( (pCustid != -1) && (_mode == cNew) )
  {
    XSqlQuery c;
    c.prepare( "SELECT cust_terms_id, cust_salesrep_id, cust_curr_id "
               "FROM cust "
               "WHERE (cust_id=:cust_id);" );
    c.bindValue(":cust_id", pCustid);
    c.exec();
    if (c.first())
    {
      _terms->setId(c.value("cust_terms_id").toInt());
      _salesrep->setId(c.value("cust_salesrep_id").toInt());
      _amount->setId(c.value("cust_curr_id").toInt());
    }
    else if (c.lastError().type() != QSqlError::NoError)
    {
      systemError(this, c.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void arOpenItem::populate()
{
  q.prepare( "SELECT aropen_cust_id, aropen_docdate, aropen_duedate,"
             "       aropen_doctype, aropen_docnumber,"
             "       aropen_ordernumber, aropen_journalnumber,"
             "       aropen_amount, aropen_amount,"
             "       aropen_paid, "
             "       (aropen_amount - aropen_paid) AS f_balance,"
             "       aropen_terms_id, aropen_salesrep_id,"
             "       aropen_commission_due,"
             "       aropen_notes, aropen_rsncode_id, aropen_salescat_id, "
	     "       aropen_accnt_id, aropen_curr_id "
             "FROM aropen "
             "WHERE (aropen_id=:aropen_id);" );
  q.bindValue(":aropen_id", _aropenid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("aropen_cust_id").toInt());
    _docDate->setDate(q.value("aropen_docdate").toDate(), true);
    _dueDate->setDate(q.value("aropen_duedate").toDate());
    _docNumber->setText(q.value("aropen_docnumber").toString());
    _orderNumber->setText(q.value("aropen_ordernumber").toString());
    _journalNumber->setText(q.value("aropen_journalnumber").toString());
    _amount->set(q.value("aropen_amount").toDouble(),
		 q.value("aropen_curr_id").toInt(),
		 q.value("aropen_docdate").toDate(), false);
    _paid->setLocalValue(q.value("aropen_paid").toDouble());
    _balance->setLocalValue(q.value("f_balance").toDouble());
    _terms->setId(q.value("aropen_terms_id").toInt());
    _salesrep->setId(q.value("aropen_salesrep_id").toInt());
    _commissionDue->setBaseValue(q.value("aropen_commission_due").toDouble());
    _notes->setText(q.value("aropen_notes").toString());

    if(!q.value("aropen_rsncode_id").isNull() && q.value("aropen_rsncode_id").toInt() != -1)
      _rsnCode->setId(q.value("aropen_rsncode_id").toInt());

    if(!q.value("aropen_accnt_id").isNull() && q.value("aropen_accnt_id").toInt() != -1)
    {
      _altPrepaid->setChecked(TRUE);
      _altAccntidSelected->setChecked(TRUE);
      _altAccntid->setId(q.value("aropen_accnt_id").toInt());
    }

    if(!q.value("aropen_salescat_id").isNull() && q.value("aropen_salescat_id").toInt() != -1)
    {
      _altPrepaid->setChecked(TRUE);
      _altSalescatidSelected->setChecked(TRUE);
      _altSalescatid->setId(q.value("aropen_salescat_id").toInt());
    }

    QString docType = q.value("aropen_doctype").toString();
    if (docType == "C")
      _docType->setCurrentIndex(0);
    else if (docType == "D")
      _docType->setCurrentIndex(1);
    else if (docType == "I")
      _docType->setCurrentIndex(2);
    else if (docType == "R")
      _docType->setCurrentIndex(3);

    _cAmount = q.value("aropen_amount").toDouble();

    if ( (docType == "I") || (docType == "D") )
    {
      q.prepare( "SELECT arapply_id, arapply_source_aropen_id,"
                 "       CASE WHEN (arapply_source_doctype = 'C') THEN :creditMemo"
                 "            WHEN (arapply_source_doctype = 'R') THEN :cashdeposit"
                 "            WHEN (arapply_fundstype='C') THEN :check"
                 "            WHEN (arapply_fundstype='T') THEN :certifiedCheck"
                 "            WHEN (arapply_fundstype='M') THEN :masterCard"
                 "            WHEN (arapply_fundstype='V') THEN :visa"
                 "            WHEN (arapply_fundstype='A') THEN :americanExpress"
                 "            WHEN (arapply_fundstype='D') THEN :discoverCard"
                 "            WHEN (arapply_fundstype='R') THEN :otherCreditCard"
                 "            WHEN (arapply_fundstype='K') THEN :cash"
                 "            WHEN (arapply_fundstype='W') THEN :wireTransfer"
                 "            WHEN (arapply_fundstype='O') THEN :other"
                 "       END AS doctype,"
                 "       CASE WHEN (arapply_source_doctype IN ('C','R')) THEN arapply_source_docnumber"
                 "            WHEN (arapply_source_doctype = 'K') THEN arapply_refnumber"
                 "            ELSE :other"
                 "       END AS docnumber,"
                 "       arapply_postdate, arapply_applied, "
                 "       currConcat(arapply_curr_id) AS currabbr,"
                 "       currToBase(arapply_curr_id, arapply_applied, arapply_postdate) AS baseapplied,"
                 "       'curr' AS arapply_applied_xtnumericrole,"
                 "       'curr' AS baseapplied_xtnumericrole "
                 "FROM arapply "
                 "WHERE (arapply_target_aropen_id=:aropen_id) "
                 "ORDER BY arapply_postdate;" );

      q.bindValue(":creditMemo", tr("Credit Memo"));
      q.bindValue(":cashdeposit", tr("Cash Deposit"));
      q.bindValue(":check", tr("Check"));
      q.bindValue(":certifiedCheck", tr("Certified Check"));
      q.bindValue(":masterCard", tr("Master Card"));
      q.bindValue(":visa", tr("Visa"));
      q.bindValue(":americanExpress", tr("American Express"));
      q.bindValue(":discoverCard", tr("Discover Card"));
      q.bindValue(":otherCreditCard", tr("Other Credit Card"));
      q.bindValue(":cash", tr("Cash"));
      q.bindValue(":wireTransfer", tr("Wire Transfer"));
      q.bindValue(":other", tr("Other"));
    }
    else if (docType == "C" || docType == "R")
    {
      q.prepare( "SELECT arapply_id, arapply_target_aropen_id,"
                 "       CASE WHEN (arapply_target_doctype = 'I') THEN :invoice"
                 "            WHEN (arapply_target_doctype = 'D') THEN :debitMemo"
                 "            WHEN (arapply_target_doctype = 'K') THEN :apcheck"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       arapply_target_docnumber AS docnumber,"
                 "       arapply_postdate, arapply_applied,"
                 "       currConcat(arapply_curr_id) AS currabbr,"
                 "       currToBase(arapply_curr_id, arapply_applied, arapply_postdate) AS baseapplied,"
                 "       'curr' AS arapply_applied_xtnumericrole,"
                 "       'curr' AS baseapplied_xtnumericrole "
                 "FROM arapply "
                 "WHERE (arapply_source_aropen_id=:aropen_id) "
                 "ORDER BY arapply_postdate;" );

      q.bindValue(":invoice", tr("Invoice"));
      q.bindValue(":debitMemo", tr("Debit Memo"));
      q.bindValue(":apcheck", tr("A/P Check"));
    }

    q.bindValue(":error", tr("Error"));
    q.bindValue(":aropen_id", _aropenid);
    q.exec();
    _arapply->populate(q, TRUE);
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void arOpenItem::reset()
{
  _cust->setId(-1);
  _docDate->clear();
  _dueDate->clear();
  _docNumber->clear();
  _orderNumber->clear();
  _journalNumber->clear();
  _terms->setId(-1);
  _salesrep->setId(-1);
  _commissionDue->clear();
  _commissionPaid->clear();
  _amount->clear();
  _paid->clear();
  _balance->clear();
  _rsnCode->setId(-1);
  _altPrepaid->setChecked(false);
  _notes->clear();
  _arapply->clear();

  _cust->setFocus();

  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void arOpenItem::sPopulateDueDate()
{
  if ( (_terms->isValid()) && (_docDate->isValid()) && (!_dueDate->isValid()) )
  {
    q.prepare("SELECT determineDueDate(:terms_id, :docDate) AS duedate;");
    q.bindValue(":terms_id", _terms->id());
    q.bindValue(":docDate", _docDate->date());
    q.exec();
    if (q.first())
      _dueDate->setDate(q.value("duedate").toDate());
  }
}

void arOpenItem::sPrintOnPost(int temp_id)
{
  ParameterList params;
  params.append("aropen_id", temp_id);

  switch (_docType->currentIndex())
  {
    case 0:
      params.append("docTypeRC", "1");
      break;
    case 1:
      params.append("docTypeID", "1");
      break;
    case 2:
      params.append("docTypeID", "1");
      break;
    case 3:
      params.append("docTypeRC", "1");
      break;
  }

  orReport report("AROpenItem", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
    return;
  }
}
