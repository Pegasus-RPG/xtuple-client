/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cashReceipt.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <metasql.h>

#include "cashReceiptItem.h"
#include "cashReceiptMiscDistrib.h"
#include "creditCard.h"
#include "creditcardprocessor.h"
#include "errorReporter.h"
#include "getGLDistDate.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

cashReceipt::cashReceipt(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  connect(_customerSelector, SIGNAL(newState(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_customerSelector, SIGNAL(newCustId(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_customerSelector, SIGNAL(newCustGroupId(int)), this, SLOT(sPopulateCustomerInfo(int)));

  connect(_received, SIGNAL(editingFinished()), this, SLOT(sUpdateBalance()));
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
  connect(_distDate, SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
  connect(_applDate, SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
  connect(_credits, SIGNAL(toggled(bool)), this, SLOT(sFillApplyList()));
  if (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _tab->removeTab(_tab->indexOf(_creditCardTab));
  }
  else
  {
    connect(_newCC, SIGNAL(clicked()), this, SLOT(sNewCreditCard()));
    connect(_customerSelector, SIGNAL(validCust(bool)), _newCC, SLOT(setEnabled(bool)));
    connect(_customerSelector, SIGNAL(validCustGroup(bool)), _newCC, SLOT(setEnabled(bool)));
    connect(_editCC, SIGNAL(clicked()), this, SLOT(sEditCreditCard()));
    connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
    connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
    connect(_downCC, SIGNAL(clicked()), this, SLOT(sMoveDown()));
    connect(_fundsType, SIGNAL(activated(int)), this, SLOT(setCreditCard()));
  }

  QButtonGroup * bg = new QButtonGroup(this);
  bg->addButton(_balCreditMemo);
  bg->addButton(_balCustomerDeposit);

  //Set up CustomerSelector widget
  _customerSelector->populate(CustomerSelector::SelectedCust | CustomerSelector::SelectedGroup);
  _customerSelector->setCurrentSelection(1);

  _customerSelector->findChild<CustCluster*>("_cust")->setFocus();

  _applied->clear();

  _CCCVV->setValidator(new QIntValidator(100, 9999, this));

  _bankaccnt->setType(XComboBox::ARBankAccounts);
  _salescat->setType(XComboBox::SalesCategoriesActive);

  _postReceipt->setEnabled(_privileges->check("PostCashReceipts"));

  _aropen->addColumn(tr("Doc. Type"), -1,              Qt::AlignCenter, true, "doctype");
  _aropen->addColumn(tr("Doc. #"),    _orderColumn,    Qt::AlignCenter, true, "aropen_docnumber");
  _aropen->addColumn(tr("Ord. #"),    _orderColumn,    Qt::AlignCenter, true, "aropen_ordernumber");
  _aropen->addColumn(tr("Doc. Date"), _dateColumn,     Qt::AlignCenter, true, "aropen_docdate");
  _aropen->addColumn(tr("Due Date"),  _dateColumn,     Qt::AlignCenter, true, "aropen_duedate");
  _aropen->addColumn(tr("Balance"),   _bigMoneyColumn, Qt::AlignRight,  true, "balance");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "balance_curr");
  _aropen->addColumn(tr("Applied"),   _bigMoneyColumn, Qt::AlignRight,  true, "applied");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "applied_curr");
  _aropen->addColumn(tr("Discount"),  _moneyColumn,    Qt::AlignRight , true, "discount" );
  _aropen->addColumn(tr("All Pending"),_moneyColumn,   Qt::AlignRight,  true, "pending");
  _aropen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft,  !omfgThis->singleCurrency(), "pending_curr");
  // Add Customer to AR List
  _aropen->addColumn(tr("Customer"),  -1,              Qt::AlignLeft, true, "cust_number");
  _aropen->addColumn(tr("Customer Name"),  -1,         Qt::AlignLeft, true, "cust_name");

  _cashrcptmisc->addColumn(tr("Account"), _itemColumn,     Qt::AlignCenter, true, "account");
  _cashrcptmisc->addColumn(tr("Notes"),     -1,              Qt::AlignLeft,  true, "firstline");
  _cashrcptmisc->addColumn(tr("Amount"),    _bigMoneyColumn, Qt::AlignRight, true, "cashrcptmisc_amount");

  _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
  _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
  _cc->addColumn(tr("Number"),  _itemColumn, Qt::AlignRight,true, "f_number");
  _cc->addColumn(tr("Active"),  _itemColumn, Qt::AlignLeft, true, "ccard_active");
  _cc->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "ccard_name");
  _cc->addColumn(tr("Expiration Date"),  -1, Qt::AlignLeft, true, "expiration");

  // Only show credit card funds types if the user can process cc transactions.
  if (_metrics->boolean("CCAccept") && _privileges->check("ProcessCreditCards"))
  {
    _fundsType->populate("SELECT fundstype_id, fundstype_name, fundstype_code FROM fundstype;");
  } else
  {
    _fundsType->populate("SELECT fundstype_id, fundstype_name, fundstype_code FROM fundstype WHERE NOT fundstype_creditcard;");
  }

  if (!_metrics->boolean("CCAccept") || ! _privileges->check("ProcessCreditCards"))
    _tab->removeTab(_tab->indexOf(_creditCardTab));

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

  if(_metrics->boolean("AltCashExchangeRate"))
  {
    connect(_altExchRate, SIGNAL(toggled(bool)), this, SLOT(sHandleAltExchRate()));
    connect(_exchRate, SIGNAL(editingFinished()), this, SLOT(sUpdateGainLoss()));
  }
  else
    _altExchRate->hide();

  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _overapplied = false;
  _cashrcptid = -1;
  _posted = false;
}

cashReceipt::~cashReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceipt::languageChange()
{
  retranslateUi(this);
}

void cashReceipt::activateButtons(bool c)
{
  _save->setEnabled(c);
  _add->setEnabled(c);
  _applyToBalance->setEnabled(c);
}

void cashReceipt::sPopulateCustomerInfo(int)
{
  activateButtons(_customerSelector->isValid());
  bool emptyGroup = true;
  XSqlQuery cust;
  QString sql;
  ParameterList pList;

  _customerSelector->appendValue(pList);

  if (_customerSelector->isSelectedCust())
    sql= "SELECT cust_curr_id FROM custinfo WHERE cust_id = <? value('cust_id') ?>;";
  else
    sql = "SELECT cust_curr_id FROM custinfo JOIN custgrpitem on cust_id=custgrpitem_cust_id "
          "WHERE custgrpitem_custgrp_id =<? value ('custgrp_id') ?> LIMIT 1;";

  MetaSQLQuery mql(sql);
  cust = mql.toQuery(pList);
  if (cust.first())
  {
    emptyGroup = false;
    if (_mode == cNew)
      _received->setId(cust.value("cust_curr_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error populating Customer Info"),
                                cust, __FILE__, __LINE__))
    return;
  sFillApplyList();
  if(emptyGroup)
    _add->setEnabled(false);
}

void cashReceipt::grpFillApplyList()
{

  ParameterList qparams = getParams();
  MetaSQLQuery dbquery = mqlLoad("arOpenApplications", "detail");
  XSqlQuery db;
  db = dbquery.toQuery(qparams);
  if (db.first())
    _aropen->populate(db, true);

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Loading Customer Group Information"),
                                db, __FILE__, __LINE__))
  {
    return;
  }

  XSqlQuery apply;
  QString sql2 = "SELECT SUM(COALESCE(cashrcptitem_amount, 0)) AS total FROM cashrcptitem "
                 "WHERE (cashrcptitem_cashrcpt_id=<? value('cashrcpt_id') ?>);";
  MetaSQLQuery mql2(sql2);
  apply = mql2.toQuery(getParams());
  if (apply.first())
    _applied->setLocalValue(apply.value("total").toDouble());
  apply.prepare("select max(aropen_docdate) AS mindate FROM aropen, cashrcptitem "
                "WHERE cashrcptitem_aropen_id=aropen_id AND cashrcptitem_cashrcpt_id=:id;");
  apply.bindValue(":id", _cashrcptid);
  apply.exec();
  if (apply.first())
  {
    _mindate = apply.value("mindate").toDate();
    if (_mindate > _applDate->date())
      _applDate->setDate(_mindate);
  }
  XSqlQuery discount;
  QString sql3 = "SELECT SUM(COALESCE(cashrcptitem_discount, 0.00)) AS disc "
                 "FROM cashrcptitem WHERE (cashrcptitem_cashrcpt_id=<? value('cashrcpt_id') ?>);";
  MetaSQLQuery mql3(sql3);
  discount = mql3.toQuery(getParams());
  if (discount.first())
    _discount->setLocalValue(discount.value("disc").toDouble());
}

