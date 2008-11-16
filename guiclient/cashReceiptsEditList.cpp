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

#include "cashReceiptsEditList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "cashReceipt.h"
#include "storedProcErrorLookup.h"

cashReceiptsEditList::cashReceiptsEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cashrcpt, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));
  connect(_post,   SIGNAL(clicked()), this, SLOT(sPost()));

  _cashrcpt->addColumn(tr("Cust. #"),       _bigMoneyColumn, Qt::AlignLeft,  true, "cust_number");                                                                
  _cashrcpt->addColumn(tr("Name"),                       -1, Qt::AlignLeft,  true, "cust_name"); 
  _cashrcpt->addColumn(tr("Check/Doc. #"),     _orderColumn, Qt::AlignLeft,  true, "cashrcpt_docnumber");
  _cashrcpt->addColumn(tr("Bank Account"),     _orderColumn, Qt::AlignLeft,  true, "bankaccnt_name");
  _cashrcpt->addColumn(tr("Dist. Date"),        _dateColumn, Qt::AlignCenter,true, "cashrcpt_distdate");
  _cashrcpt->addColumn(tr("Funds Type"),    _bigMoneyColumn, Qt::AlignCenter,true, "cashrcpt_fundstype");
  _cashrcpt->addColumn(tr("Amount"),        _bigMoneyColumn, Qt::AlignRight, true, "cashrcpt_amount");
  _cashrcpt->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (omfgThis->singleCurrency())
      _cashrcpt->hideColumn("cashrcpt_curr_id");

  if (_privileges->check("PostCashReceipts"))
    connect(_cashrcpt, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    
  if (_privileges->check("MaintainCashReceipts"))
  {
    connect(_cashrcpt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

cashReceiptsEditList::~cashReceiptsEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void cashReceiptsEditList::languageChange()
{
  retranslateUi(this);
}

void cashReceiptsEditList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post..."), this, SLOT(sPost()), 0);
  if (!_privileges->check("PostCashReceipts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void cashReceiptsEditList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void cashReceiptsEditList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void cashReceiptsEditList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void cashReceiptsEditList::sDelete()
{
  q.prepare( "SELECT deleteCashRcpt(:cashrcpt_id) AS result;");
  q.bindValue(":cashrcpt_id", _cashrcpt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCashRcpt", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void cashReceiptsEditList::sPost()
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

void cashReceiptsEditList::sPrint()
{
  orReport report("CashReceiptsEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void cashReceiptsEditList::sFillList()
{
  MetaSQLQuery mql = mqlLoad("unpostedCashReceipts", "detail");
  ParameterList params;
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
