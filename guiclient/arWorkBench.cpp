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

#include "arWorkBench.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <stdlib.h>
#include <metasql.h>
#include "mqlutil.h"

#include "arOpenItem.h"
#include "applyARCreditMemo.h"
#include "cashReceipt.h"
#include "creditMemo.h"
#include "creditcardprocessor.h"
#include "dspInvoiceInformation.h"
#include "incident.h"
#include "invoice.h"
#include "storedProcErrorLookup.h"

arWorkBench::arWorkBench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_viewAropen, SIGNAL(clicked()), this, SLOT(sViewAropen()));
  connect(_editAropen, SIGNAL(clicked()), this, SLOT(sEditAropen()));
  connect(_applyAropenCM, SIGNAL(clicked()), this, SLOT(sApplyAropenCM()));
  connect(_editAropenCM, SIGNAL(clicked()), this, SLOT(sEditAropenCM()));
  connect(_viewAropenCM, SIGNAL(clicked()), this, SLOT(sViewAropenCM()));
  connect(_ccRefundCM,   SIGNAL(clicked()), this, SLOT(sCCRefundCM()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_newCashrcpt, SIGNAL(clicked()), this, SLOT(sNewCashrcpt()));
  connect(_editCashrcpt, SIGNAL(clicked()), this, SLOT(sEditCashrcpt()));
  connect(_viewCashrcpt, SIGNAL(clicked()), this, SLOT(sViewCashrcpt()));
  connect(_deleteCashrcpt, SIGNAL(clicked()), this, SLOT(sDeleteCashrcpt()));
  connect(_postCashrcpt, SIGNAL(clicked()), this, SLOT(sPostCashrcpt()));
  connect(_preauth, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(sgetCCAmount()));
  connect(_postPreauth, SIGNAL(clicked()), this, SLOT(sPostPreauth()));
  connect(_voidPreauth, SIGNAL(clicked()), this, SLOT(sVoidPreauth()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateAropenMenu(QMenu*)));
  connect(_aropenCM, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateAropenCMMenu(QMenu*)));
  connect(_cashrcpt, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateCashRcptMenu(QMenu*)));
  connect(_preauth, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulatePreauthMenu(QMenu*)));
  connect(_select, SIGNAL(currentIndexChanged(int)), this, SLOT(sClear()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sClear()));
  connect(_customerTypes, SIGNAL(currentIndexChanged(int)), this, SLOT(sClear()));
  connect(_customerType, SIGNAL(textChanged(QString)), this, SLOT(sClear()));
  connect(_searchDocNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged()));
  connect(_aropen, SIGNAL(valid(bool)), this, SLOT(sPopulateAropenButtonMenu()));
  
  _aropen->addColumn( tr("Cust/Incdt"),  _bigMoneyColumn, Qt::AlignLeft,  true,  "cust_number");                                                                
  _aropen->addColumn( tr("Name/Desc."),               -1, Qt::AlignLeft,  true,  "cust_name");    
  _aropen->addColumn(tr("Type"),            _orderColumn, Qt::AlignLeft,  true,  "doctype");
  _aropen->addColumn(tr("Doc. #"),          _orderColumn, Qt::AlignRight, true,  "aropen_docnumber");
  _aropen->addColumn(tr("Order/Assign"),              90, Qt::AlignRight, true,  "aropen_ordernumber");
  _aropen->addColumn(tr("Doc. Date"),        _dateColumn, Qt::AlignCenter,true,  "aropen_docdate");
  _aropen->addColumn(tr("Due Date"),         _dateColumn, Qt::AlignCenter,true,  "aropen_duedate");
  _aropen->addColumn(tr("Amount"),          _moneyColumn, Qt::AlignRight, false, "aropen_amount");
  _aropen->addColumn(tr("Paid"),            _moneyColumn, Qt::AlignRight, false, "aropen_paid");
  _aropen->addColumn(tr("Balance"),         _moneyColumn, Qt::AlignRight, true,  "balance");
  _aropen->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,  true,  "currAbbr");
  _aropen->addColumn(tr("Base Balance"), _bigMoneyColumn, Qt::AlignRight, true,  "base_balance");
  
  _aropenCM->addColumn( tr("Cust. #"),     _bigMoneyColumn, Qt::AlignLeft,  true, "cust_number");                                                                
  _aropenCM->addColumn( tr("Name"),                     -1, Qt::AlignLeft,  true, "cust_name");                                                                                                                                                                                                                  
  _aropenCM->addColumn( tr("Type"),           _orderColumn, Qt::AlignLeft,  true, "doctype");
  _aropenCM->addColumn( tr("Doc. #"),         _orderColumn, Qt::AlignCenter,true, "aropen_docnumber");
  _aropenCM->addColumn( tr("Amount"),         _moneyColumn, Qt::AlignRight, true, "aropen_amount");
  _aropenCM->addColumn( tr("Applied"),        _moneyColumn, Qt::AlignRight, true, "applied");
  _aropenCM->addColumn( tr("Balance"),        _moneyColumn, Qt::AlignRight, true, "balance");
  _aropenCM->addColumn( tr("Currency"),    _currencyColumn, Qt::AlignLeft,  true, "currabbr");
  _aropenCM->addColumn(tr("Base Balance"), _bigMoneyColumn, Qt::AlignRight, true, "base_balance");

  _cashrcpt->addColumn(tr("Cust. #"),       _bigMoneyColumn, Qt::AlignLeft,  true, "cust_number");                                                                
  _cashrcpt->addColumn(tr("Name"),                       -1, Qt::AlignLeft,  true, "cust_name"); 
  _cashrcpt->addColumn(tr("Check/Doc. #"),     _orderColumn, Qt::AlignLeft,  true, "cashrcpt_docnumber");
  _cashrcpt->addColumn(tr("Bank Account"),     _orderColumn, Qt::AlignLeft,  true, "bankaccnt_name");
  _cashrcpt->addColumn(tr("Dist. Date"),        _dateColumn, Qt::AlignCenter,true, "cashrcpt_distdate");
  _cashrcpt->addColumn(tr("Funds Type"),    _bigMoneyColumn, Qt::AlignCenter,true, "cashrcpt_fundstype");
  _cashrcpt->addColumn(tr("Amount"),        _bigMoneyColumn, Qt::AlignRight, true, "cashrcpt_amount");
  _cashrcpt->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,  true, "currabbr");
  
  _preauth->addColumn(tr("Cust. #"), _bigMoneyColumn, Qt::AlignLeft,  true, "cust_number");                                                                
  _preauth->addColumn(tr("Name"),                 -1, Qt::AlignLeft,  true, "cust_name");
  _preauth->addColumn(tr("Order-Seq."),           -1, Qt::AlignRight, true, "ordnum" );
  _preauth->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight, true, "ccpay_amount");
  _preauth->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");
  
  if (_privileges->check("MaintainCashReceipts"))
  {
    connect(_cashrcpt, SIGNAL(valid(bool)), _editCashrcpt, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _deleteCashrcpt, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _postCashrcpt, SLOT(setEnabled(bool)));
  }
  else
  {
    _newCashrcpt->setEnabled(FALSE);
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _viewCashrcpt, SLOT(animateClick()));
  }
  
  if(_privileges->check("PostCashReceipts"))
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _editCashrcpt, SLOT(animateClick()));
  connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(sFillList()));

  if(_privileges->check("EditAROpenItem"))
  {
    connect(_aropen, SIGNAL(valid(bool)), _editAropen, SLOT(setEnabled(bool)));
    connect(_aropen, SIGNAL(itemSelected(int)), _editAropen, SLOT(animateClick()));
    connect(_aropenCM, SIGNAL(valid(bool)), _editAropenCM, SLOT(setEnabled(bool)));
    connect(_aropenCM, SIGNAL(itemSelected(int)), _editAropenCM, SLOT(animateClick()));
  }
  else
  {
    connect(_aropen, SIGNAL(itemSelected(int)), _viewAropen, SLOT(animateClick()));
    connect(_aropenCM, SIGNAL(itemSelected(int)), _viewAropenCM, SLOT(animateClick()));
  }
  if (_privileges->check("ApplyARMemos"))
    connect(_aropenCM, SIGNAL(valid(bool)), _applyAropenCM, SLOT(setEnabled(bool)));

  if (omfgThis->singleCurrency())
  {
    _cashrcpt->hideColumn(2);
    _preauth->hideColumn(2);
    _aropen->hideColumn(8);
    _aropenCM->hideColumn(4);
  }

  if (_metrics->boolean("CCAccept") && _privileges->check("ProcessCreditCards"))
  {
    connect(_aropenCM, SIGNAL(valid(bool)), _ccRefundCM, SLOT(setEnabled(bool)));
    if (_metrics->value("CCValidDays").toInt())
      _validDays->setValue(_metrics->value("CCValidDays").toInt());
    else
      _validDays->setValue(7);
  }
  else
  {
    _tab->removeTab(_tab->indexOf(_creditCardTab));
    _ccRefundCM->hide();
  }
}