ParameterList cashReceipt::getParams()
{
  ParameterList p;

  _customerSelector->appendValue(p); //cust_id or cust_grp_id
  p.append("amount", _received->localValue());
  p.append("applDate", _applDate->date());
  p.append("bankaccnt_id", _bankaccnt->id());
  p.append("cashdeposit", tr("Customer Deposit"));
  p.append("cashrcpt_id", _cashrcptid);
  p.append("cashrcptmisc_id", _cashrcptmisc->id());
  p.append("creditMemo", tr("Credit Memo"));
  p.append("curr_id", _received->id());
  p.append("custSelector", _customerSelector->selectCode());
  p.append("debitMemo", tr("Debit Memo"));
  p.append("discount", _discount->localValue());
  p.append("distDate", _distDate->date());
  p.append("docdate", _docDate->date());
  p.append("docnumber", _docNumber->text());
  p.append("effective", _received->effective());
  p.append("fundstype", _fundsType->code());
  p.append("invoice", tr("Invoice"));
  p.append("notes", _notes->toPlainText());//_notes.plainText
  if (!_credits->isChecked())
    p.append("noCredits", true);
  p.append("number", _number->text());
  p.append("salescat_id", _salescat->id());
  p.append("usecustdeposit", _balCustomerDeposit->isChecked());//->checked()?

  return p;
}


void cashReceipt::updateCustomerGroup()
{
  if (_customerSelector->isSelectedGroup() &&
      (_mode == cNew || _mode == cEdit))
  {

    QString sql="UPDATE cashrcpt SET cashrcpt_custgrp_id=<? value ('custgrp_id') ?> "
                "WHERE cashrcpt_number = <? value('number') ?>;";
    XSqlQuery query;
    MetaSQLQuery mql(sql);
    query = mql.toQuery(getParams());
    if (query.first())
    {
      sql="SELECT cashrcpt_id FROM cashrcpt WHERE cashrcpt_number=<? value('number') ?>;";
      XSqlQuery query2;
      MetaSQLQuery mql2(sql);
      query2 = mql2.toQuery(getParams());
      if (query2.first())
        _cashrcptid = query.value("cashrcpt_id").toInt();
    }
    if((ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Customer Group"),
                             query, __FILE__, __LINE__)))
      return;
  }
  else
    return;
}

enum SetResponse cashReceipt::set(const ParameterList &pParams)
{
  XSqlQuery cashet, d;
  XWidget::set(pParams);
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
      _transType = cNew;

