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

#include "selectPayments.h"

#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "selectBankAccount.h"
#include "selectPayment.h"
#include "storedProcErrorLookup.h"

selectPayments::selectPayments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_clearAll, SIGNAL(clicked()), this, SLOT(sClearAll()));
  connect(_selectDate, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillList()));
  connect(_startDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_endDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_onOrBeforeDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_selectDiscount, SIGNAL(clicked()), this, SLOT(sSelectDiscount()));
  connect(_selectDue, SIGNAL(clicked()), this, SLOT(sSelectDue()));
  connect(_selectLine, SIGNAL(clicked()), this, SLOT(sSelectLine()));
  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _ignoreUpdates = false;
  
  _bankaccnt->setType(XComboBox::APBankAccounts);

  _apopen->addColumn(tr("Vendor"),    -1,           Qt::AlignLeft  , true, "vendor" );
  _apopen->addColumn(tr("Doc. Type"), _orderColumn, Qt::AlignCenter, true, "doctype" );
  _apopen->addColumn(tr("Doc. #"),    _orderColumn, Qt::AlignRight , true, "apopen_docnumber" );
  _apopen->addColumn(tr("Inv. #"),    _orderColumn, Qt::AlignRight , true, "apopen_invcnumber" );
  _apopen->addColumn(tr("P/O #"),     _orderColumn, Qt::AlignRight , true, "apopen_ponumber" );
  _apopen->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignCenter, true, "apopen_duedate" );
  _apopen->addColumn(tr("Doc. Date"), _dateColumn,  Qt::AlignCenter, false, "apopen_docdate" );
  _apopen->addColumn(tr("Amount"),    _moneyColumn, Qt::AlignRight , true, "amount" );
  _apopen->addColumn(tr("Selected"),  _moneyColumn, Qt::AlignRight , true, "selected" );
  _apopen->addColumn(tr("Discount"),  _moneyColumn, Qt::AlignRight , true, "discount" );
  _apopen->addColumn(tr("Currency"),  _currencyColumn, Qt::AlignLeft, true, "curr_concat" );
  _apopen->addColumn(tr("Running (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, true, "running_selected"  );

  if (omfgThis->singleCurrency())
  {
    _apopen->hideColumn("curr_concat");
    _apopen->headerItem()->setText(11, tr("Running"));
  }

  connect(omfgThis, SIGNAL(paymentsUpdated(int, int, bool)), this, SLOT(sFillList()));

  sFillList();
}

selectPayments::~selectPayments()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectPayments::languageChange()
{
  retranslateUi(this);
}

bool selectPayments::setParams(ParameterList &params)
{
  _vendorgroup->appendValue(params);

  return true;
}

void selectPayments::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("SelectPaymentsList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void selectPayments::sSelectDue()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid >= 0)
  {
    MetaSQLQuery mql("SELECT selectDueItemsForPayment("
                     "    <? if exists(\"vend_id\") ?> <? value(\"vend_id\") ?>"
                     "    <? else ?> vend_id <? endif ?>,"
                     "    <? value(\"bankaccnt_id\") ?>) AS result "
                     "<? if exists(\"vend_id\") ?>"
                     ";"
                     "<? elseif exists(\"vendtype_id\") ?>"
                     "FROM vend "
                     "WHERE (vend_vendtype_id=<? value(\"vendtype_id\") ?>);"
                     "<? elseif exists(\"vendtype_pattern\") ?>"
                     "FROM vend "
                     "WHERE (vend_vendtype_id IN (SELECT vendtype_id"
                     "                            FROM vendtype"
                     "                            WHERE (vendtype_code ~ <? value(\"vendtype_pattern\") ?>)));"
                     "<? else ?>"
                     "FROM vend;" 
                     "<? endif ?>");
    ParameterList params;
    if (! setParams(params))
        return;
    params.append("bankaccnt_id", bankaccntid);
    q = mql.toQuery(params);
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("selectDueItemsForPayment", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sSelectDiscount()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid >= 0)
  {
    MetaSQLQuery mql("SELECT selectDiscountItemsForPayment("
                     "    <? if exists(\"vend_id\") ?> <? value(\"vend_id\") ?>"
                     "    <? else ?> vend_id <? endif ?>,"
                     "    <? value(\"bankaccnt_id\") ?>) AS result "
                     "<? if exists(\"vend_id\") ?>"
                     ";"
                     "<? elseif exists(\"vendtype_id\") ?>"
                     "FROM vend "
                     "WHERE (vend_vendtype_id=<? value(\"vendtype_id\") ?>);"
                     "<? elseif exists(\"vendtype_pattern\") ?>"
                     "FROM vend "
                     "WHERE (vend_vendtype_id IN (SELECT vendtype_id"
                     "                            FROM vendtype"
                     "                            WHERE (vendtype_code ~ <? value(\"vendtype_pattern\") ?>)));"
                     "<? else ?>"
                     "FROM vend;" 
                     "<? endif ?>");
    ParameterList params;
    if (! setParams(params))
        return;
    params.append("bankaccnt_id", bankaccntid);
    q = mql.toQuery(params);
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("selectDiscountItemsForPayment", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sClearAll()
{
  switch (_vendorgroup->state())
  {
    case VendorGroup::All:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM apselect;" );
        break;
    case VendorGroup::Selected:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM apopen, apselect "
                 "WHERE ( (apselect_apopen_id=apopen_id)"
                 " AND (apopen_vend_id=:vend_id) );" );
      break;
    case VendorGroup::SelectedType:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM vend, apopen, apselect "
                 "WHERE ( (apselect_apopen_id=apopen_id)"
                 " AND (apopen_vend_id=vend_id)"
                 " AND (vend_vendtype_id=:vendtype_id) );" );
      break;
    case VendorGroup::TypePattern:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM vend, apopen, apselect "
                 "WHERE ( (apselect_apopen_id=apopen_id)"
                 " AND (apopen_vend_id=vend_id)"
                 " AND (vend_vendtype_id IN (SELECT vendtype_id"
                 "                           FROM vendtype"
                 "                           WHERE (vendtype_code ~ :vendtype_pattern)));" );
        break;
    }

  _vendorgroup->bindValue(q);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("clearPayment", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPaymentsUpdated(-1, -1, TRUE);
}

void selectPayments::sSelect()
{
  _ignoreUpdates = true;
  bool update = false;
  QList<QTreeWidgetItem*> list = _apopen->selectedItems();
  XTreeWidgetItem * cursor = 0;
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    ParameterList params;
    params.append("apopen_id", cursor->id());

    if(_bankaccnt->id() != -1)
      params.append("bankaccnt_id", _bankaccnt->id());

    selectPayment newdlg(this, "", TRUE);
    newdlg.set(params);
    if(newdlg.exec() != XDialog::Rejected)
      update = true;
  }
  _ignoreUpdates = false;
  if(update)
    sFillList();
}

void selectPayments::sSelectLine()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid != -1)
  {
    bool update = FALSE;
    QList<QTreeWidgetItem*> list = _apopen->selectedItems();
    XTreeWidgetItem * cursor = 0;
    q.prepare("SELECT selectPayment(:apopen_id, :bankaccnt_id) AS result;");
    for(int i = 0; i < list.size(); i++)
    {
      cursor = (XTreeWidgetItem*)list.at(i);
      q.bindValue(":apopen_id", cursor->id());
      q.bindValue(":bankaccnt_id", bankaccntid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, cursor->text(0) + " " + cursor->text(2) + "\n" +
			    storedProcErrorLookup("selectPayment", result),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      update = TRUE;
    }
  
    if(update)
      omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sClear()
{
  bool update = FALSE;
  QList<QTreeWidgetItem*> list = _apopen->selectedItems();
  XTreeWidgetItem * cursor = 0;
  q.prepare("SELECT clearPayment(:apopen_id) AS result;");
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.bindValue(":apopen_id", cursor->altId());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, cursor->text(0) + " " + cursor->text(2) + "\n" +
			  storedProcErrorLookup("clearPayment", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    update = TRUE;
  }

  if(update)
    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
}

void selectPayments::sFillList()
{
  if(_ignoreUpdates)
    return;

  if ( (_selectDate->currentIndex() == 1 && !_onOrBeforeDate->isValid())  ||
        (_selectDate->currentIndex() == 2 && (!_startDate->isValid() || !_endDate->isValid())) )
    return;
    
  int _currid = -1;
  if (_bankaccnt->isValid())
  {
    q.prepare( "SELECT bankaccnt_curr_id "
               "FROM bankaccnt "
               "WHERE (bankaccnt_id=:bankaccnt_id);" );
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.exec();
    if (q.first())
      _currid = q.value("bankaccnt_curr_id").toInt();
  }
  
  MetaSQLQuery mql(
         "SELECT apopen_id, COALESCE(apselect_id, -1) AS apselectid,"
         "       (vend_number || '-' || vend_name) AS vendor,"
         "       CASE WHEN (apopen_doctype='V') THEN <? value(\"voucher\") ?>"
         "            When (apopen_doctype='D') THEN <? value(\"debitMemo\") ?>"
         "       END AS doctype,"
         "       apopen_docnumber, apopen_ponumber,"
         "       apopen_duedate,"
         "       apopen_docdate,"
         "       (apopen_amount - apopen_paid - "
         "                   COALESCE((SELECT SUM((checkitem_amount + checkitem_discount) / round(checkitem_curr_rate,5)) "
         "                             FROM checkitem, checkhead "
         "                             WHERE ((checkitem_checkhead_id=checkhead_id) "
         "                              AND (checkitem_apopen_id=apopen_id) "
         "                              AND (NOT checkhead_void) "
         "                              AND (NOT checkhead_posted)) "
         "                           ), 0)) AS amount,"
         "       COALESCE(SUM(apselect_amount), 0) AS selected,"
         "       COALESCE(currToBase(apselect_curr_id, SUM(apselect_amount), CURRENT_DATE), 0) AS running_selected,"
         "       COALESCE(SUM(apselect_discount),0) AS discount,"
         "       CASE WHEN (apopen_duedate <= CURRENT_DATE) THEN 'error' END AS qtforegroundrole, "
         "       apopen_invcnumber,"
         "       currConcat(apopen_curr_id) AS curr_concat, "
         "       'curr' AS amount_xtnumericrole, "
         "       'curr' AS selected_xtnumericrole, "
         "       'curr' AS running_selected_xtnumericrole, "
         "       'curr' AS running_selected_xtrunningrole, "
         "       'curr' AS discount_xtnumericrole "
         "FROM vend, apopen LEFT OUTER JOIN apselect ON (apselect_apopen_id=apopen_id) "
         "WHERE ( (apopen_open)"
         " AND (apopen_doctype IN ('V', 'D'))"
         " AND (apopen_vend_id=vend_id)"
         "<? if exists(\"vend_id\") ?>"
         " AND (vend_id=<? value(\"vend_id\") ?>)"
         "<? elseif exists(\"vendtype_id\") ?>"
         " AND (vend_vendtype_id=<? value(\"vendtype_id\") ?>)"
         "<? elseif exists(\"vendtype_pattern\") ?>"
         " AND (vend_vendtype_id IN (SELECT vendtype_id"
         "                           FROM vendtype"
         "                           WHERE (vendtype_code ~ <? value(\"vendtype_pattern\") ?>)))"
         "<? endif ?>"
         "<? if exists(\"olderDate\") ?>"
         " AND (apopen_duedate <= <? value(\"olderDate\") ?>)"
         "<? elseif exists(\"startDate\") ?>"
         " AND (apopen_duedate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
         "<? endif ?>"
         "<? if exists(\"curr_id\") ?>"
         " AND (apopen_curr_id=<? value(\"curr_id\") ?>)"
         "<? endif ?>"
         ") "
         "GROUP BY apopen_id, apselect_id, vend_number, vend_name,"
         "         apopen_doctype, apopen_docnumber, apopen_ponumber,"
         "         apopen_duedate, apopen_docdate, apopen_amount, apopen_paid, "
         "         curr_concat, apopen_curr_id, apselect_curr_id, apopen_invcnumber "
         "ORDER BY apopen_duedate, (apopen_amount - apopen_paid) DESC;");

  ParameterList params;
  if (! setParams(params))
    return;
  params.append("voucher", tr("Voucher"));
  params.append("debitMemo", tr("Debit Memo"));
  if (_selectDate->currentIndex()==1)
    params.append("olderDate", _onOrBeforeDate->date());
  else if (_selectDate->currentIndex()==2)
  {
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }
  if (_currid >= 0)
    params.append("curr_id", _currid);

  q = mql.toQuery(params);
  _apopen->populate(q,true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
