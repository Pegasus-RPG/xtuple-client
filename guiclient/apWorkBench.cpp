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

#include "apWorkBench.h"

#include <QSqlError>

#include <metasql.h>

#include "selectPayments.h"
#include "selectedPayments.h"
#include "viewCheckRun.h"
#include "unappliedAPCreditMemos.h"

apWorkBench::apWorkBench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QWidget *hideme = 0;

  _payables = new selectPayments(this, "selectPayments", Qt::Widget);
  _payablesTab->layout()->addWidget(_payables);
  hideme = _payables->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _payables->show();
  _vendorgroup->synchronize((VendorGroup*)(_payables->findChild<QWidget*>("_vendorgroup")));

  _credits = new unappliedAPCreditMemos(this, "creditMemos", Qt::Widget);
  _creditsTab->layout()->addWidget(_credits);
  hideme = _credits->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _credits->show();
  _vendorgroup->synchronize((VendorGroup*)(_credits->findChild<QWidget*>("_vendorgroup")));

  _selectedPayments = new selectedPayments(this, "selectedPayments", Qt::Widget);
  _selectionsTab->layout()->addWidget(_selectedPayments);
  hideme = _selectedPayments->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _selectedPayments->show();
  _vendorgroup->synchronize((VendorGroup*)(_selectedPayments->findChild<QWidget*>("_vendorgroup")));

  QWidget * _checkRun = new viewCheckRun(this, "viewCheckRun", Qt::Widget);
  _checkRunTab->layout()->addWidget(_checkRun);
  hideme = _checkRun->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _checkRun->setWindowFlags(Qt::Widget);
  _checkRun->show();
  _vendorgroup->synchronize((VendorGroup*)(_checkRun->findChild<QWidget*>("_vendorgroup")));

  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sCalculateTotalOpen()));

  sCalculateTotalOpen();
}

apWorkBench::~apWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void apWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apWorkBench::set(const ParameterList & pParams)
{
  QVariant param;
  bool    valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendorgroup->setVendId(param.toInt());

  return NoError;
}

void apWorkBench::sCalculateTotalOpen()
{
  // copied and edited from selectPayments.cpp
  ParameterList params;
  _vendorgroup->appendValue(params);
  params.append("voucher", tr("Voucher"));
  params.append("debitMemo", tr("D/M"));

  XComboBox *bankaccnt = _payables->findChild<XComboBox*>("_bankaccnt");
  if (bankaccnt->isValid())
  {
    q.prepare( "SELECT bankaccnt_curr_id "
               "FROM bankaccnt "
               "WHERE (bankaccnt_id=:bankaccnt_id);" );
    q.bindValue(":bankaccnt_id", bankaccnt->id());
    q.exec();
    if (q.first())
      params.append("curr_id", q.value("bankaccnt_curr_id").toInt());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  
  if (_payables->findChild<QRadioButton*>("_dueOlder")->isChecked())
    params.append("olderDate", _payables->findChild<DLineEdit*>("_dueOlderDate")->date());
  else if(_payables->findChild<QRadioButton*>("_dueBetween")->isChecked())
    _payables->findChild<DateCluster*>("_dueBetweenDates")->appendValue(params);
  MetaSQLQuery due(
         "SELECT SUM(currToBase(apopen_curr_id,"
         "                      apopen_amount - apopen_paid - "
         "                      COALESCE((SELECT SUM(currToCurr(checkitem_curr_id, apopen_curr_id, checkitem_amount + checkitem_discount, CURRENT_DATE)) "
         "                                FROM checkitem, checkhead "
         "                                WHERE ((checkitem_checkhead_id=checkhead_id) "
         "                                   AND (checkitem_apopen_id=apopen_id) "
         "                                   AND (NOT checkhead_void) "
         "                                   AND (NOT checkhead_posted)) "
         "                               ), 0), CURRENT_DATE)) AS openamount_base,"
         "       SUM(COALESCE(currToBase(apselect_curr_id, apselect_amount,"
         "                               CURRENT_DATE), 0)) AS selected_base "
         "FROM vend, apopen"
         "     LEFT OUTER JOIN apselect ON (apselect_apopen_id=apopen_id) "
         "WHERE ((apopen_open)"
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
         " AND (apopen_duedate < <? value(\"olderDate\") ?>)"
         "<? elseif exists(\"startDate\") ?>"
         " AND (apopen_duedate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>)"
         "<? endif ?>"
         "<? if exists(\"curr_id\") ?>"
         " AND (apopen_curr_id=<? value(\"curr_id\") ?>)"
         "<? endif ?>"
         ");" );
  q = due.toQuery(params);
  if (q.first())
    _apopenTotal->setLocalValue(q.value("openamount_base").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // copied from unappliedAPCreditMemos and edited
  MetaSQLQuery cr(
             "SELECT SUM(currtobase(apopen_curr_id,"
             "                      (apopen_amount - apopen_paid),"
             "                      apopen_docdate)) AS basebalance "
             "FROM apopen, vend "
             "WHERE ( (apopen_doctype='C')"
             " AND (apopen_open)"
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
             ");");
  q = cr.toQuery(params);
  if (q.first())
    _apopenTotal->setLocalValue(_apopenTotal->localValue() -
                                q.value("basebalance").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