      cashet.exec("SELECT fetchCashRcptNumber() AS number;");
      if (cashet.first())
        _number->setText(cashet.value("number").toString());
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipts Information"),
                                    cashet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      cashet.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
      if (cashet.first())
      {
        _cashrcptid = cashet.value("cashrcpt_id").toInt();
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipts Information"),
                                    cashet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      _applDate->setDate(omfgThis->dbDate(), true);
      _distDate->setDate(omfgThis->dbDate(), true);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _transType = cEdit;

      param = pParams.value("cashrcpt_id", &valid);
      if (valid)
      {
        _cashrcptid = param.toInt();
        QString sql = "SELECT * from cashrcpt WHERE cashrcpt_id = <? value('cashrcpt_id') ?>";
        MetaSQLQuery mql(sql);
        d = mql.toQuery(pParams);
        if (d.first())
        {
          if (d.value("cashrcpt_cust_id").toInt() > 0)
          {
             _customerSelector->setCustId(d.value("cashrcpt_cust_id").toInt());
          }
          else
          {
            _customerSelector->setCustGroupId(d.value("cashrcpt_custgrp_id").toInt());
          }
          _save->setEnabled(true);
        }
        _received->setEnabled(true);
        _fundsType->setEnabled(true);
        _docNumber->setEnabled(true);
        _docDate->setEnabled(true);
        _bankaccnt->setEnabled(true);
        _distDate->setEnabled(true);
        _applDate->setEnabled(true);
        activateButtons(true);
      }
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _transType = cView;

      param = pParams.value("cashrcpt_id", &valid);
      if (valid)
      {
        _cashrcptid = param.toInt();
        QString sql = "SELECT * from cashrcpt WHERE cashrcpt_id = <? value('cashrcpt_id') ?>";
        MetaSQLQuery mql(sql);
        d = mql.toQuery(pParams);
        if (d.first())
        {
          if (d.value("cashrcpt_cust_id").toInt() > 0)
          {
            _customerSelector->setCustId(d.value("cashrcpt_cust_id").toInt());
          }
          else
          {
            _customerSelector->setCustGroupId(d.value("cashrcpt_custgrp_id").toInt());
          }
        }
      }
      _received->setEnabled(false);
      _fundsType->setEnabled(false);
      _docNumber->setEnabled(false);
      _docDate->setEnabled(false);
      _bankaccnt->setEnabled(false);
      _distDate->setEnabled(false);
      _applDate->setEnabled(false);
      _aropen->setEnabled(false);
      _searchDocNum->setEnabled(false);
      _cashrcptmisc->setEnabled(false);
      _notes->setReadOnly(true);
      _applyToBalance->setEnabled(false);
      _add->setEnabled(false);
      _balCreditMemo->setEnabled(false);
      _balCustomerDeposit->setEnabled(false);
      _save->hide();
      _close->setText(tr("&Close"));
      _altAccnt->setEnabled(false);
      _newCC->setEnabled(false);
      _editCC->setEnabled(false);
      _customerSelector->setEnabled(false);
      _project->setEnabled(false);
      _postReceipt->setVisible(false);
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_cashrcptmisc, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    }

    // If this cash receipt was by an external processor transaction then prevent changes.
    XSqlQuery isExternalQry;
    bool isExternal = false;
    isExternalQry.prepare("SELECT isExternalFundsType(:fundstypecode) AS isexternal;");
    isExternalQry.bindValue(":fundstypecode", _origFunds);
    isExternalQry.exec();
    if (isExternalQry.first())
    {
      isExternal = isExternalQry.value("isexternal").toBool();
    }

    _ccEdit = (_mode == cEdit) && (isExternal);

    if(_ccEdit)
    {
      _received->setEnabled(false);
      _fundsType->setEnabled(false);
      _docNumber->setEnabled(false);
      _docDate->setEnabled(false);
      _bankaccnt->setEnabled(false);
      _distDate->setEnabled(false);
      _applDate->setEnabled(false);
    }

  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
      _customerSelector->setCustId(param.toInt());
  param = pParams.value("docnumber", & valid);
  if (valid)
    _searchDocNum->setText(param.toString());

  return NoError;
}


void cashReceipt::sApplyToBalance()
{

  if(!save(true))
    return;

  XSqlQuery applyToBal;

  applyToBal.prepare( "UPDATE cashrcpt "
             "SET cashrcpt_cust_id=:cust_id, "
             "    cashrcpt_curr_id=:curr_id, "
             "    cashrcpt_docdate=:docdate "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  _customerSelector->bindValue(applyToBal);
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.bindValue(":docdate", _docDate->date());
  applyToBal.exec();
  if((ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Cash Receipt To Balance"),
                           applyToBal, __FILE__, __LINE__)))
    return;

  applyToBal.prepare("SELECT applyCashReceiptToBalance(:cashrcpt_id, "
                     "             :amount, :curr_id, :inclCredits) AS result;");
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":amount", _received->localValue());
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.bindValue(":inclCredits", _credits->isChecked());
  applyToBal.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Cash Receipt To Balance"),
                       applyToBal, __FILE__, __LINE__);

  updateCustomerGroup();
  sFillApplyList();
}

void cashReceipt::sApply()
{

  if(_mode == cNew)
  {
    if(!save(true))
      return;
  }

  bool update  = false;
  QList<XTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    ParameterList params;

    if(cursor->altId() != -1)
    {
      params.append("mode", "edit");
      params.append("cashrcptitem_id", cursor->altId());
      params.append("amount_to_apply", _received->localValue());
    }
    else
    {
      params.append("mode", "new");
      params.append("amount_to_apply", _balance->localValue());
    }
    params.append("cashrcpt_id", _cashrcptid);
    params.append("aropen_id", cursor->id());
    params.append("curr_id", _received->id());

    cashReceiptItem newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      update = true;
  }

  updateCustomerGroup();

  if (update)
    sFillApplyList();
}