arWorkBench::~arWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void arWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arWorkBench::set( const ParameterList & pParams )
{
  QVariant param;
  bool    valid;
  
  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _select->setCurrentIndex(1);
    _cust->setId(param.toInt());
    sFillList();
  }

  return NoError;
}

bool arWorkBench::setParams(ParameterList &params)
{
  if (_select->currentIndex()==1)
    params.append("cust_id", _cust->id());
  else if (_select->currentIndex()==2)
    params.append("custtype_id", _customerTypes->id());
  else if (_select->currentIndex()==3)
    params.append("custtype_pattern", _customerType->text());
  
  params.append("invoice", tr("Invoice"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("cashdeposit", tr("Customer Deposit"));
  params.append("other", tr("Other"));
  params.append("orderByDocDate");
  
  return true;
}


void arWorkBench::sFillList()
{
  _CCAmount->clear();

  sFillAropenList();
  sFillAropenCMList();
  sFillCashrcptList();
  sFillPreauthList();
}

void arWorkBench::sClear()
{
  _aropenTotal->clear();
  _aropen->clear();
  _aropenCM->clear();
  _cashrcpt->clear();
  _preauth->clear();
}

void arWorkBench::sFillAropenList()
{
  MetaSQLQuery mql = mqlLoad("arOpenItems", "detail");
  ParameterList params;
  setParams(params);
  if (_selectDate->currentIndex()==1)
    params.append("endDueDate", _onOrBeforeDate->date());
  else if (_selectDate->currentIndex()==2)
  {
    params.append("startDueDate", _startDate->date());
    params.append("endDueDate", _endDate->date());
  }
  params.append("debitsOnly");
  q = mql.toQuery(params);
  _aropen->populate(q, true);
  
  params.append("totalOnly");
  q = mql.toQuery(params);
  if(q.first())
  {
    _aropenTotal->setBaseValue(q.value("total_balance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void arWorkBench::sFillAropenCMList()
{
  MetaSQLQuery mql = mqlLoad("arOpenItems", "detail");
  ParameterList params;
  setParams(params);
  params.append("creditsOnly");
  q = mql.toQuery(params);
  _aropenCM->populate(q, true);
}

void arWorkBench::sFillCashrcptList()
{
  MetaSQLQuery mql = mqlLoad("unpostedCashReceipts", "detail");
  ParameterList params;
  setParams(params);
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("other", tr("Other"));
  q = mql.toQuery(params);
  _cashrcpt->populate(q);
}

void arWorkBench::sFillPreauthList()
{    
  MetaSQLQuery mql = mqlLoad("preauthCreditCard", "detail");
  ParameterList params;
  setParams(params);
  if (!_showExpired->isChecked())
    params.append("validOnly");
  params.append("ccValidDays", _validDays->value());
  q = mql.toQuery(params);
  _preauth->populate(q);
}

void arWorkBench::sEditAropen()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void arWorkBench::sViewAropen()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void arWorkBench::sViewInvoice()
{
  ParameterList params;
  XTreeWidgetItem * item = static_cast<XTreeWidgetItem*>(_aropen->currentItem());
  params.append("invoiceNumber", item->text(3));
  params.append("mode", "view");
  dspInvoiceInformation * newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sViewInvoiceDetails()
{
  q.prepare("SELECT invchead_id FROM aropen, invchead WHERE ((aropen_id=:aropen_id) AND (invchead_invcnumber=aropen_docnumber));");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("invchead_id", q.value("invchead_id"));
    params.append("mode", "view");
    invoice* newdlg = new invoice();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void arWorkBench::sEditAropenOnlyCM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropenCM->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void arWorkBench::sViewAropenOnlyCM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropenCM->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void arWorkBench::sEditAropenCM()
{
  ParameterList params;
  params.append("mode", "edit");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT 2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype IN ('C','R')) "
            ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _aropenCM->currentItem()->text(3));
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
        sFillList();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
                             tr("<p>The Credit Memo #%1 could not be found.")
                             .arg(_aropenCM->currentItem()->text(1)));
    return;
  }
}

void arWorkBench::sViewAropenCM()
{
  ParameterList params;
  params.append("mode", "view");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
            "FROM cmhead "
            "WHERE (cmhead_number=:docnum) "
            "UNION "
            "SELECT 2 AS type, aropen_id AS id "
            "FROM aropen "
            "WHERE ((aropen_docnumber=:docnum)"
            "  AND (aropen_doctype IN ('C','R')) "
            ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", _aropenCM->currentItem()->text(3));
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
        sFillList();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
                             tr("<p>The Credit Memo #%1 could not be found.")
                             .arg(_aropenCM->currentItem()->text(1)));
    return;
  }
}

void arWorkBench::sApplyAropenCM()
{
  ParameterList params;
  params.append("aropen_id", _aropenCM->id());

  applyARCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void arWorkBench::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  if (_tab->currentIndex()==_tab->indexOf(_receivablesTab) &&
      _aropen->id() > -1)
  {
    q.prepare("SELECT aropen_cust_id FROM aropen WHERE aropen_id=:aropen_id;");
    q.bindValue(":aropen_id", _aropen->id());
    q.exec();
    if (q.first())
    {
      params.append("cust_id", q.value("aropen_cust_id").toInt());
      XTreeWidgetItem * item = static_cast<XTreeWidgetItem*>(_aropen->currentItem());
      params.append("docnumber", item->text(3));
    }
  }
  else
  {
    if (_select->currentIndex()==1 && _cust->isValid())
      params.append("cust_id", _cust->id());
  }

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sEditCashrcpt()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sViewCashrcpt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sDeleteCashrcpt()
{
  q.prepare("SELECT deleteCashrcpt(:cashrcpt_id) AS result;");
  q.bindValue(":cashrcpt_id", _cashrcpt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCashrcpt", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillCashrcptList();
}

void arWorkBench::sPostCashrcpt()
{
  int journalNumber = -1;

  XSqlQuery tx;
  tx.exec("BEGIN;");
  q.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (q.first())
    journalNumber = q.value("journalnumber").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QList<QTreeWidgetItem*> selected = _cashrcpt->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
    q.bindValue(":cashrcpt_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.bindValue(":journalNumber", journalNumber);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("postCashReceipt", result),
                    __FILE__, __LINE__);
        tx.exec("ROLLBACK;");
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      tx.exec("ROLLBACK;");
      return;
    }
  }
  tx.exec("COMMIT;");
  sFillList();
}

void arWorkBench::sgetCCAmount()
{
  q.prepare("SELECT ccpay_amount, ccpay_curr_id "
             "FROM ccpay "
             " WHERE (ccpay_id = :ccpay_id);");
  q.bindValue(":ccpay_id", _preauth->id());
  if (q.exec() && q.first())
  {
    /* _CCAmount->id() defaults to customer's currency
       if CC payment is in either customer's currency or base
       set _CCAmount appropriately
       but handle it if it somehow happens to be in a 3rd currency
     */
    int ccpayCurrId = q.value("ccpay_curr_id").toInt(); 
    if (ccpayCurrId == _CCAmount->baseId())
      _CCAmount->setBaseValue(q.value("ccpay_amount").toDouble());
    else if (ccpayCurrId != _CCAmount->id())
    {
      _CCAmount->setId(ccpayCurrId);
      _CCAmount->setLocalValue(q.value("ccpay_amount").toDouble());
    }
    else
      _CCAmount->setLocalValue(q.value("ccpay_amount").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  else
  {
    _CCAmount->clear();
  }
}

void arWorkBench::sPostPreauth()
{
  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }
  if (! cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    _CCAmount->setFocus();
    return;
  }

  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  int ccpayid   = _preauth->id();
  QString ordernum;
  int returnVal = cardproc->chargePreauthorized(-2,
						_CCAmount->localValue(),
						_CCAmount->id(),
						ordernum, ordernum, ccpayid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			 cardproc->errorMsg());
  else if (! cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
			 cardproc->errorMsg());
  else
    _CCAmount->clear();

  sFillCashrcptList();
  sFillAropenCMList();
  sFillAropenList();
  sFillPreauthList();

  _voidPreauth->setEnabled(true);
  _postPreauth->setEnabled(true);
}

void arWorkBench::sVoidPreauth()
{
  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }

  if (! cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    _CCAmount->setFocus();
    return;
  }

  _postPreauth->setEnabled(false);
  _voidPreauth->setEnabled(false);
  int ccpayid   = _preauth->id();
  QString ordernum;
  int returnVal = cardproc->voidPrevious(ccpayid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			 cardproc->errorMsg());
  else if (! cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
			 cardproc->errorMsg());
  else
    _CCAmount->clear();

  sFillCashrcptList();
  sFillAropenCMList();
  sFillAropenList();
  sFillPreauthList();

  _voidPreauth->setEnabled(true);
  _postPreauth->setEnabled(true);
}

void arWorkBench::sPopulateAropenButtonMenu()
{
  QMenu * viewAropenMenu = new QMenu;
  int menuItem;
  
  if (_aropen->altId() == 0)
    {
    menuItem = viewAropenMenu->insertItem(tr("Open Item"), this, SLOT(sViewAropen()));
    viewAropenMenu->setItemEnabled(menuItem, (_privileges->check("ViewAROpenItem") 
                                          || _privileges->check("EditAROpenItem")));
    menuItem = viewAropenMenu->insertItem(tr("Invoice"), this, SLOT(sViewInvoice()));
    viewAropenMenu->setItemEnabled(menuItem, _privileges->check("ViewAROpenItems"));
    menuItem = viewAropenMenu->insertItem(tr("Invoice Details"), this, SLOT(sViewInvoiceDetails()));
    viewAropenMenu->setItemEnabled(menuItem, _privileges->check("ViewAROpenItems"));
    _viewAropen->setMenu(viewAropenMenu);
  }
  else
  {
    menuItem = viewAropenMenu->insertItem(tr("Open Item"), this, SLOT(sViewAropen()));
    viewAropenMenu->setItemEnabled(menuItem, (_privileges->check("ViewAROpenItem") 
                                          || _privileges->check("EditAROpenItem")));
    _viewAropen->setMenu(viewAropenMenu);
  }
}

void arWorkBench::sPopulateAropenMenu(QMenu *pMenu)
{
  int menuItem;

  if (_aropen->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit Open Item..."), this, SLOT(sEditAropen()), 0);
    pMenu->setItemEnabled(menuItem, (_privileges->check("MaintainARMemos")));
      
    menuItem = pMenu->insertItem(tr("View Open Item..."), this, SLOT(sViewAropen()), 0);
    pMenu->setItemEnabled(menuItem,  (_privileges->check("MaintainARMemos") ||
                                      _privileges->check("ViewARMemos")));
  }

  if (_aropen->altId() == 0)
  {
    pMenu->insertSeparator();
      
    menuItem = pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
    pMenu->setItemEnabled(menuItem, (_privileges->check("MaintainMiscInvoices") ||
                                     _privileges->check("ViewMiscInvoices")));
                                     
    menuItem = pMenu->insertItem(tr("View Invoice Detail..."), this, SLOT(sViewInvoiceDetails()), 0);
    pMenu->setItemEnabled(menuItem, (_privileges->check("MaintainMiscInvoices") ||
                                     _privileges->check("ViewMiscInvoices")));
  }
  
  if (_aropen->id() == -1)
    {
    pMenu->insertSeparator();
    
    menuItem = pMenu->insertItem(tr("Edit Incident..."), this, SLOT(sEditIncident()), 0);
    if (!_privileges->check("MaintainIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View Incident..."), this, SLOT(sViewIncident()), 0);
    if (!_privileges->check("ViewIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    pMenu->insertSeparator();
    
    menuItem = pMenu->insertItem(tr("New Cash Receipt..."), this, SLOT(sNewCashrcpt()),0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCashReceipts"));
    
    pMenu->insertSeparator();
        
    menuItem = pMenu->insertItem(tr("New Incident..."), this, SLOT(sIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("AddIncidents"));
  }
}

void arWorkBench::sPopulateAropenCMMenu(QMenu *pMenu)
{
  int menuItem;

  if (_aropenCM->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit Open Item..."), this, SLOT(sEditAropenOnlyCM()), 0);
    pMenu->setItemEnabled(menuItem, (_privileges->check("MaintainARMemos")));
      
    menuItem = pMenu->insertItem(tr("View Open Item..."), this, SLOT(sViewAropenOnlyCM()), 0);
    pMenu->setItemEnabled(menuItem,  (_privileges->check("MaintainARMemos") ||
                                      _privileges->check("ViewARMemos")));
  }

  if (_aropenCM->altId() == 1)
  {
    pMenu->insertSeparator();
      
    menuItem = pMenu->insertItem(tr("Edit Credit Memo..."), this, SLOT(sEditAropenCM()), 0);
    if (! _privileges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Credit Memo..."), this, SLOT(sViewAropenCM()), 0);
    if (! _privileges->check("EditAROpenItem") &&
        ! _privileges->check("ViewAROpenItems"))
      pMenu->setItemEnabled(menuItem, FALSE);
      
    pMenu->insertSeparator();
      
    menuItem = pMenu->insertItem(tr("Apply Credit Memo..."), this, SLOT(sApplyAropenCM()), 0);
    if (! _privileges->check("ApplyARMemos"))
      pMenu->setItemEnabled(menuItem, FALSE);

  }
}

void arWorkBench::sPopulateCashRcptMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Cash Receipt..."), this, SLOT(sEditCashrcpt()), 0);
  if (! _privileges->check("MaintainCashReceipts") &&
      ! _privileges->check("ViewCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Cash Receipt..."), this, SLOT(sViewCashrcpt()), 0);
  if (! _privileges->check("ViewCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Cash Receipt..."), this, SLOT(sDeleteCashrcpt()), 0);
  if (! _privileges->check("MaintainCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post Cash Receipt..."), this, SLOT(sPostCashrcpt()), 0);
  if (! _privileges->check("PostCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void arWorkBench::sPopulatePreauthMenu(QMenu*)
{
  /*
  id = ccpay_id
  column 0 = ccpay_order_number || ccpay_order_number_seq
  */
}

void arWorkBench::sCCRefundCM()
{
  if (_aropenCM->altId() < 0)
  {
    QMessageBox::warning(this, tr("Cannot Refund by Credit Card"),
			 tr("<p>The application cannot refund this "
			    "transaction using a credit card."));
    return;
  }
  
  int     ccardid = -1;
  double  total   =  0.0;
  double  tax     =  0.0;
  double  freight =  0.0;
  double  duty    =  0.0;
  int     currid  = -1;
  bool    taxexempt = false;
  QString docnum;
  QString refnum;
  int     ccpayid = -1;

  q.prepare("SELECT cmhead_id "
	    "FROM cmhead "
	    "WHERE (cmhead_number=:cmheadnumber);");
  q.bindValue(":cmheadnumber", _aropenCM->currentItem()->text(1));
  q.exec();
  if (q.first())
  {
    ParameterList ccp;
    ccp.append("cmhead_id", q.value("cmhead_id"));
    MetaSQLQuery ccm = mqlLoad("creditMemoCreditCards", "detail");
    XSqlQuery ccq = ccm.toQuery(ccp);
    if (ccq.first())
    {
      ccardid = ccq.value("ccard_id").toInt();
      total   = ccq.value("total").toDouble();
      tax     = ccq.value("tax_in_cmcurr").toDouble();
      taxexempt = ccq.value("cmhead_tax_id").isNull();
      freight = ccq.value("cmhead_freight").toDouble();
      currid  = ccq.value("cmhead_curr_id").toInt();
      docnum  = ccq.value("cmhead_number").toString();
      refnum  = ccq.value("cohead_number").toString();
      ccpayid = ccq.value("ccpay_id").toInt();
    }
    else if (ccq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, ccq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    tr("Could not find a Credit Card to use for "
			       "this Credit transaction."));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else // cmhead not found - maybe it's just an open item
  {
    q.prepare("SELECT ccard_id, aropen_amount - aropen_paid AS balance,"
	      "       aropen_curr_id, aropen_docnumber "
	      "FROM aropen, ccard "
	      "WHERE ((aropen_cust_id=ccard_cust_id)"
	      "  AND  (ccard_active)"
	      "  AND  (aropen_open)"
	      "  AND  (aropen_id=:aropenid));");
    q.bindValue(":aropenid", _aropenCM->id());
    q.exec();

    if (q.first())
    {
      ccardid = q.value("ccard_id").toInt();
      total   = q.value("balance").toDouble();
      currid  = q.value("aropen_curr_id").toInt();
      docnum  = q.value("aropen_docnumber").toString();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    tr("Could not find a Credit Card to use for "
			       "this Credit transaction."));
      return;
    }
  }

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (! cardproc)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
			  CreditCardProcessor::errorMsg());
  else
  {
    int refid = _aropenCM->id();
    int returnVal = cardproc->credit(ccardid, -2, total, tax, taxexempt,
				     freight, duty, currid,
				     docnum, refnum, ccpayid, "aropen", refid);
    if (returnVal < 0)
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
			    cardproc->errorMsg());
    else if (returnVal > 0)
      QMessageBox::warning(this, tr("Credit Card Processing Warning"),
			   cardproc->errorMsg());
    else if (! cardproc->errorMsg().isEmpty())
      QMessageBox::information(this, tr("Credit Card Processing Note"),
			   cardproc->errorMsg());
  }

  sFillList();
}

void arWorkBench::sSearchDocNumChanged()
{
  QString sub = _searchDocNum->text().trimmed();
  if(sub.isEmpty())
    return;

  qDebug("sub=" + sub);
  QList<QTreeWidgetItem*> list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchCaseSensitive, 3);
  if(list.isEmpty())
  {
   qDebug("List empty");
    list = _aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchStartsWith|Qt::MatchCaseSensitive, 3);
    }

  if(!list.isEmpty())
  {
     qDebug("List not empty");
    _aropen->setCurrentItem(list.at(0));
    _aropen->scrollTo(_aropen->currentIndex());
  }
}

void arWorkBench::sIncident()
{
  q.prepare("SELECT crmacct_id, crmacct_cntct_id_1 "
            "FROM crmacct, aropen "
            "WHERE ((aropen_id=:aropen_id) "
            "AND (crmacct_cust_id=aropen_cust_id));");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("aropen_id", _aropen->id());
    params.append("crmacct_id", q.value("crmacct_id"));
    params.append("cntct_id", q.value("crmacct_cntct_id_1"));
    incident newdlg(this, 0, TRUE);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      sFillList();
  }
}

void arWorkBench::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void arWorkBench::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

