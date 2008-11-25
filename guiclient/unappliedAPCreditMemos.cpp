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

#include "unappliedAPCreditMemos.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "applyAPCreditMemo.h"
#include "apOpenItem.h"

unappliedAPCreditMemos::unappliedAPCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));

  _new->setEnabled(_privileges->check("MaintainAPMemos"));

  connect(_apopen, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _apopen->addColumn( tr("Doc. #"),       _itemColumn,     Qt::AlignLeft,   true,  "apopen_docnumber" );
  _apopen->addColumn( tr("Vendor"),       -1,              Qt::AlignLeft,   true,  "vendor"   );
  _apopen->addColumn( tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "apopen_amount"  );
  _apopen->addColumn( tr("Applied"),      _moneyColumn,    Qt::AlignRight,  true,  "apopen_paid"  );
  _apopen->addColumn( tr("Balance"),      _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _apopen->addColumn( tr("Currency"),     _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _apopen->addColumn( tr("Balance (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, true, "basebalance");

  if (omfgThis->singleCurrency())
    _apopen->hideColumn("currAbbr");

  if (_privileges->check("ApplyAPMemos"))
    connect(_apopen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));

  sFillList();
}

unappliedAPCreditMemos::~unappliedAPCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void unappliedAPCreditMemos::languageChange()
{
  retranslateUi(this);
}

bool unappliedAPCreditMemos::setParams(ParameterList &params)
{
  _vendorgroup->appendValue(params);
  return true;
}

void unappliedAPCreditMemos::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("UnappliedAPCreditMemos", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unappliedAPCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode",    "new");
  params.append("docType", "creditMemo");
  if (_vendorgroup->isSelectedVend())
    params.append("vend_id", _vendorgroup->vendId());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void unappliedAPCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apopen->id());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void unappliedAPCreditMemos::sFillList()
{
  MetaSQLQuery mql(
             "SELECT apopen_id, apopen_docnumber,"
             "       (vend_number || '-' || vend_name) AS vendor,"
             "       apopen_amount, apopen_paid,"
             "       (apopen_amount - apopen_paid) AS balance,"
             "       currtobase(apopen_curr_id,(apopen_amount - apopen_paid),apopen_docdate) AS basebalance,"
             "	     currConcat(apopen_curr_id) AS currAbbr,"
             "       'curr' AS apopen_amount_xtnumericrole,"
             "       'curr' AS apopen_paid_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole,"
             "       'curr' AS basebalance_xtnumericrole,"
             "       0 AS basebalance_xttotalrole "
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
             ") "
             "ORDER BY apopen_docnumber;" );
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _apopen->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void unappliedAPCreditMemos::sApply()
{
  ParameterList params;
  params.append("apopen_id", _apopen->id());

  applyAPCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}