void cashReceipt::sApplyLineBalance()
{
  if(!save(true))
    return;

  XSqlQuery applyToBal;
  applyToBal.prepare( "UPDATE cashrcpt "
                      "SET cashrcpt_cust_id=:cust_id, "
                      "    cashrcpt_curr_id=:curr_id, "
                      "    cashrcpt_docdate=:docdate "
                      "WHERE (cashrcpt_id=:cashrcpt_id);" );

  _customerSelector->bindValue(applyToBal);
  applyToBal.bindValue(":cashrcpt_id", _cashrcptid);
  applyToBal.bindValue(":curr_id", _received->id());
  applyToBal.bindValue(":docdate", _docDate->date());
  applyToBal.exec();
  if((ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Cash Receipt To Line Balance"),
                           applyToBal, __FILE__, __LINE__)))
    return;

  XSqlQuery applyq;
  applyq.prepare("SELECT applycashreceiptlinebalance(:cashrcpt_id,"
                 "       :cashrcptitem_aropen_id,:amount,:curr_id) AS result;");
  applyq.bindValue(":cashrcpt_id", _cashrcptid);
  applyq.bindValue(":curr_id",     _received->id());

  // loop twice - first collect the credits, then apply them to debits
  // stop when we hit a hard error, then refresh the list to show what happened
  bool crediterr = false;
  foreach (XTreeWidgetItem *cursor, _aropen->selectedItems())
  {
    if (cursor->rawValue("doctype").toString() == "C" ||
        cursor->rawValue("doctype").toString() == "R")
    {
      double credit = _received->convert(cursor->rawValue("balance_curr").toInt(),
                                         _received->id(),
                                         cursor->rawValue("balance").toDouble(),
                                         _distDate->date());
      applyq.bindValue(":amount",                 credit);
      applyq.bindValue(":cashrcptitem_aropen_id", cursor->id());
      applyq.exec();
      if (applyq.first())
      {
        double result = applyq.value("result").toDouble();
        if (result < 0)
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Credits"),
                               tr("Could not apply %1 %2 to Cash Receipt (%3)")
                               .arg(cursor->text("doctype"),
                                    cursor->text("aropen_docnumber")).arg(result),
                               __FILE__, __LINE__);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Credits"),
                                    applyq, __FILE__, __LINE__))
      {
        crediterr = true;
        break;
      }
    }

  }

  if (! crediterr)
  {
    foreach (XTreeWidgetItem *cursor, _aropen->selectedItems())
    {
      if (cursor->rawValue("doctype").toString() == "I" ||
          cursor->rawValue("doctype").toString() == "D")
      {
        applyq.bindValue(":amount",                 _received->localValue());
        applyq.bindValue(":cashrcptitem_aropen_id", cursor->id());
        applyq.exec();
        if (applyq.first())
        {
          double result = applyq.value("result").toDouble();
          if (result < 0)
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Debits"),
                                 tr("Could not apply %1 %2 to Cash Receipt (%3)")
                                 .arg(cursor->text("doctype"),
                                      cursor->text("aropen_docnumber")).arg(result),
                                 __FILE__, __LINE__);
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Applying Debits"),
                                      applyq, __FILE__, __LINE__))
          break;
      }
    }
  }
  updateCustomerGroup();
  sFillApplyList();
}

void cashReceipt::sClear()
{

  XSqlQuery cashClear;
  QList<XTreeWidgetItem*> list = _aropen->selectedItems();
  XTreeWidgetItem *cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    cashClear.prepare( "DELETE FROM cashrcptitem "
               " WHERE ((cashrcptitem_aropen_id=:aropen_id) "
               " AND (cashrcptitem_cashrcpt_id=:cashrcpt_id));" );
    cashClear.bindValue(":cashrcpt_id", _cashrcptid);
    cashClear.bindValue(":aropen_id", cursor->id());
    cashClear.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Clearning Cash Receipt"),
                                  cashClear, __FILE__, __LINE__))
    {
      return;
    }
  }

  sUpdateBalance();
  updateCustomerGroup();
  sFillApplyList();
}

void cashReceipt::sAdd()
{
  if(_mode == cNew)
  {
    if (_customerSelector->isSelectedGroup())
      sApply();

    if(!save(true))
      return;
  }

  if (_customerSelector->isSelectedGroup())
    save(true);

  ParameterList params = getParams();
  params.append("mode", "new");

  cashReceiptMiscDistrib newdlg(this, "", true);

  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sEdit()
{
  ParameterList params = getParams();
  params.append("mode", "edit");

  cashReceiptMiscDistrib newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillMiscList();
}

void cashReceipt::sDelete()
{
  XSqlQuery cashDelete;
  cashDelete.prepare( "DELETE FROM cashrcptmisc "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  cashDelete.bindValue(":cashrcptmisc_id", _cashrcptmisc->id());
  cashDelete.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Miscellaneous Distribution"),
                                cashDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillMiscList();
}

void cashReceipt::close()
{
  XSqlQuery cashclose;
  if (_transType == cNew && _cashrcptid >= 0)
  {
    cashclose.prepare("SELECT deleteCashRcpt(:cashrcpt_id) AS result;");
    cashclose.bindValue(":cashrcpt_id", _cashrcptid);
    cashclose.exec();
    if (cashclose.first())
    {
      int result = cashclose.value("result").toInt();
      if (result < 0)
      {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cash Receipt"),
                               storedProcErrorLookup("deleteCashRcpt", result),
                               __FILE__, __LINE__);
          return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cash Receipt"),
                                  cashclose, __FILE__, __LINE__))
    {
      return;
    }

    cashclose.prepare("SELECT releaseCashRcptNumber(:number);");
    cashclose.bindValue(":number", _number->text().toInt());
    cashclose.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Cash Receipt"),
                                  cashclose, __FILE__, __LINE__))
    {
      return;
    }
  }

  XWidget::close();
}

