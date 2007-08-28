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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "cashReceipt.h"

#include <QVariant>
#include <QValidator>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QSqlError>
#include <stdlib.h>
#include "cashReceiptItem.h"
#include "cashReceiptMiscDistrib.h"
#include "creditCard.h"

const char *_fundsTypes[] = { "C", "T", "M", "V", "A", "D", "R", "K", "W", "O" };
const int _fundsTypeCount = 9;

/*
 *  Constructs a cashReceipt as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
cashReceipt::cashReceipt(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_received, SIGNAL(lostFocus()), this, SLOT(sUpdateBalance()));
  connect(_applyToBalance, SIGNAL(clicked()), this, SLOT(sApplyToBalance()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_applyLineBalance, SIGNAL(clicked()), this, SLOT(sApplyLineBalance()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_searchDocNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged()));
  connect(_applied, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_miscDistribs, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_received, SIGNAL(valueChanged()), this, SLOT(sUpdateBalance()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sFillApplyList()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sFillMiscList()));
  connect(_received, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillApplyList()));
  connect(_received, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillMiscList()));
  connect(_received, SIGNAL(idChanged(int)), this, SLOT(sChangeCurrency(int)));
  connect(_newCC, SIGNAL(clicked()), this, SLOT(sNewCreditCard()));
  connect(_editCC, SIGNAL(clicked()), this, SLOT(sEditCreditCard()));
  connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
  connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_downCC, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_fundsType, SIGNAL(activated(int)), this, SLOT(setCreditCard()));

  QButtonGroup * bg = new QButtonGroup(this);
  bg->addButton(_balCreditMemo);
  bg->addButton(_balCustomerDeposit);

  statusBar()->hide();
  
  _applied->clear();

  _cust->setAutoFocus(false);

  _CCCVV->setValidator(new QIntValidator(100, 9999, this));

  _bankaccnt->setType(XComboBox::ARBankAccounts);
  _salescat->setType(XComboBox::SalesCategories);
  
  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Ord. #"),    _orderColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Open"),      _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft   );
  _aropen->addColumn(tr("Applied"),   _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft   );

  _cashrcptmisc->addColumn(tr("Account #"), _itemColumn,     Qt::AlignCenter );
  _cashrcptmisc->addColumn(tr("Notes"),     -1,              Qt::AlignLeft   );
  _cashrcptmisc->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight  );

  _cc->addColumn(tr("Sequence"),        _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Type"),            _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Number"),          _itemColumn,  Qt::AlignRight );
  _cc->addColumn(tr("Active"),          _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Name"),            _itemColumn,  Qt::AlignLeft );
  _cc->addColumn(tr("Expiration Date"), -1,           Qt::AlignLeft );

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn(6);
    _aropen->hideColumn(8);
    _cashrcptmisc->hideColumn(3);
  }
  
  key = omfgThis->_key;
  if(!_metrics->boolean("CCAccept") || key.length() == 0 || key.isNull() || key.isEmpty())
    _tab->removePage(_tab->page(1));
  
  if(_metrics->boolean("HideApplyToBalance"))
    _applyToBalance->hide();

  if(_metrics->boolean("EnableCustomerDeposits"))
    _balCustomerDeposit->setChecked(true);
  else
  {
   _applyBalLit->hide();
   _balCreditMemo->hide();
   _balCustomerDeposit->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
cashReceipt::~cashReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void cashReceipt::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceipt::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
  {
    _cashrcptid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
      if (q.first())
        _cashrcptid = q.value("cashrcpt_id").toInt();
//  ToDo

      _distDate->setDate(omfgThis->dbDate(), true);

      q.prepare( "INSERT INTO cashrcpt "
                 "( cashrcpt_id, cashrcpt_cust_id, cashrcpt_distdate, cashrcpt_amount,"
                 "  cashrcpt_fundstype, cashrcpt_bankaccnt_id ) "
                 "VALUES "
                 "( :cashrcpt_id, -1, CURRENT_DATE, 0.0,"
                 "  '', -1 );" );
      q.bindValue(":cashrcpt_id", _cashrcptid);
      q.exec();

      _cust->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
     _tab->removePage(_tab->page(1));

      _cust->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
     _tab->removePage(_tab->page(1));

      _cust->setReadOnly(TRUE);
      _received->setEnabled(FALSE);
      _fundsType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _bankaccnt->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
      _aropen->setEnabled(FALSE);
      _cashrcptmisc->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _applyToBalance->setEnabled(FALSE);
      _add->setEnabled(FALSE);
      _balCreditMemo->setEnabled(false);
      _balCustomerDeposit->setEnabled(false);
      _save->hide();
      _close->setText(tr("&Close"));
      _altAccnt->setEnabled(false);
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _close->setFocus();
    }
    
    _ccEdit = FALSE;
    
    if (_mode == cEdit)
    {
// We need to check and see if we are dealing a credit card cash receipt
//  If so we need to shut everythin down so no changes can be made
       if (_origFunds == "A" || _origFunds == "D" || _origFunds == "M" || _origFunds == "V")
       {
         _ccEdit = TRUE;
       }
    }
    
    if(_ccEdit)
    {
// Ok let's shut it down for edits
      _received->setEnabled(FALSE);
      _fundsType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _bankaccnt->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
    }

  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

void cashReceipt::sApplyToBalance()
{
  XSqlQuery applyToBal;
  applyToBal.prepare( "UPDATE cashrcpt "
             "SET cashrcpt_cust_id=:cust_id, cashrcpt_curr_id=:curr_id "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  applyToBal.bindValue(":cust_id", _cust->id());
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.exec();

  applyToBal.prepare("SELECT applyCashReceiptToBalance(:cashrcpt_id, "
                     "             :amount, :curr_id) AS result;");
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":amount", _received->localValue());
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.exec();
  if (applyToBal.lastError().type() != QSqlError::NoError)
      systemError(this, applyToBal.lastError().databaseText(), __FILE__, __LINE__);

  sFillApplyList();
}

void cashReceipt::sApply()
{
  bool update  = FALSE;
  QList<QTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    ParameterList params;

    if(cursor->altId() != -1)
    {
      params.append("mode", "edit");
      params.append("cashrcptitem_id", cursor->altId());
    }
    else
      params.append("mode", "new");
    params.append("cashrcpt_id", _cashrcptid);
    params.append("aropen_id", cursor->id());
    params.append("curr_id", _received->id());

    cashReceiptItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      update = TRUE;
  }
  
  if (update)
    sFillApplyList();
}

void cashReceipt::sApplyLineBalance()
{
  QList<QTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.prepare( "SELECT applycashreceiptlinebalance(:cashrcpt_id,:cashrcptitem_aropen_id,:amount,:curr_id);" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.bindValue(":cashrcptitem_aropen_id", cursor->id());
    q.bindValue(":amount", _received->localValue());
    q.bindValue(":curr_id", _received->id());
    q.exec();
  }

  sFillApplyList();
}

void cashReceipt::sClear()
{
  QList<QTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.prepare( "DELETE FROM cashrcptitem "
               " WHERE ((cashrcptitem_aropen_id=:aropen_id) "
               " AND (cashrcptitem_cashrcpt_id=:cashrcpt_id));" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.bindValue(":aropen_id", cursor->id());
    q.exec();
  }

  sFillApplyList();
}

void cashReceipt::sAdd()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cashrcpt_id", _cashrcptid);
  params.append("curr_id", _received->id());
  params.append("effective", _received->effective());

  cashReceiptMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcptmisc_id", _cashrcptmisc->id());
  params.append("curr_id", _received->id());
  params.append("effective", _received->effective());

  cashReceiptMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sDelete()
{
  q.prepare( "DELETE FROM cashrcptmisc "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  q.bindValue(":cashrcptmisc_id", _cashrcptmisc->id());
  q.exec();

  sFillMiscList();
}

void cashReceipt::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM cashrcpt "
               "WHERE (cashrcpt_id=:cashrcpt_id); "

               "DELETE FROM cashrcptitem "
               "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id); "

               "DELETE FROM cashrcptmisc "
               "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
    q.bindValue(":cashrcpt_id", _cashrcptid);
    q.exec();
  }

  close();
}

void cashReceipt::sSave()
{
  int _bankaccnt_curr_id;
  QString _bankaccnt_currAbbr;
  q.prepare( "SELECT bankaccnt_curr_id, "
             "       currConcat(bankaccnt_curr_id) AS currAbbr "
             "  FROM bankaccnt "
             " WHERE (bankaccnt_id=:bankaccnt_id);");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
      _bankaccnt_curr_id = q.value("bankaccnt_curr_id").toInt();
      _bankaccnt_currAbbr = q.value("currAbbr").toString();
  }
  else
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
  }

  if (_received->id() != _bankaccnt_curr_id &&
      QMessageBox::question(this, tr("Cash Receipt Transaction Not In Bank Currency"),
                          tr("This transaction is specified in %1 while the "
                             "Bank Account is specified in %2.\nDo you wish to "
                             "convert at the current Exchange Rate?\n\nIf not, click NO "
                             "and change the Bank Account in the POST TO field.")
                          .arg(_received->currAbbr())
                          .arg(_bankaccnt_currAbbr),
                          QMessageBox::Yes|QMessageBox::Escape,
                          QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
        _received->setFocus();
        return;
  }
    
  myFunds = QString(*(_fundsTypes + _fundsType->currentItem()));
  if (!_ccEdit)
  {
      if (myFunds == "A" || myFunds == "D" || myFunds == "M" || myFunds == "V")
      {
// We have a new credit card transaction to process  
        _passPrecheck = true;

        precheckCreditCard();
    
        if (!_passPrecheck)
          return;
    
        if (_metrics->value("CCConfirmTrans") == "A" || _metrics->value("CCConfirmTrans") == "B")
        {
          switch( QMessageBox::question( this, tr("Confirm Charge of Credit Card Purchase"),
                    tr("You must confirm that you wish to charge credit card %1\n"
                       "in the amount of $%2. Would you like to charge now?")
                       .arg(_credit_card_x)
                       .arg(_received->localValue()),
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No | QMessageBox::Escape ) )
          {
            case QMessageBox::Yes:
                break;
            case QMessageBox::No:
            default:
              return;
          }
        }
    
        _chargeBankAccount =   _metrics->value("CCDefaultBank").toInt();
    
        if (_chargeBankAccount < 1)
        {
          QMessageBox::warning( this, tr("Bank Account Missing"),
                              tr("The bank default bank account for credit card processing has not been assigned.") );
          _received->setFocus();
          return;
        }
    
        if(YourPay)
        {
          if(!processYourPay())
            return;
        }
        if(VeriSign)
        {
          QMessageBox::warning( this, tr("VeriSign"),
                 tr("VeriSign is not yet supported") );
        }
      }
  }

  q.prepare( "UPDATE cashrcpt "
             "SET cashrcpt_cust_id=:cashrcpt_cust_id, cashrcpt_amount=:cashrcpt_amount,"
             "    cashrcpt_fundstype=:cashrcpt_fundstype, cashrcpt_docnumber=:cashrcpt_docnumber,"
             "    cashrcpt_bankaccnt_id=:cashrcpt_bankaccnt_id, cashrcpt_distdate=:cashrcpt_distdate,"
             "    cashrcpt_notes=:cashrcpt_notes, "
             "    cashrcpt_salescat_id=:cashrcpt_salescat_id, "
             "    cashrcpt_curr_id=:curr_id, cashrcpt_usecustdeposit=:cashrcpt_usecustdeposit "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );

  q.bindValue(":cashrcpt_id", _cashrcptid);
  q.bindValue(":cashrcpt_cust_id", _cust->id());
  q.bindValue(":cashrcpt_amount", _received->localValue());
  q.bindValue(":cashrcpt_fundstype", QString(*(_fundsTypes + _fundsType->currentItem())));
  q.bindValue(":cashrcpt_docnumber", _docNumber->text());
  q.bindValue(":cashrcpt_bankaccnt_id", _bankaccnt->id());
  q.bindValue(":cashrcpt_distdate", _distDate->date());
  q.bindValue(":cashrcpt_notes", _notes->text().stripWhiteSpace());
  q.bindValue(":cashrcpt_usecustdeposit", QVariant(_balCustomerDeposit->isChecked(), 0));
  q.bindValue(":curr_id", _received->id());
  if(_altAccnt->isChecked())
    q.bindValue(":cashrcpt_salescat_id", _salescat->id());
  else
    q.bindValue(":cashrcpt_salescat_id", -1);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sCashReceiptsUpdated(_cashrcptid, TRUE);

  close();
}

void cashReceipt::sPopulateCustomerInfo(int)
{
  if (_mode == cNew)
  {
    XSqlQuery cust;
    cust.prepare("SELECT cust_curr_id FROM cust WHERE cust_id = :cust_id;");
    cust.bindValue(":cust_id", _cust->id());
    cust.exec();
    if (cust.first())
      _received->setId(cust.value("cust_curr_id").toInt());
    else if (cust.lastError().type() != QSqlError::NoError)
      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          cust.lastError().databaseText());
  }

  sFillApplyList();
}

void cashReceipt::sFillApplyList()
{
  if (_cust->isValid())
  {
    _cust->setReadOnly(TRUE);

    XSqlQuery apply;
    apply.prepare( "SELECT aropen_id, COALESCE(s.cashrcptitem_id, -1),"
               "       CASE WHEN (aropen_doctype='D') THEN :debitMemo"
               "            WHEN (aropen_doctype='I') THEN :invoice"
               "       END,"
               "       aropen_docnumber, aropen_ordernumber,"
               "       formatDate(aropen_docdate),"
               "       formatDate(aropen_duedate),"
               "       formatMoney(aropen_amount - aropen_paid),"
               "       currConcat(aropen_curr_id), "
               "       formatMoney((SELECT COALESCE(SUM(p.cashrcptitem_amount), 0) "
               "                      FROM cashrcptitem p"
               "                     WHERE p.cashrcptitem_aropen_id=aropen_id)), "
               "       currConcat(cashrcpt_curr_id) "
               " FROM aropen LEFT OUTER JOIN "
               "      cashrcptitem s ON (s.cashrcptitem_aropen_id=aropen_id "
               "                     AND cashrcptitem_cashrcpt_id=:cashrcpt_id) "
               "        LEFT OUTER JOIN "
               "      cashrcpt ON (cashrcptitem_cashrcpt_id = cashrcpt_id "
               "               AND cashrcpt_id=:cashrcpt_id) "
               " WHERE ( (aropen_open)"
               "   AND (aropen_doctype IN ('D', 'I'))"
               "   AND (aropen_cust_id=:cust_id) ) "
               " ORDER BY aropen_duedate, (aropen_amount - aropen_paid)" );
    apply.bindValue(":cashrcpt_id", _cashrcptid);
    apply.bindValue(":cust_id", _cust->id());
    apply.bindValue(":debitMemo", tr("D/M"));
    apply.bindValue(":invoice", tr("Invoice"));
    apply.exec();
    _aropen->populate(apply, true);

    apply.prepare( "SELECT SUM(COALESCE(cashrcptitem_amount, 0)) AS total "
               "FROM cashrcptitem "
               "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id);" );
    apply.bindValue(":cashrcpt_id", _cashrcptid);
    apply.exec();
    if (apply.first())
    {
      _applied->setLocalValue(apply.value("total").toDouble());
    }
  }
  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());
}

void cashReceipt::sFillMiscList()
{
  XSqlQuery misc;
  misc.prepare( "SELECT cashrcptmisc_id, formatGLAccount(cashrcptmisc_accnt_id),"
             "       firstLine(cashrcptmisc_notes), "
             "       formatMoney(cashrcptmisc_amount)"
             "FROM cashrcptmisc "
             "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  _cashrcptmisc->populate(misc);

  misc.prepare( "SELECT SUM(cashrcptmisc_amount) AS total "
             "FROM cashrcptmisc "
             "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  if (misc.first())
    _miscDistribs->setLocalValue(misc.value("total").toDouble());
  else
    _miscDistribs->clear();
  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());
}

void cashReceipt::sUpdateBalance()
{
  _balance->setLocalValue(_received->localValue() - _applied->localValue() -
                          _miscDistribs->localValue());
  if (!_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("red"));
}

void cashReceipt::populate()
{
  q.prepare( "SELECT cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id, "
             "       cashrcpt_fundstype, cashrcpt_docnumber, cashrcpt_bankaccnt_id,"
             "       cashrcpt_distdate, cashrcpt_notes, cashrcpt_salescat_id,"
             "       cashrcpt_usecustdeposit "
             "FROM cashrcpt "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  q.bindValue(":cashrcpt_id", _cashrcptid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("cashrcpt_cust_id").toInt());
    _received->set(q.value("cashrcpt_amount").toDouble(),
                   q.value("cashrcpt_curr_id").toInt(),
                   q.value("cashrcpt_distdate").toDate(), false);
    _docNumber->setText(q.value("cashrcpt_docnumber").toString());
    _bankaccnt->setId(q.value("cashrcpt_bankaccnt_id").toInt());
    _distDate->setDate(q.value("cashrcpt_distdate").toDate(), true);
    _notes->setText(q.value("cashrcpt_notes").toString());
    if(q.value("cashrcpt_salescat_id").toInt() != -1)
    {
      _altAccnt->setChecked(TRUE);
      _salescat->setId(q.value("cashrcpt_salescat_id").toInt());
    }
    if(q.value("cashrcpt_usecustdeposit").toBool())
      _balCustomerDeposit->setChecked(true);
    else
      _balCreditMemo->setChecked(true);

    for (int counter = 0; counter < _fundsType->count(); counter++)
      if (QString(q.value("cashrcpt_fundstype").toString()[0]) == _fundsTypes[counter])
        {
          _fundsType->setCurrentItem(counter);
        }

    _origFunds = q.value("cashrcpt_fundstype").toString();
    _cust->setId(q.value("cashrcpt_cust_id").toInt());

    sFillApplyList();
    sFillMiscList();
    setCreditCard();
  }
//  ToDo
}

void cashReceipt::sSearchDocNumChanged()
{
  QString sub = _searchDocNum->text().stripWhiteSpace();
  if(sub.isEmpty())
    return;

  QList<QTreeWidgetItem*> list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchCaseSensitive, 1);
  if(list.isEmpty())
    list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchStartsWith|Qt::MatchCaseSensitive, 1);

  if(!list.isEmpty())
  {
    _aropen->setCurrentItem(list.at(0));
    _aropen->scrollTo(_aropen->currentIndex());
  }
}

void cashReceipt::sChangeCurrency( int newId)
{
  if (_received->isEnabled())
  {
    XSqlQuery id;
    id.prepare( "UPDATE cashrcpt "
                "SET cashrcpt_curr_id=:curr_id "
                "WHERE (cashrcpt_id=:cashrcpt_id);" );
    id.bindValue(":cashrcpt_id", _cashrcptid);
    id.bindValue(":curr_id", newId);
    id.exec();
    if (id.lastError().type() != QSqlError::NoError)
      systemError(this, id.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void cashReceipt::setCreditCard()
{
  key = omfgThis->_key;
  
  q.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key));");
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":key", key);
  q.exec(); 
  
//  QString myFunds;
  myFunds = QString(*(_fundsTypes + _fundsType->currentItem()));
  
  q.prepare( "SELECT ccard_id,"
             "       ccard_seq,"
             "       CASE WHEN (ccard_type='M') THEN :masterCard"
             "            WHEN (ccard_type='V') THEN :visa"
             "            WHEN (ccard_type='A') THEN :americanExpress"
             "            WHEN (ccard_type='D') THEN :discover"
             "            ELSE :other"
             "       END AS creditcard," 
             "       formatccnumber(decrypt(setbytea(ccard_number), setbytea(:key), 'bf')) AS ccard_number,"
             "       formatBoolYN(ccard_active), "
             "       formatbytea(decrypt(setbytea(ccard_name), setbytea(:key), 'bf')) AS ccard_name,"
             "       formatbytea(decrypt(setbytea(ccard_month_expired), setbytea(:key), 'bf')) ||  '-' ||formatbytea(decrypt(setbytea(ccard_year_expired), setbytea(:key), 'bf')) AS ccard_expired "
             "FROM ccard "
             "WHERE ((ccard_cust_id=:cust_id) "
             " AND       (ccard_type=:ccard_type)) "
             "ORDER BY ccard_seq;" );
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":ccard_type", myFunds);
  q.bindValue(":masterCard", tr("MasterCard"));
  q.bindValue(":visa", tr("VISA"));
  q.bindValue(":americanExpress", tr("American Express"));
  q.bindValue(":discover", tr("Discover"));
  q.bindValue(":other", tr("Other"));
  q.bindValue(":key", key);
  q.exec();
//  _metrics->set("CCsql", q.executedQuery());
  _cc->populate(q);
}

void cashReceipt::precheckCreditCard()
{
  if(!_metrics->boolean("CCAccept"))
  {
    QMessageBox::warning( this, tr("Credit Cards"),
                          tr("The application is not set up to process credit cards") );
    _received->setFocus();
    _passPrecheck = false;
    return;
  }

  key =  omfgThis->_key;

  if(key.length() == 0 || key.isEmpty() || key.isNull())
  {
    QMessageBox::warning( this, tr("Encryption Key"),
                          tr("You do not have an encryption key defined") );
    _received->setFocus();
    _passPrecheck = false;
    return;
  }

  q.prepare( "SELECT ccard_active, "
             "       formatbytea(decrypt(setbytea(ccard_number), setbytea(:key), 'bf')) AS ccard_number,"
             "       formatccnumber(decrypt(setbytea(ccard_number), setbytea(:key), 'bf')) AS ccard_number_x,"
             "       formatbytea(decrypt(setbytea(ccard_name), setbytea(:key), 'bf')) AS ccard_name,"
             "       formatbytea(decrypt(setbytea(ccard_address1), setbytea(:key), 'bf')) AS ccard_address1,"
             "       formatbytea(decrypt(setbytea(ccard_address2), setbytea(:key), 'bf')) AS ccard_address2,"
             "       formatbytea(decrypt(setbytea(ccard_city), setbytea(:key), 'bf')) AS ccard_city,"
             "       formatbytea(decrypt(setbytea(ccard_state), setbytea(:key), 'bf')) AS ccard_state,"
             "       formatbytea(decrypt(setbytea(ccard_zip), setbytea(:key), 'bf')) AS ccard_zip,"
             "       formatbytea(decrypt(setbytea(ccard_country), setbytea(:key), 'bf')) AS ccard_country,"
             "       formatbytea(decrypt(setbytea(ccard_month_expired), setbytea(:key), 'bf')) AS ccard_month_expired,"
             "       formatbytea(decrypt(setbytea(ccard_year_expired), setbytea(:key), 'bf')) AS ccard_year_expired, "
             "       ccard_type "
             "  FROM ccard "
             " WHERE (ccard_id=:ccard_id); ");
  q.bindValue(":ccard_id", _cc->id());
  q.bindValue(":key", key);
  q.exec();
  q.first();

  _ccActive = q.value("ccard_active").toBool();

  if (!_ccActive)
  {
    QMessageBox::warning( this, tr("Invalid Credit Card"),
                          tr("The Credit Card you are attempting to use is not active.\n"
                             "Make the card active or select another credit card.") );
    _received->setFocus();
    _passPrecheck = false;
    return;
  }

  _ccard_number = q.value("ccard_number").toString();
  _credit_card_x = q.value("ccard_number_x").toString();
  _ccard_name = q.value("ccard_name").toString();
  _ccard_address1 = q.value("ccard_address1").toString();
  _ccard_address2 = q.value("ccard_address2").toString();
  _ccard_city = q.value("ccard_city").toString();
  _ccard_state = q.value("ccard_state").toString();
  _ccard_zip = q.value("ccard_zip").toString();
  _ccard_country = q.value("ccard_country").toString();
  _ccard_month_expired = q.value("ccard_month_expired").toInt();
  _ccard_year_expired = q.value("ccard_year_expired").toInt();
  _ccard_type = q.value("ccard_type").toString();

  YourPay = false;
  VeriSign = false;

  if (_metrics->value("CCServer") == "test-payflow.verisign.com"  ||  _metrics->value("CCServer") == "payflow.verisign.com") 
  {
    VeriSign = true; 
  }

  if (_metrics->value("CCServer") == "staging.linkpt.net"  ||  _metrics->value("CCServer") == "secure.linkpt.net") 
  {
    YourPay = true; 
  }

  if (!YourPay && !VeriSign)
  {
    QMessageBox::warning( this, tr("Invalid Credit Card Service Selected"),
                          tr("OpenMFG only supports YourPay and VeriSign.  You have not selected either of these as your credit card processors.") );
    _received->setFocus();
    _passPrecheck = false;
    return;
  }

  if (VeriSign)
  {
    if ((_metrics->boolean("CCTest") && _metrics->value("CCServer") == "test-payflow.verisign.com") || (!_metrics->boolean("CCTest") && _metrics->value("CCServer") == "payflow.verisign.com"))
    {
// This is OK - We are either running test and test or production and production on Verisign
    }
    else
    {
// Not OK - we have an error
      QMessageBox::warning( this, tr("Invalid Server Configuration"),
                            tr("If Credit Card test is selected you must select a test server.  If Credit Card Test is not selected you must select a production server") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }
  }


  if (YourPay)
  {
    if ((_metrics->boolean("CCTest") && _metrics->value("CCServer") == "staging.linkpt.net") || (!_metrics->boolean("CCTest") && _metrics->value("CCServer") == "secure.linkpt.net"))
    {
// This is OK - We are either running test and test or production and production on Verisign
    }
    else
    {
// Not OK - we have an error
      QMessageBox::warning( this, tr("Invalid Server Configuration"),
                            tr("If Credit Card test is selected you must select a test server.  If Credit Card Test is not selected you must select a production server") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }
  }
 
  port = _metrics->value("CCPort").toInt();
  
  if (YourPay)
    if (port != 1129)
    {
      QMessageBox::warning( this, tr("Invalid Server Port Configuration"),
                            tr("You have an invalid port identified for the requested server") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }
  
  if (VeriSign)
    if (port != 443)
    {
      QMessageBox::warning( this, tr("Invalid Server Port Configuration"),
                            tr("You have an invalid port identified for the requested server") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }

  if (YourPay)
  {
// Set up all of the YourPay parameters and check them
    _linkshield = _metrics->boolean("CCYPLinkShield");
    _maxlinkshield = _metrics->value("CCYPLinkShieldMax").toInt();
    _storenum = _metricsenc->value("CCYPStoreNum");

#ifdef Q_WS_WIN
    _pemfile = _metrics->value("CCYPWinPathPEM");
#elif defined Q_WS_MACX
    _pemfile = _metrics->value("CCYPMacPathPEM");
#elif defined Q_WS_X11
    _pemfile = _metrics->value("CCYPLinPathPEM");
#endif

    if (_storenum.length() == 0 || _storenum.isEmpty() || _storenum.isNull())
    {
      QMessageBox::warning( this, tr("Store Number Missing"),
                            tr("The YourPay Store Number is missing") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }

    if (_pemfile.length() == 0 || _pemfile.isEmpty() || _pemfile.isNull())
    {
      QMessageBox::warning( this, tr("Pem File Missing"),
                            tr("The YourPay pem file is missing") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }

    if (_received->localValue() <= 0)
    {
      QMessageBox::warning( this, tr("Charge Amount Missing or incorrect"),
                            tr("The Charge Amount must be greater than 0.00") );
      _received->setFocus();
      _passPrecheck = false;
      return;
    }

    _noCVV = false;

    if ((int)_CCCVV->toDouble() < 100 || (int)_CCCVV->toDouble() > 9999)
    {
      if ((int)_CCCVV->toDouble() == 0)
      {
        switch( QMessageBox::question( this, tr("Confirm No CVV Code"),
            tr("You must confirm that you wish to proceed without a CVV Code.\n"
               "Would you like to continue now?"),
            QMessageBox::Yes | QMessageBox::Default,
            QMessageBox::No  | QMessageBox::Escape ) )
        {
          case QMessageBox::Yes:
            _noCVV = true;
            break;
          case QMessageBox::No:
          default:
            _CCCVV->setFocus();
            _passPrecheck = false;
            return;
        }
      }
      else
      {
        QMessageBox::warning( this, tr("Improper CVV Code"),
                              tr("The CVV Code must be numeric and between 100 and 9999") );
        _CCCVV->setFocus();
        _passPrecheck = false;
        return;
      }
    }

    _cvv = (int)_CCCVV->toDouble();


// We got this far time to load everything up.

    configfile = new char[strlen(_storenum.latin1()) + 1];
    strcpy(configfile, _storenum.latin1());
    host = new char[strlen(_metrics->value("CCServer").latin1()) + 1];
    strcpy(host, _metrics->value("CCServer").latin1());
    pemfile = new char[strlen(_pemfile.latin1()) + 1];
    strcpy(pemfile, _pemfile.latin1());

    QFileInfo fileinfo( pemfile );

    if (!fileinfo.isFile())
    {
// Oops we don't have a usable pemfile
      QMessageBox::warning( this, tr("Missing PEM FIle"),
           tr("Unable to verify that you have a PEM file for this application\nPlease contact your local support") );
     _CCCVV->setFocus();
     _passPrecheck = false;
     return;
    }

    _numtospace = _ccard_address1.find(" ");
    addrnum = _ccard_address1.left( _numtospace);

  }
  QString plogin;
  QString ppassword;
  QString pserver;
  QString pport;
    
  plogin = _metricsenc->value("CCProxyLogin");
  ppassword = _metricsenc->value("CCPassword");
  pserver = _metrics->value("CCProxyServer");
  pport = _metrics->value("CCProxyPort");

  if(_metrics->boolean("CCUseProxyServer"))
  {
    if (plogin.length() == 0 || plogin.isEmpty() || plogin.isNull())
    {
      QMessageBox::warning( this, tr("Missing Proxy Server Data"),
           tr("You have selected proxy server support, yet you have not provided a login") );
     _passPrecheck = false;
     return;
    }
    if (ppassword.length() == 0 || ppassword.isEmpty() || ppassword.isNull())
    {
      QMessageBox::warning( this, tr("Missing Proxy Server Data"),
           tr("You have selected proxy server support, yet you have not provided a password") );
     _passPrecheck = false;
     return;
    }
    if (pserver.length() == 0 || pserver.isEmpty() || pserver.isNull())
    {
      QMessageBox::warning( this, tr("Missing Proxy Server Data"),
           tr("You have selected proxy server support, yet you have not provided a proxy server to use") );
     _passPrecheck = false;
     return;
    }
    if (pport.length() == 0 || pport.isEmpty() || pport.isNull())
    {
      QMessageBox::warning( this, tr("Missing Proxy Server Data"),
           tr("You have selected proxy server support, yet you have not provided a lport to use") );
     _passPrecheck = false;
     return;
    }
  }
}

bool cashReceipt::processYourPay()
{
  bool good = true;
  QDomDocument odoc;
  // Build the order
  QDomElement root = odoc.createElement("order");
  odoc.appendChild(root);
  QDomElement elem, sub;

  // add the 'billing'
  elem = odoc.createElement("billing");

  sub = odoc.createElement("address1");
  sub.appendChild(odoc.createTextNode(_ccard_address1));
  elem.appendChild(sub);

  sub = odoc.createElement("address2");
  sub.appendChild(odoc.createTextNode(_ccard_address2));
  elem.appendChild(sub);

  sub = odoc.createElement("addrnum");
  sub.appendChild(odoc.createTextNode(addrnum));
  elem.appendChild(sub);

  sub = odoc.createElement("city");
  sub.appendChild(odoc.createTextNode(_ccard_city));
  elem.appendChild(sub);

  sub = odoc.createElement("name");
  sub.appendChild(odoc.createTextNode(_ccard_name));
  elem.appendChild(sub);

  sub = odoc.createElement("state");
  sub.appendChild(odoc.createTextNode(_ccard_state));
  elem.appendChild(sub);

  sub = odoc.createElement("zip");
  sub.appendChild(odoc.createTextNode(_ccard_zip));
  elem.appendChild(sub);

  root.appendChild(elem);
  
  // add the 'credit card'
  elem = odoc.createElement("creditcard");

  QString work_month;
  work_month.setNum(_ccard_month_expired);
  if (work_month.length() == 1)
      work_month = "0" + work_month;
  sub = odoc.createElement("cardexpmonth");
  sub.appendChild(odoc.createTextNode(work_month));
  elem.appendChild(sub);

  QString work_year;
  work_year.setNum(_ccard_year_expired);
  work_year = work_year.right(2);
  sub = odoc.createElement("cardexpyear");
  sub.appendChild(odoc.createTextNode(work_year));
  elem.appendChild(sub);

  sub = odoc.createElement("cardnumber");
  sub.appendChild(odoc.createTextNode(_ccard_number));
  elem.appendChild(sub);

  QString work_cvv;
  work_cvv.setNum(_cvv);
  if (!_noCVV)
  {
    sub = odoc.createElement("cvmvalue");
    sub.appendChild(odoc.createTextNode(work_cvv));
    elem.appendChild(sub);
  }

  root.appendChild(elem);
  
  // Build 'merchantinfo'
  elem = odoc.createElement("merchantinfo");

  sub = odoc.createElement("configfile");
  sub.appendChild(odoc.createTextNode(configfile));
  elem.appendChild(sub);

  root.appendChild(elem);
  
  // Build 'orderoptions'
  elem = odoc.createElement("orderoptions");

  sub = odoc.createElement("ordertype");
  sub.appendChild(odoc.createTextNode("SALE"));
  elem.appendChild(sub);
  
  sub = odoc.createElement("result");
  sub.appendChild(odoc.createTextNode("LIVE"));
  elem.appendChild(sub);

  root.appendChild(elem);

  // Build 'payment'
  elem = odoc.createElement("payment");

  sub = odoc.createElement("chargetotal");
  // Todo: change 2 to the appropriate precision for _received->id()
  QString tmp;
  sub.appendChild(odoc.createTextNode(tmp.setNum(_received->baseValue(), 'f', 2)));
  elem.appendChild(sub);

  root.appendChild(elem);

  // Build 'transaction details'
  elem = odoc.createElement("transactiondetails");
  // No Transaction details here
  root.appendChild(elem);
  
  // Process the order
      
  saved_order = odoc.toString();

  if (_metrics->boolean("CCTest"))
  {
    _metrics->set("CCOrder", saved_order);
  }
  
  proc = new Q3Process( this );
  QString curl_path;
#ifdef Q_WS_WIN
  curl_path = qApp->applicationDirPath() + "\\curl";
#elif defined Q_WS_MACX
  curl_path = "/usr/bin/curl";
#elif defined Q_WS_X11
  curl_path = "/usr/bin/curl";
#endif
  
  proc->addArgument( curl_path );
  proc->addArgument( "-k" );
  proc->addArgument( "-d" );
  proc->addArgument( saved_order );
  proc->addArgument( "-E" );
  proc->addArgument( pemfile );
  
  _port.setNum(port);
  doServer = "https://" + _metrics->value("CCServer") + ":" + _port;
  
  proc->addArgument( doServer );
  
  QString proxy_login;
  QString proxy_server;
  
  if(_metrics->boolean("CCUseProxyServer"))
  {
    proxy_login =  _metricsenc->value("CCProxyLogin") + ":" + _metricsenc->value("CCPassword") ;
    proxy_server = _metrics->value("CCProxyServer") + ":" + _metrics->value("CCProxyPort");
    proc->addArgument( "-x" );
    proc->addArgument( proxy_server );
    proc->addArgument( "-U" );
    proc->addArgument( proxy_login );
  }
  
  _response = "";
  
  connect( proc, SIGNAL(readyReadStdout()),
           this, SLOT(readFromStdout()) );
  
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  _save->setEnabled(false);
  
  if ( !proc->start() ) 
  {
    QMessageBox::critical( 0,
        tr("Fatal error"),
        tr("Could not start the curl command."),
        tr("Quit") );
    return false;
  }
  
  while (proc->isRunning())
    qApp->processEvents();
  
  _save->setEnabled(true);
  QApplication::restoreOverrideCursor();
  
  _response =  "<myroot>" + _response + "</myroot>";
  
  QString whyMe;
  
  if (_metrics->boolean("CCTest"))
  {
    whyMe = _ccard_number + "  " + _response;
    _metrics->set("CCTestMe", whyMe);
  }
  
  /*if (_metrics->boolean("CCTest"))
  {
    QMessageBox::information(this, tr("YourPay"), tr("The return code was ") + _response, QMessageBox::Ok);
  }*/
  QDomDocument doc;
  doc.setContent(_response);
  QDomNode node;
  root = doc.documentElement();
  
  QString _r_avs;
  QString _r_ordernum;
  QString _r_error;
  QString _r_approved;
  QString _r_code;
  QString _r_score;
  QString _r_shipping;
  QString _r_tax;
  QString _r_tdate;
  QString _r_ref;
  QString _r_message;
  QString _r_time;
  
  node = root.firstChild();
  while ( !node.isNull() ) {
    if ( node.isElement() && node.nodeName() == "r_avs" ) {
      QDomElement r_avs = node.toElement();
      _r_avs = r_avs.text();
    }
    if ( node.isElement() && node.nodeName() == "r_ordernum" ) {
      QDomElement r_ordernum = node.toElement();
      _r_ordernum = r_ordernum.text();
    }
    if ( node.isElement() && node.nodeName() == "r_error" ) {
      QDomElement r_error = node.toElement();
      _r_error = r_error.text();
    }
    if ( node.isElement() && node.nodeName() == "r_approved" ) {
      QDomElement r_approved = node.toElement();
      _r_approved = r_approved.text();
    }
    if ( node.isElement() && node.nodeName() == "r_code" ) {
      QDomElement r_code = node.toElement();
      _r_code = r_code.text();
    }
    if ( node.isElement() && node.nodeName() == "r_message" ) {
      QDomElement r_message = node.toElement();
      _r_message = r_message.text();
    }
    if ( node.isElement() && node.nodeName() == "r_time" ) {
      QDomElement r_time = node.toElement();
      _r_time = r_time.text();
    }
    if ( node.isElement() && node.nodeName() == "r_ref" ) {
      QDomElement r_ref = node.toElement();
      _r_ref = r_ref.text();
    }
    if ( node.isElement() && node.nodeName() == "r_tdate" ) {
      QDomElement r_tdate = node.toElement();
      _r_tdate = r_tdate.text();
    }
    if ( node.isElement() && node.nodeName() == "r_tax" ) {
      QDomElement r_tax = node.toElement();
      _r_tax = r_tax.text();
    }
    if ( node.isElement() && node.nodeName() == "r_shipping" ) {
      QDomElement r_shipping = node.toElement();
      _r_shipping = r_shipping.text();
    }
    if ( node.isElement() && node.nodeName() == "r_score") {
      QDomElement r_score = node.toElement();
      _r_score = r_score.text();
    }
    node = node.nextSibling();
  }
  
  q.prepare( "SELECT nextval('ccpay_ccpay_id_seq') as ccpay_id;");
  q.exec();
  q.first();
    
  _ccpay_id = q.value("ccpay_id").toInt();
    
  q.prepare( "INSERT INTO ccpay"
             "      (ccpay_id, "
             "       ccpay_ccard_id, "
             "       ccpay_cust_id, "
             "       ccpay_amount, " 
             "       ccpay_curr_id, " 
             "       ccpay_auth, "
             "       ccpay_status, "
             "       ccpay_type, "
             "       ccpay_auth_charge, "
             "       ccpay_order_number, "
             "       ccpay_order_number_seq, "
             "       ccpay_yp_r_avs, "
             "       ccpay_yp_r_ordernum, "
             "       ccpay_yp_r_error, "
             "       ccpay_yp_r_approved, "
             "       ccpay_yp_r_code, "
             "       ccpay_yp_r_message, "
             "       ccpay_yp_r_time, "
             "       ccpay_yp_r_ref, "
             "       ccpay_yp_r_tdate, "
             "       ccpay_yp_r_tax, "
             "       ccpay_yp_r_shipping, "
             "       ccpay_yp_r_score) "
             "VALUES(:ccpay_id, "
             "       :ccpay_ccard_id, "
             "       :ccpay_cust_id, "
             "       :ccpay_amount, " 
             "       :ccpay_curr_id, " 
             "       :ccpay_auth, "
             "       :ccpay_status, "
             "       :ccpay_type, "
             "       :ccpay_auth_charge, "
             "       :ccpay_order_number, "
             "       :ccpay_order_number_seq, "
             "       :ccpay_yp_r_avs, "
             "       :ccpay_yp_r_ordernum, "
             "       :ccpay_yp_r_error, "
             "       :ccpay_yp_r_approved, "
             "       :ccpay_yp_r_code, "
             "       :ccpay_yp_r_message, "
             "       :ccpay_yp_r_time, "
             "       :ccpay_yp_r_ref, "
             "       :ccpay_yp_r_tdate, "
             "       :ccpay_yp_r_tax, "
             "       :ccpay_yp_r_shipping, "
             "       :ccpay_yp_r_score);");
  q.bindValue(":ccpay_id", _ccpay_id);
  q.bindValue(":ccpay_ccard_id", _cc->id());
  q.bindValue(":ccpay_cust_id",_cust->id());
  q.bindValue(":ccpay_amount",_received->baseValue()); // see comments marked
  q.bindValue(":ccpay_curr_id",_received->baseId());   // ##### in salesOrder.ui.h
  
  /*if (_doAuthorize)
  {
    q.bindValue(":ccpay_auth",QVariant(TRUE, 0));
  // Authorization or Charge for the initial transaction
    q.bindValue(":ccpay_auth_charge","A");
  }
  else
  {
    q.bindValue(":ccpay_auth",QVariant(FALSE, 1));
  // Authorization or Charge for the initial transaction
    q.bindValue(":ccpay_auth_charge","C");
  }*/
  
  doDollars = 0;
  
  if (_r_approved == "APPROVED")
  {
    QMessageBox::information(this, tr("YourPay"), tr("This transaction was approved\n") + _r_ref, QMessageBox::Ok);
    q.bindValue(":ccpay_status","C");
    doDollars = _received->baseValue();            // ##### localValue()?
  }
  
  if (_r_approved == "DENIED")
  {
    QMessageBox::information(this, tr("YourPay"), tr("This transaction was denied\n") + _r_error, QMessageBox::Ok);
    q.bindValue(":ccpay_status","D");
    good = false;
  }
  
  if (_r_approved == "DUPLICATE")
  {
    QMessageBox::information(this, tr("YourPay"), tr("This transaction is a duplicate\n") + _r_error, QMessageBox::Ok);
    q.bindValue(":ccpay_status","D");
    good = false;
  }
  
  if (_r_approved == "DECLINED")
  {
    QMessageBox::information(this, tr("YourPay"), tr("This transaction is a declined\n") + _r_error, QMessageBox::Ok);
    q.bindValue(":ccpay_status","D");
    good = false;
  }
  
  if (_r_approved == "FRAUD")
  {
    QMessageBox::information(this, tr("YourPay"), tr("This transaction is denied because of possible fraud\n") + _r_error, QMessageBox::Ok);
    q.bindValue(":ccpay_status","D");
    good = false;
  }
  
  if (_r_approved.length() == 0 || _r_approved.isNull() || _r_approved.isEmpty())
  {
    QMessageBox::information(this, tr("YourPay"), tr("No Approval Code\n") + _r_message, QMessageBox::Ok);
    q.bindValue(":ccpay_status","X");
    good = false;
  }
  
  // Credit Card for now - we may get Debit Cards in the future
  q.bindValue(":ccpay_type","C");
  // Originating Order Number
  q.bindValue(":ccpay_order_number",0);
  q.bindValue(":ccpay_order_number_seq",0);
  q.bindValue(":ccpay_yp_r_avs",_r_avs);
  q.bindValue(":ccpay_yp_r_ordernum",_r_ordernum);
  q.bindValue(":ccpay_yp_r_error",_r_error);
  q.bindValue(":ccpay_yp_r_approved",_r_approved);
  q.bindValue(":ccpay_yp_r_code",_r_code);
  q.bindValue(":ccpay_yp_r_score",_r_score.toInt());
  q.bindValue(":ccpay_yp_r_shipping",_r_shipping);
  q.bindValue(":ccpay_yp_r_tax",_r_tax);
  QDateTime myTime;
  myTime.setTime_t(_r_tdate.toInt());
  // QDateTime myTime::setTime_t(_r_tdate.toInt());
  //q.bindValue(":ccpay_yp_r_tdate",myTime);
  q.bindValue(":ccpay_yp_r_tdate",_r_tdate);
  q.bindValue(":ccpay_yp_r_ref",_r_ref);
  q.bindValue(":ccpay_yp_r_message",_r_message);
  q.bindValue(":ccpay_yp_r_time",_r_time);
  q.exec();
  
  return good;
}

void cashReceipt::readFromStdout()
{
  _response += QString(proc->readStdout());
}

void cashReceipt::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    setCreditCard();

}


void cashReceipt::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    setCreditCard();

}


void cashReceipt::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


void cashReceipt::sMoveUp()
{
  q.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();
  
  setCreditCard();
}


void cashReceipt::sMoveDown()
{
  q.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();
  
  setCreditCard();
}
