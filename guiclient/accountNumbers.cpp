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

#include "accountNumbers.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "accountNumber.h"
#include "storedProcErrorLookup.h"

accountNumbers::accountNumbers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_account,        SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_delete,           SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,             SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,              SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,            SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showExternal, SIGNAL(toggled(bool)), this, SLOT(sBuildList()));

  connect(omfgThis, SIGNAL(configureGLUpdated()), this, SLOT(sBuildList()));

  _showExternal->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));

  _externalCol = -1;

  sBuildList();
}

accountNumbers::~accountNumbers()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountNumbers::languageChange()
{
  retranslateUi(this);
}

void accountNumbers::sDelete()
{
  q.prepare("SELECT deleteAccount(:accnt_id) AS result;");
  q.bindValue(":accnt_id", _account->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteAccount", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumbers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  accountNumber newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountNumbers::sEdit()
{
  ParameterList params;
  // don't allow editing external accounts
  if (_externalCol >= 0 &&
      _account->currentItem()->rawValue("company_external").toBool())
    params.append("mode", "view");
  else
    params.append("mode", "edit");
  params.append("accnt_id", _account->id());

  accountNumber newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool accountNumbers::setParams(ParameterList &pParams)
{
  if (_metrics->boolean("MultiCompanyFinancialConsolidation") &&
      _showExternal->isChecked())
    pParams.append("showExternal");

  pParams.append("asset",     tr("Asset"));
  pParams.append("expense",   tr("Expense"));
  pParams.append("liability", tr("Liability"));
  pParams.append("equity",    tr("Equity"));
  pParams.append("revenue",   tr("Revenue"));

  return true;
}

void accountNumbers::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("AccountNumberMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void accountNumbers::sFillList()
{
  QString sql = "SELECT accnt_id, *,"
                "       CASE WHEN(accnt_type='A') THEN <? value(\"asset\") ?>"
                "            WHEN(accnt_type='E') THEN <? value(\"expense\") ?>"
                "            WHEN(accnt_type='L') THEN <? value(\"liability\") ?>"
                "            WHEN(accnt_type='Q') THEN <? value(\"equity\") ?>"
                "            WHEN(accnt_type='R') THEN <? value(\"revenue\") ?>"
                "            ELSE accnt_type"
                "       END AS accnt_type_qtdisplayrole "
                "FROM (accnt LEFT OUTER JOIN"
                "     company ON (accnt_company=company_number)) "
                "     LEFT OUTER JOIN subaccnttype ON (accnt_type=subaccnttype_accnt_type AND accnt_subaccnttype_code=subaccnttype_code) "
                "<? if not exists(\"showExternal\") ?>"
                "WHERE (NOT COALESCE(company_external, false)) "
                "<? endif ?>"
                "ORDER BY accnt_number, accnt_sub, accnt_profit;" ;
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  _account->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumbers::sBuildList()
{
  _account->setColumnCount(0);
  _externalCol = -1;

  if (_metrics->value("GLCompanySize").toInt() > 0)
  {
    _account->addColumn(tr("Company"),       50, Qt::AlignCenter, true, "accnt_company");
    _externalCol++;
  }

  if (_metrics->value("GLProfitSize").toInt() > 0)
  {
    _account->addColumn(tr("Profit"),        50, Qt::AlignCenter, true, "accnt_profit");
    _externalCol++;
  }

  _account->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");
  _externalCol++;

  if (_metrics->value("GLSubaccountSize").toInt() > 0)
  {
    _account->addColumn(tr("Sub."),          50, Qt::AlignCenter, true, "accnt_sub");
    _externalCol++;
  }

  _account->addColumn(tr("Description"),     -1, Qt::AlignLeft,   true, "accnt_descrip");
  _externalCol++;

  _account->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _externalCol++;

  _account->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _externalCol++;

  _account->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");
  _externalCol++;

  if (_metrics->value("Application") == "xTuple" ||
      _metrics->value("Application") == "OpenMFG")
  {
    _account->addColumn(tr("External"), _ynColumn, Qt::AlignCenter, false, "company_external");
    _externalCol++;
  }
  else
    _externalCol = -1;

  sFillList();
}

void accountNumbers::sHandleButtons()
{
  // don't allow editing external accounts
  if (_externalCol >= 0 && _account->currentItem() &&
      _account->currentItem()->rawValue("company_external").toBool())
    _edit->setText(tr("View"));
  else
    _edit->setText(tr("Edit"));
}