void cashReceipt::sSave()
{
  if (_received->localValue() == 0.00 &&
      QMessageBox::question(this, tr("Invalid amount"),
                            tr("It appears that the amount received has not been specified"
                               " or is equal to zero. Do you want to save the current "
                               "cash receipt anyway?"),
                               QMessageBox::Yes,
                               QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

  if (save(false))
  {
    if (_postReceipt->isChecked() && !_posted)
    {    
      if (!postReceipt())
        return;
    }

    omfgThis->sCashReceiptsUpdated(_cashrcptid, true);
    _cashrcptid = -1;

    close();
  }

  updateCustomerGroup();
}

bool cashReceipt::save(bool partial)
{
  XSqlQuery cashave;
  if (_overapplied &&
      QMessageBox::question(this, tr("Overapplied?"),
                            tr("This Cash Receipt appears to apply too much to"
                               " at least one of the Open Items. Do you want "
                               "to save the current applications anyway?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return false;

  int _bankaccnt_curr_id = -1;
  QString _bankaccnt_currAbbr;
  cashave.prepare( "SELECT bankaccnt_curr_id, "
             "       currConcat(bankaccnt_curr_id) AS currAbbr "
             "  FROM bankaccnt "
             " WHERE (bankaccnt_id=:bankaccnt_id);");
  cashave.bindValue(":bankaccnt_id", _bankaccnt->id());
  cashave.exec();
  if (cashave.first())
  {
    _bankaccnt_curr_id = cashave.value("bankaccnt_curr_id").toInt();
    _bankaccnt_currAbbr = cashave.value("currAbbr").toString();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Cash Receipt"),
                                cashave, __FILE__, __LINE__))
  {
    return false;
  }

  if (_received->currencyEnabled() && _received->id() != _bankaccnt_curr_id &&
      QMessageBox::question(this, tr("Cash Receipt Transaction Not In Bank Currency"),
                          tr("<p>This transaction is specified in %1 while the "
                             "Bank Account is specified in %2. Do you wish to "
                             "convert at the current Exchange Rate?"
                             "<p>If not, click NO "
                             "and change the Bank Account in the POST TO field.")
                          .arg(_received->currAbbr())
                          .arg(_bankaccnt_currAbbr),
                          QMessageBox::Yes|QMessageBox::Escape,
                          QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
    _received->setFocus();
    return false;
  }
  _received->setCurrencyDisabled(true);

  if (!partial && _metrics->boolean("CCAccept") && _privileges->check("ProcessCreditCards"))
  {
    XSqlQuery isCreditCardQry;
    bool isCreditCard = false;
    isCreditCardQry.prepare("SELECT isCreditCardFundsType(:fundstypecode) AS iscreditcard;");
    isCreditCardQry.bindValue(":fundstypecode", _fundsType->code());
    isCreditCardQry.exec();
    if (isCreditCardQry.first())
    {
      isCreditCard = isCreditCardQry.value("iscreditcard").toBool();
    }

    if (!_ccEdit && isCreditCard)
    {
      if (_cc->id() <= -1)
      {
        QMessageBox::warning(this, tr("Select a Credit Card"),
                             tr("Please select a Credit Card from the list "
                                "before continuing."));
        _tab->setCurrentIndex(_tab->indexOf(_creditCardTab));
        _cc->setFocus();
        return false;
      }
      CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
      if (! cardproc)
      {
        QMessageBox::critical(this, tr("Credit Card Processing Error"),
                              CreditCardProcessor::errorMsg());
        return false;
      }

      _save->setEnabled(false);
      int ccpayid = -1;
      QString neworder = _docNumber->text().isEmpty() ?
                           QString::number(_cashrcptid) : _docNumber->text();
      QString reforder = neworder; // 2 sep variables because they're passed by ref
      int returnVal = cardproc->charge(_cc->id(),
                                       _CCCVV->text(),
                                       _received->localValue(),
                                       0, false, 0, 0,
                                       _received->id(),
                                       neworder, reforder, ccpayid,
                                       QString("cashrcpt"), _cashrcptid);
      if (returnVal < 0)
        QMessageBox::critical(this, tr("Credit Card Processing Error"),
                              cardproc->errorMsg());
      else if (returnVal > 0)
        QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                             cardproc->errorMsg());
      else if (! cardproc->errorMsg().isEmpty())
        QMessageBox::information(this, tr("Credit Card Processing Note"),
                                 cardproc->errorMsg());

      _save->setEnabled(true);
      if (returnVal < 0)
        return false;
    }
  }

  if (!_distDate->isValid())
    _distDate->setDate(omfgThis->dbDate());

  QString sql;
  if (_mode == cNew)
    sql = "INSERT INTO cashrcpt "
                    "( cashrcpt_id, cashrcpt_cust_id, cashrcpt_custgrp_id, cashrcpt_distdate, cashrcpt_amount,"
                    "  cashrcpt_fundstype, cashrcpt_bankaccnt_id, cashrcpt_curr_id, "
                    "  cashrcpt_usecustdeposit, cashrcpt_docnumber, cashrcpt_docdate, "
                    "  cashrcpt_notes, cashrcpt_salescat_id, cashrcpt_number, cashrcpt_applydate, "
                    "  cashrcpt_discount, cashrcpt_alt_curr_rate, cashrcpt_prj_id ) "
                    "VALUES "
                    "( :cashrcpt_id, :cashrcpt_cust_id, :cashrcpt_custgrp_id, :cashrcpt_distdate, :cashrcpt_amount,"
                    "  :cashrcpt_fundstype, :cashrcpt_bankaccnt_id, :curr_id, "
                    "  :cashrcpt_usecustdeposit, :cashrcpt_docnumber, :cashrcpt_docdate, "
                    "  :cashrcpt_notes, :cashrcpt_salescat_id, :cashrcpt_number, :cashrcpt_applydate, "
                    "  :cashrcpt_discount, ROUND(:cashrcpt_alt_curr_rate, 8), :cashrcpt_prj_id );";
  else
    sql= "UPDATE cashrcpt "
                    "SET cashrcpt_cust_id=:cashrcpt_cust_id,"
                    "    cashrcpt_custgrp_id=:cashrcpt_custgrp_id,"
                    "    cashrcpt_amount=:cashrcpt_amount,"
                    "    cashrcpt_fundstype=:cashrcpt_fundstype,"
                    "    cashrcpt_docnumber=:cashrcpt_docnumber,"
                    "    cashrcpt_docdate=:cashrcpt_docdate,"
                    "    cashrcpt_bankaccnt_id=:cashrcpt_bankaccnt_id,"
                    "    cashrcpt_distdate=:cashrcpt_distdate,"
                    "    cashrcpt_notes=:cashrcpt_notes, "
                    "    cashrcpt_salescat_id=:cashrcpt_salescat_id, "
                    "    cashrcpt_curr_id=:curr_id,"
                    "    cashrcpt_usecustdeposit=:cashrcpt_usecustdeposit,"
                    "    cashrcpt_applydate=:cashrcpt_applydate,"
                    "    cashrcpt_discount=:cashrcpt_discount, "
                    "    cashrcpt_alt_curr_rate= ROUND(:cashrcpt_alt_curr_rate, 8), "
                    "    cashrcpt_prj_id=:cashrcpt_prj_id, "
                    "    cashrcpt_curr_rate=null " // force a curr rate re-evaluation
                    "WHERE (cashrcpt_id=:cashrcpt_id);";

  cashave.prepare(sql);
  cashave.bindValue(":cashrcpt_id", _cashrcptid);
  cashave.bindValue(":cashrcpt_number", _number->text());
  if (_customerSelector->isSelectedGroup())
    cashave.bindValue(":cashrcpt_custgrp_id", _customerSelector->custGroupId());
  else if (_customerSelector->isSelectedCust())
    cashave.bindValue(":cashrcpt_cust_id", _customerSelector->custId());
  cashave.bindValue(":cashrcpt_amount", _received->localValue());
  cashave.bindValue(":cashrcpt_fundstype", _fundsType->code());
  cashave.bindValue(":cashrcpt_docnumber", _docNumber->text());
  cashave.bindValue(":cashrcpt_docdate", _docDate->date());
  cashave.bindValue(":cashrcpt_bankaccnt_id", _bankaccnt->id());
  cashave.bindValue(":cashrcpt_distdate", _distDate->date());
  cashave.bindValue(":cashrcpt_applydate", _applDate->date());
  cashave.bindValue(":cashrcpt_notes",          _notes->toPlainText().trimmed());
  cashave.bindValue(":cashrcpt_usecustdeposit", QVariant(_balCustomerDeposit->isChecked()));
  cashave.bindValue(":cashrcpt_discount", _discount->localValue());
  cashave.bindValue(":curr_id", _received->id());
  if(_altAccnt->isChecked())
    cashave.bindValue(":cashrcpt_salescat_id", _salescat->id());
  else
    cashave.bindValue(":cashrcpt_salescat_id", -1);
  if(_altExchRate->isChecked())
  {
    if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
      cashave.bindValue(":cashrcpt_alt_curr_rate", 1.0 / _exchRate->toDouble());
    else
      cashave.bindValue(":cashrcpt_alt_curr_rate", _exchRate->toDouble());
  }
  if(_project->isValid())
    cashave.bindValue(":cashrcpt_prj_id", _project->id());
  cashave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Cash Receipt"),
                                cashave, __FILE__, __LINE__))
  {
    return false;
  }
  _mode=cEdit;
  return true;
}

bool cashReceipt::postReceipt()
{
  XSqlQuery cashPost;
  bool changeDate = false;
  QDate newDate = QDate();
  QDate seriesDate;
  
  if (_privileges->check("ChangeCashRecvPostDate"))
  {
    getGLDistDate newdlg(this, "", true);
    newdlg.sSetDefaultLit(tr("Distribution Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
      seriesDate = newdlg.seriesDate();
    }
    else
      return false;
  }
  
  XSqlQuery tx;
  tx.exec("BEGIN;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE cashrcpt SET cashrcpt_distdate=:distdate,"
                  "                    cashrcpt_applydate=CASE WHEN (cashrcpt_applydate < :distdate) THEN :distdate"
                  "                                            ELSE cashrcpt_applydate END "
                  "WHERE cashrcpt_id=:cashrcpt_id;");
   
  if (changeDate)
  {
    setDate.bindValue(":distdate",    newDate);
    setDate.bindValue(":cashrcpt_id", _cashrcptid);
    setDate.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Changing Dist. Date"),
                         setDate, __FILE__, __LINE__))
    {
      tx.exec("ROLLBACK");
      return false;
    }
  }
  
  cashPost.prepare("SELECT postCashReceipt(:cashrcpt_id, fetchJournalNumber('C/R', :seriesDate)) AS result;");
  cashPost.bindValue(":cashrcpt_id", _cashrcptid);
  cashPost.bindValue(":seriesDate", seriesDate);
  cashPost.exec();
  if (cashPost.first())
  {
    int result = cashPost.value("result").toInt();
    if (result < 0)
    {
      tx.exec("ROLLBACK;");
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt Entry"),
                             storedProcErrorLookup("postCashReceipt", result),
                             __FILE__, __LINE__);
      return false;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Cash Receipt Entry"),
                                cashPost, __FILE__, __LINE__))
  {
    tx.exec("ROLLBACK");
    return false;
  }

  tx.exec("COMMIT;");
  return true;
}

