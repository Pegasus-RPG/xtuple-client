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

#include "dspTrialBalances.h"

#include <math.h>

#include <QMenu>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "dspGLTransactions.h"
#include "storedProcErrorLookup.h"

dspTrialBalances::dspTrialBalances(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_trialbal, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _trialbal->addColumn(tr("Start"),       _dateColumn,     Qt::AlignCenter, true,  "period_start" );
  _trialbal->addColumn(tr("End"),         _dateColumn,     Qt::AlignCenter, true,  "period_end" );
  _trialbal->addColumn(tr("Account #"),   _itemColumn,     Qt::AlignCenter, true,  "account" );
  _trialbal->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "accnt_descrip"   );
  _trialbal->addColumn(tr("Beg. Bal."),   _bigMoneyColumn, Qt::AlignRight,  true,  "beginning"  );
  _trialbal->addColumn("",                25,              Qt::AlignLeft,   true,  "beginningsense"   );
  _trialbal->addColumn(tr("Debits"),      _bigMoneyColumn, Qt::AlignRight,  true,  "debits"  );
  _trialbal->addColumn(tr("Credits"),     _bigMoneyColumn, Qt::AlignRight,  true,  "credits"  );
  _trialbal->addColumn(tr("Difference"),  _bigMoneyColumn, Qt::AlignRight,  true,  "diff"  );
  _trialbal->addColumn("",                25,              Qt::AlignLeft,   true,  "diffsense"   );
  _trialbal->addColumn(tr("End Bal."),    _bigMoneyColumn, Qt::AlignRight,  true,  "ending"  );
  _trialbal->addColumn("",                25,              Qt::AlignLeft,   true,  "endingsense"   );
}

dspTrialBalances::~dspTrialBalances()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTrialBalances::languageChange()
{
  retranslateUi(this);
}

void dspTrialBalances::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View G/L Transactions..."), this, SLOT(sViewTransactions()), 0);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Forward Update"), this, SLOT(sForwardUpdate()), 0);
}

void dspTrialBalances::sViewTransactions()
{
  ParameterList params;
  params.append("accnt_id", _trialbal->id());
  params.append("period_id", _trialbal->altId());
  params.append("run");

  dspGLTransactions *newdlg = new dspGLTransactions();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTrialBalances::sForwardUpdate()
{
  q.prepare( "SELECT MIN(forwardUpdateTrialBalance(trialbal_id)) AS result "
             "FROM trialbal "
             "WHERE ( (trialbal_period_id=:period_id)"
             " AND (trialbal_accnt_id=:accnt_id) );");
  q.bindValue(":accnt_id", _trialbal->id());
  q.bindValue(":period_id", _trialbal->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("forwardUpdateTrialBalance", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspTrialBalances::setParams(ParameterList & params)
{
  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());

  if (_selectedPeriod->isChecked())
    params.append("period_id", _period->id());
}

void dspTrialBalances::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("TrialBalances", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspTrialBalances::sFillList()
{
  _trialbal->clear();

  QString sql( "SELECT accnt_id, period_id, accnt_descrip, trialbal_dirty,"
               "       period_start, period_end,"
               "       formatGLAccount(accnt_id) AS account,"
               "       (trialbal_debits) AS debits,"
               "       (trialbal_credits) AS credits,"
               "       (trialbal_beginning*-1) AS beginning,"
               "       (trialbal_ending*-1) AS ending,"
               "       (trialbal_debits - trialbal_credits) AS diff,"
               "       ABS(trialbal_beginning*-1) AS beginning_qtdisplayrole,"
               "       ABS(trialbal_ending*-1) AS ending_qtdisplayrole,"
               "       ABS(trialbal_debits - trialbal_credits) AS diff_qtdisplayrole,"
               "       CASE WHEN ((trialbal_beginning*-1)<0) THEN 'CR' END AS beginningsense,"
               "       CASE WHEN ((trialbal_ending*-1)<0) THEN 'CR' END AS endingsense,"
               "       CASE WHEN ((trialbal_debits - trialbal_credits)<0) THEN 'CR' END AS diffsense,"
               "       'curr' AS beginning_xtnumericrole,"
               "       'curr' AS debits_xtnumericrole,"
               "       'curr' AS credits_xtnumericrole,"
               "       'curr' AS ending_xtnumericrole,"
               "       'curr' AS diff_xtnumericrole,"
               "       0 AS beginning_xttotalrole,"
               "       0 AS debits_xttotalrole,"
               "       0 AS credits_xttotalrole,"
               "       0 AS ending_xttotalrole,"
               "       0 AS diff_xttotalrole,"
               "       CASE WHEN (trialbal_dirty) THEN 'warning' END AS ending_qtforegroundrole "
               "FROM trialbal, accnt, period "
               "WHERE ( (trialbal_accnt_id=accnt_id)"
               " AND (trialbal_period_id=period_id)"
	       "<? if exists(\"accnt_id\") ?>"
	       " AND (trialbal_accnt_id=<? value(\"accnt_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"period_id\") ?>"
	       " AND (period_id=<? value(\"period_id\") ?>)"
	       "<? endif ?>"
	       ") "
	       "ORDER BY period_start, formatGLAccount(accnt_id);" );

  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    _trialbal->populate(q, true);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