void cashReceipt::sFillApplyList()
{
  // Customer Group selected
  if (_customerSelector->isSelectedGroup())
  {
      _aropen->clear();
      grpFillApplyList();
      if (_mode == cNew)
        activateButtons();
  }
  // Valid Customer selected
  if (_customerSelector->isSelectedCust() && _customerSelector->isValid())
  {
    if (_mode == cNew)
      activateButtons();
    _aropen->clear();
    MetaSQLQuery mql = mqlLoad("arOpenApplications", "detail");
    ParameterList params = getParams();
    if (_posted)
      params.append("posted", true);

    XSqlQuery apply;
    apply = mql.toQuery(params);
    _aropen->populate(apply, true);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                  apply, __FILE__, __LINE__))
    {
      return;
    }

    _overapplied = false;
    XTreeWidgetItem *last;
    for (int i = 0; i < _aropen->topLevelItemCount(); i++)
    {
      last = _aropen->topLevelItem(i);
      if (last->data(9, Qt::UserRole).toMap().value("raw").toDouble() >
          last->data(5, Qt::UserRole).toMap().value("raw").toDouble())
      {
        _overapplied = true;
        break;
      }
    }

    apply.prepare( "SELECT SUM(COALESCE(cashrcptitem_amount, 0)) AS total "
               "FROM cashrcptitem "
               "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id);" );
    apply.bindValue(":cashrcpt_id", _cashrcptid);
    apply.exec();
    if (apply.first())
      _applied->setLocalValue(apply.value("total").toDouble());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                  apply, __FILE__, __LINE__))
    {
      return;
    }

    apply.prepare("select max(aropen_docdate) AS mindate FROM aropen, cashrcptitem WHERE cashrcptitem_aropen_id=aropen_id AND cashrcptitem_cashrcpt_id=:id;");
    apply.bindValue(":id", _cashrcptid);
    apply.exec();
    if(apply.first())
    {
      _mindate = apply.value("mindate").toDate();
      if(_mindate > _applDate->date())
        _applDate->setDate(_mindate);
    }

    XSqlQuery discount ;

    discount.prepare( "SELECT SUM(COALESCE(cashrcptitem_discount, 0.00)) AS disc "
                      "FROM cashrcptitem "
                      "WHERE (cashrcptitem_cashrcpt_id=:cashrcpt_id);" );
    discount.bindValue(":cashrcpt_id", _cashrcptid);
    discount.exec();
    if (discount.first())
    {
      _discount->setLocalValue(discount.value("disc").toDouble());
      sUpdateBalance();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                  discount, __FILE__, __LINE__))
    {
      return;
    }
  }
  // Invalid Customer selected
  if (_customerSelector->isSelectedCust() && !_customerSelector->isValid())
  {
    _aropen->clear();
  }

  _received->setCurrencyEditable(_applied->isZero() && _miscDistribs->isZero());

}

void cashReceipt::sFillMiscList()
{
  XSqlQuery misc;
  misc.prepare("SELECT cashrcptmisc_id,"
               "       COALESCE(tax_code||' - '||tax_descrip, "
               "          formatGLAccount(cashrcptmisc_accnt_id)) AS account,"
               "       firstLine(cashrcptmisc_notes) AS firstline, "
               "       cashrcptmisc_amount,"
               "       'curr' AS cashrcptmisc_amount_xtnumericrole "
               "FROM cashrcptmisc "
               " LEFT OUTER JOIN tax ON (tax_id=cashrcptmisc_tax_id) "
               "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  _cashrcptmisc->populate(misc);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipts Information"),
                                misc, __FILE__, __LINE__))
  {
    return;
  }

  misc.prepare( "SELECT SUM(cashrcptmisc_amount) AS total "
             "FROM cashrcptmisc "
             "WHERE (cashrcptmisc_cashrcpt_id=:cashrcpt_id);" );
  misc.bindValue(":cashrcpt_id", _cashrcptid);
  misc.exec();
  if (misc.first())
    _miscDistribs->setLocalValue(misc.value("total").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                misc, __FILE__, __LINE__))
  {
    return;
  }
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
  else
    _balance->setPaletteForegroundColor(QColor("black"));
}

void cashReceipt::populate()
{
  XSqlQuery cashpopulate;
  cashpopulate.prepare( "SELECT *,"
             "       COALESCE(cashrcpt_applydate, cashrcpt_distdate) AS applydate "
             "FROM cashrcpt "
             "WHERE (cashrcpt_id=:cashrcpt_id);" );
  cashpopulate.bindValue(":cashrcpt_id", _cashrcptid);

  cashpopulate.exec();
  if (cashpopulate.first())
  {
    _customerSelector->setCustId(cashpopulate.value("cashrcpt_cust_id").toInt());
    _number->setText(cashpopulate.value("cashrcpt_number").toString());
    _received->set(cashpopulate.value("cashrcpt_amount").toDouble(),
                   cashpopulate.value("cashrcpt_curr_id").toInt(),
                   cashpopulate.value("cashrcpt_distdate").toDate(), false);
    _docNumber->setText(cashpopulate.value("cashrcpt_docnumber").toString());
    _docDate->setDate(cashpopulate.value("cashrcpt_docdate").toDate(), true);
    _bankaccnt->setId(cashpopulate.value("cashrcpt_bankaccnt_id").toInt());
    _received->setCurrencyDisabled(true);
    _applDate->setDate(cashpopulate.value("applydate").toDate(), true);
    _distDate->setDate(cashpopulate.value("cashrcpt_distdate").toDate(), true);
    _notes->setText(cashpopulate.value("cashrcpt_notes").toString());
    _posted = cashpopulate.value("cashrcpt_posted").toBool();
    if(cashpopulate.value("cashrcpt_salescat_id").toInt() != -1)
    {
      _altAccnt->setChecked(true);
      _salescat->setId(cashpopulate.value("cashrcpt_salescat_id").toInt());
    }
    if(cashpopulate.value("cashrcpt_alt_curr_rate").toDouble() > 0.0)
    {
      _altExchRate->setChecked(true);
      if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
        _exchRate->setDouble(1.0 / cashpopulate.value("cashrcpt_alt_curr_rate").toDouble());
      else
        _exchRate->setDouble(cashpopulate.value("cashrcpt_alt_curr_rate").toDouble());
      sUpdateGainLoss();
    }
    if(cashpopulate.value("cashrcpt_usecustdeposit").toBool())
      _balCustomerDeposit->setChecked(true);
    else
      _balCreditMemo->setChecked(true);

    if(cashpopulate.value("cashrcpt_posted").toBool())
      _postReceipt->setVisible(false);

    _origFunds = cashpopulate.value("cashrcpt_fundstype").toString();
    _fundsType->setCode(_origFunds);
    _project->setId(cashpopulate.value("cashrcpt_prj_id").toInt());

    sFillApplyList();
    sFillMiscList();
    setCreditCard();

    emit populated();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Cash Receipt Information"),
                                cashpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void cashReceipt::sSearchDocNumChanged()
{
  QString sub = _searchDocNum->text().trimmed();
  if(sub.isEmpty())
    return;

  QList<XTreeWidgetItem*> list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchCaseSensitive, 1);
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
                "SET cashrcpt_curr_id=:curr_id, "
                "    cashrcpt_curr_rate=null "
                "WHERE (cashrcpt_id=:cashrcpt_id);" );
    id.bindValue(":cashrcpt_id", _cashrcptid);

    id.bindValue(":curr_id", newId);
    id.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Switching Currencies"),
                         id, __FILE__, __LINE__);
  }
}

void cashReceipt::setCreditCard()
{
  XSqlQuery cashetCreditCard;
  if (! _metrics->boolean("CCAccept"))
    return;

  XSqlQuery isCreditCardQry;
  isCreditCardQry.prepare("SELECT isCreditCardFundsType(:fundstypecode) AS iscreditcard;");
  isCreditCardQry.bindValue(":fundstypecode", _fundsType->code());
  isCreditCardQry.exec();
  if (isCreditCardQry.first())
  {
    if (!isCreditCardQry.value("iscreditcard").toBool())
      return;
  }

  XSqlQuery bankq;
  bankq.prepare("SELECT COALESCE(ccbank_bankaccnt_id, -1) AS ccbank_bankaccnt_id"
                "  FROM ccbank"
                " WHERE (ccbank_ccard_type=:cardtype);");
  bankq.bindValue(":cardtype", _fundsType->code());
  bankq.exec();
  if (bankq.first())
    if (bankq.value("ccbank_bankaccnt_id").toInt() > 0)
      _bankaccnt->setId(bankq.value("ccbank_bankaccnt_id").toInt());
    else
    {
      QMessageBox::warning(this, tr("No Bank Account"),
                           tr("<p>Cannot find the Bank Account to post this "
                              "transaction against. Either this card type is not "
                              "accepted or the Credit Card configuration is not "
                              "complete."));
      return;
    }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving CC Data"),
                                bankq, __FILE__, __LINE__))
  {
    return;
  }
  else
  {
    QMessageBox::warning(this, tr("No Bank Account"),
                         tr("<p>Cannot find the Bank Account to post this "
                         "transaction against. Either this card type is not "
                         "accepted or the Credit Card configuration is not "
                         "complete."));
    return;
  }

  cashetCreditCard.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key)) AS result;");
  _customerSelector->bindValue(cashetCreditCard);
  cashetCreditCard.bindValue(":key", omfgThis->_key);
  cashetCreditCard.exec();
  if (cashetCreditCard.first())
  {
    int result = cashetCreditCard.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving CC Data"),
                           storedProcErrorLookup("expireCreditCard", result),
                           __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving CC Data"),
                                cashetCreditCard, __FILE__, __LINE__))
  {
    return;
  }

  MetaSQLQuery mql = mqlLoad("creditCards", "detail");
  ParameterList params;
  params.append("cust_id", _customerSelector->custId());
  params.append("ccard_type", _fundsType->code());
  params.append("masterCard", tr("MasterCard"));
  params.append("visa",       tr("VISA"));
  params.append("americanExpress", tr("American Express"));
  params.append("discover",   tr("Discover"));
  params.append("other",      tr("Other"));
  params.append("key",        omfgThis->_key);
  params.append("activeonly", true);
  cashetCreditCard = mql.toQuery(params);
  _cc->populate(cashetCreditCard);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving CC Data"),
                                cashetCreditCard, __FILE__, __LINE__))
  {
    return;
  }
  if (_cc->topLevelItemCount() == 1)
    _cc->setCurrentItem(_cc->topLevelItem(0));
}

void cashReceipt::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  _customerSelector->appendValue(params);

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    setCreditCard();

}

void cashReceipt::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  _customerSelector->appendValue(params);
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    setCreditCard();
}

void cashReceipt::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  _customerSelector->appendValue(params);
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void cashReceipt::sMoveUp()
{
  XSqlQuery cashMoveUp;
  cashMoveUp.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  cashMoveUp.bindValue(":ccard_id", _cc->id());
  cashMoveUp.exec();

  setCreditCard();
}

void cashReceipt::sMoveDown()
{
  XSqlQuery cashMoveDown;
  cashMoveDown.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  cashMoveDown.bindValue(":ccard_id", _cc->id());
  cashMoveDown.exec();

  setCreditCard();
}

void cashReceipt::sDateChanged()
{
  if(_applDate->date() < _mindate)
    _applDate->setDate(_mindate);

  if(_distDate->date() > _applDate->date())
    _distDate->setDate(_applDate->date());
  else if(_distDate->date() < _applDate->date())
    _applyBalLit->setText(tr("Record Receipt as:"));
  else
    _applyBalLit->setText(tr("Apply Balance As:"));
}

void cashReceipt::sHandleAltExchRate()
{
  if (_altExchRate->isChecked())
  {
    QString inverter("");
    if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
      inverter = "1 / ";
    XSqlQuery currRate;
    QString sql = QString("SELECT %1 currRate(:cashrcpt_curr_id, :cashrcpt_distdate) AS _currrate;")
                          .arg(inverter);
    currRate.prepare(sql);
    currRate.bindValue(":cashrcpt_curr_id", _received->id());
    currRate.bindValue(":cashrcpt_distdate", _applDate->date());
    currRate.exec();
    if (currRate.first())
    {
      _exchRate->setDouble(currRate.value("_currrate").toDouble());
      _gainLoss->setBaseValue(0.0);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Alt Exch Rate Data"),
                                  currRate, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    _exchRate->clear();
    _gainLoss->clear();
  }
}

void cashReceipt::sUpdateGainLoss()
{
  if (_altExchRate->isChecked())
  {
    XSqlQuery gainLoss;
    QString sql = QString("SELECT ROUND((:cashrcpt_amount / ROUND(:cashrcpt_alt_curr_rate, 8)) - "
                          "             (:cashrcpt_amount / currRate(:cashrcpt_curr_id, :cashrcpt_distdate))"
                          "             , 2) AS gainloss;");
    gainLoss.prepare(sql);
    gainLoss.bindValue(":cashrcpt_curr_id", _received->id());
    gainLoss.bindValue(":cashrcpt_distdate", _applDate->date());
    gainLoss.bindValue(":cashrcpt_amount", _received->localValue());
    if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
      gainLoss.bindValue(":cashrcpt_alt_curr_rate", 1.0 / _exchRate->toDouble());
    else
      gainLoss.bindValue(":cashrcpt_alt_curr_rate", _exchRate->toDouble());
    gainLoss.exec();
    if (gainLoss.first())
    {
      _gainLoss->setBaseValue(gainLoss.value("gainloss").toDouble());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Setting Alt Exch Rate"),
                                  gainLoss, __FILE__, __LINE__))
    {
      return;
    }
  }
}
