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

//  moduleGL.cpp
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2007, OpenMFG, LLC

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "OpenMFGGUIClient.h"

#include "glTransaction.h"
#include "glSeries.h"
#include "unpostedGlSeries.h"

#include "standardJournal.h"
#include "standardJournals.h"
#include "standardJournalGroups.h"
#include "postStandardJournal.h"
#include "postStandardJournalGroup.h"
#include "dspStandardJournalHistory.h"
#include "rptStandardJournalHistory.h"

#include "financialLayouts.h"
#include "financialLayout.h"
#include "dspFinancialReport.h"

#include "dspGLTransactions.h"
#include "dspSummarizedGLTransactions.h"
#include "dspGLSeries.h"
#include "dspTrialBalances.h"

#include "rptSummarizedGLTransactions.h"
#include "rptTrialBalances.h"

#include "companies.h"
#include "profitCenters.h"
#include "subaccounts.h"
#include "accountNumbers.h"
#include "subAccntTypes.h"
#include "accountingPeriods.h"
#include "accountingYearPeriods.h"
#include "taxCodes.h"
#include "taxTypes.h"
#include "taxAuthorities.h"
#include "searchForCRMAccount.h"
#include "taxSelections.h"
#include "taxRegistrations.h"

#include "reconcileBankaccount.h"
#include "bankAdjustment.h"
#include "bankAdjustmentEditList.h"
#include "bankAdjustmentTypes.h"
#include "dspBankrecHistory.h"
#include "dspSummarizedBankrecHistory.h"
#include "rptSummarizedBankrecHistory.h"

#include "budgets.h"
#include "maintainBudget.h"
#include "forwardUpdateAccounts.h"
#include "duplicateAccountNumbers.h"

// START_RW
#include "postGLTransactionsToExternal.h"
#include "dspRWTransactions.h"
// END_RW

#include "moduleGL.h"

moduleGL::moduleGL(OpenMFGGUIClient *Pparent) :
 QObject(Pparent, "wmModule")
{
  parent = Pparent;

  toolBar = new QToolBar(tr("G/L Tools"));
  toolBar->setObjectName("G/L Tools");
  toolBar->setIconSize(QSize(32, 32));
  if (_preferences->boolean("ShowGLToolbar"))
    parent->addToolBar(toolBar);


  mainMenu		= new QMenu();
  enterTransactionMenu	= new QMenu();
  standardJournalsMenu	= new QMenu();
  bankrecMenu		= new QMenu();
  financialReportsMenu	= new QMenu();
  financialReportsDisplaysMenu	= new QMenu();
  financialReportsReportsMenu	= new QMenu();
  budgetMenu		= new QMenu();
  displaysMenu		= new QMenu();
  reportsMenu		= new QMenu();
  masterInfoMenu	= new QMenu();
  utilitiesMenu		= new QMenu();

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    bool		priv;
    QPixmap*		pixmap;
    QToolBar*		toolBar;
    bool		visible;
  };

  actionProperties acts[] = {
    { "menu",		    tr("Enter G/L Journal"),		(char*)enterTransactionMenu,	mainMenu,		true,					NULL, NULL, true},
    { "gl.simpleEntry",	    tr("Simple Journal Entry..."),	SLOT(sSimpleEntry()),		enterTransactionMenu,	_privleges->check("PostJournalEntries"),NULL, NULL, true},
    { "gl.seriesEntry",     tr("Series Journal Entry..."),	SLOT(sSeriesEntry()),		enterTransactionMenu,	_privleges->check("PostJournalEntries"),NULL, NULL, true},
    { "separator",	    NULL,				NULL,			        enterTransactionMenu,   true,					NULL, NULL, true},
    { "gl.unpostedEntries", tr("List Unposted Journal Entries..."), SLOT(sUnpostedEntries()),	enterTransactionMenu,	_privleges->check("PostJournalEntries"), new QPixmap(":/images/journalEntries.png"), toolBar,  true},

    { "menu",			     tr("Standard Journals"),		   (char*)standardJournalsMenu,	     mainMenu,		   true,					      NULL, NULL, true},
    { "gl.enterNewStandardJournal",  tr("Enter New Standard Journal..."),  SLOT(sEnterStandardJournal()),    standardJournalsMenu, _privleges->check("MaintainStandardJournals"),     NULL, NULL, true},
    { "gl.listStandardJournals",     tr("List Standard Journals..."),	   SLOT(sStandardJournals()),	     standardJournalsMenu, _privleges->check("MaintainStandardJournals"),     NULL, NULL, true},
    { "gl.listStandardJournalGroups",tr("List Standard Journal Groups..."),SLOT(sStandardJournalGroups()),   standardJournalsMenu, _privleges->check("MaintainStandardJournalGroups"),NULL, NULL, true},
    { "separator",		     NULL,				   NULL,			     standardJournalsMenu, true,					      NULL, NULL, true},
    { "gl.postStandardJournal",	     tr("Post Standard Journal..."),	   SLOT(sPostStandardJournal()),     standardJournalsMenu, _privleges->check("PostStandardJournals"),	      NULL, NULL, true},
    { "gl.postStandardJournalGroup", tr("Post Standard Journal Group..."), SLOT(sPostStandardJournalGroup()),standardJournalsMenu, _privleges->check("PostStandardJournalGroups"),    NULL, NULL, true},

    { "menu",			tr("Bank Reconciliation"), 	(char*)bankrecMenu,		mainMenu,    true,						NULL, NULL, true},
    { "gl.reconcileBankaccnt",	tr("Reconcile Bank Account..."),SLOT(sReconcileBankaccount()),	bankrecMenu, _privleges->check("MaintainBankRec"), new QPixmap(":/images/bankReconciliation.png"), toolBar,  true},
    { "separator",		NULL,				NULL,				bankrecMenu, true,						NULL, NULL, true},
    { "gl.enterAdjustment",	tr("Enter Adjustment..."),	SLOT(sEnterAdjustment()),	bankrecMenu, _privleges->check("MaintainBankAdjustments"),	NULL, NULL, true},
    { "gl.adjustmentEditList",	tr("Adjustment Edit List..."),	SLOT(sAdjustmentEditList()),	bankrecMenu, (_privleges->check("MaintainBankAdjustments") || _privleges->check("ViewBankAdjustments")),NULL, NULL, true},

    { "menu",			  tr("Financial Reports"),		(char*)financialReportsMenu,		mainMenu,			true,					       NULL, NULL, true},
    { "gl.createFinancialReports",tr("Create Financial Report..."),	SLOT(sNewFinancialReport()),		financialReportsMenu,		_privleges->check("MaintainFinancialLayouts"), NULL, NULL, true},
    { "gl.editFinancialReports",  tr("List Financial Reports..."),	SLOT(sFinancialReports()),		financialReportsMenu,		_privleges->check("MaintainFinancialLayouts"), NULL, NULL, true},
    { "gl.viewFinancialReport",	  tr("View Financial Report..."),	SLOT(sViewFinancialReport()),		financialReportsMenu,		_privleges->check("ViewFinancialReports"),   new QPixmap(":/images/viewFinancialReport.png"), toolBar, true},
    { "gl.dspTrialBalances",	  tr("View Trial Balances..."),		SLOT(sDspTrialBalances()),		financialReportsMenu,		_privleges->check("ViewTrialBalances"),	   new QPixmap(":/images/viewTrialBalance.png"), toolBar,  true},
    { "separator",		  NULL,					NULL,					financialReportsMenu,		true,					       NULL, NULL, true},

    { "menu",			tr("Budget"),		(char*)budgetMenu,	 mainMenu,	true,	NULL, NULL, true},
    { "gl.maintainBudget",	tr("New Budget..."),	SLOT(sMaintainBudget()), budgetMenu,	_privleges->check("MaintainBudgets"),	NULL, NULL, true},
    { "gl.maintainBudget",	tr("List Budgets..."),	SLOT(sBudgets()),	 budgetMenu,	(_privleges->check("MaintainBudgets") || _privleges->check("ViewBudgets")),	NULL, NULL, true},

    { "menu",				tr("Displays"),				(char*)displaysMenu,			mainMenu,      true,					NULL, NULL, true},
    { "gl.dspGLTransactions",		tr("G/L Transactions..."),		SLOT(sDspGLTransactions()),		displaysMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true},
    { "gl.dspSummarizedGLTransactions",	tr("Summarized G/L Transactions..."),	SLOT(sDspSummarizedGLTransactions()),	displaysMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true},
    { "gl.dspGLSeries",			tr("G/L Series..."),			SLOT(sDspGLSeries()),			displaysMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true},
    { "gl.dspStandardJournalHistory",	tr("Standard Journal History..."),	SLOT(sDspStandardJournalHistory()),	displaysMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true},
    { "gl.dspBankrecHistory",		tr("Bank Rec. History"),		SLOT(sDspBankrecHistory()),		displaysMenu, _privleges->check("ViewBankRec"),		NULL, NULL, true},
    { "gl.dspSummarizedBankrecHistory",	tr("Summarized Bank Rec. History"),	SLOT(sDspSummarizedBankrecHistory()),	displaysMenu, _privleges->check("ViewBankRec"),		NULL, NULL, true},


    { "menu",				tr("Reports"),				(char*)reportsMenu,			mainMenu,    true,					NULL, NULL, true},
    { "gl.rptSummarizedGLTransactions",	tr("Summarized G/L Transactions..."),	SLOT(sRptSummarizedGLTransactions()),	reportsMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true},
    { "gl.rptStandardJournalHistory",	tr("Standard Journal History..."),	SLOT(sRptStandardJournalHistory()),	reportsMenu, _privleges->check("ViewGLTransactions"),	NULL, NULL, true},
    { "gl.rptSummarizedBankrecHistory",	tr("Summarized Bank Rec. History"),	SLOT(sRptSummarizedBankrecHistory()),	reportsMenu, _privleges->check("ViewBankRec"),		NULL, NULL, true},

    { "menu",			tr("Master Information"),	(char*)masterInfoMenu,		mainMenu,	true,						NULL, NULL, true},
    { "gl.companies",		tr("Companies..."),		SLOT(sCompanies()),		masterInfoMenu,	(_privleges->check("MaintainChartOfAccounts") && (_metrics->value("GLCompanySize").toInt() > 0)),
    																				NULL, NULL, true},
    { "gl.profitCenterNumber",	tr("Profit Center Numbers..."),	SLOT(sProfitCenters()),		masterInfoMenu,	(_privleges->check("MaintainChartOfAccounts") && (_metrics->value("GLProfitSize").toInt() > 0)),
    																				NULL, NULL, true},
    { "gl.subaccountNumbers",	tr("Subaccount Numbers..."),	SLOT(sSubaccounts()),		masterInfoMenu,	(_privleges->check("MaintainChartOfAccounts") && (_metrics->value("GLSubaccountSize").toInt() > 0)),	
    																				NULL, NULL, true},
    { "gl.accountNumbers",	tr("Chart of Accounts..."),	SLOT(sAccountNumbers()),	masterInfoMenu,	_privleges->check("MaintainChartOfAccounts"),	NULL, NULL, true},
    { "gl.subAccntTypes",	tr("Subaccount Types..."),	SLOT(sSubAccntTypes()),		masterInfoMenu,	_privleges->check("MaintainChartOfAccounts"),	NULL, NULL, true},
    { "gl.accountingPeriods",	tr("Accounting Periods..."),	SLOT(sAccountingPeriods()),	masterInfoMenu,	_privleges->check("MaintainAccountingPeriods"),	NULL, NULL, true},
    { "gl.accountingYearPeriods",	tr("Fiscal Years..."),	SLOT(sAccountingYearPeriods()),	masterInfoMenu,	_privleges->check("MaintainAccountingPeriods"),	NULL, NULL, true},
    { "separator",		NULL,				NULL,				masterInfoMenu,	true,						NULL, NULL, true},
    { "gl.searchForTaxAuth",	tr("Search for Tax Authority..."), SLOT(sTaxAuthoritySearch()),	masterInfoMenu,	(_privleges->check("MaintainTaxAuthorities") || _privleges->check("ViewTaxAuthorities")),
    																				NULL, NULL, true},
    { "gl.taxAuthorities",	tr("Tax Authorities..."),	SLOT(sTaxAuthorities()),	masterInfoMenu,	(_privleges->check("MaintainTaxAuthorities") || _privleges->check("ViewTaxAuthorities")),
    																				NULL, NULL, true},
    { "gl.taxCodes",		tr("Tax Codes..."),		SLOT(sTaxCodes()),		masterInfoMenu,	(_privleges->check("MaintainTaxCodes") || _privleges->check("ViewTaxCodes")),	
    																				NULL, NULL, true},
    { "gl.taxTypes",		tr("Tax Types..."),		SLOT(sTaxTypes()),		masterInfoMenu,	(_privleges->check("MaintainTaxTypes") || _privleges->check("ViewTaxTypes")),	
    																				NULL, NULL, true},
    { "gl.taxSelections",	tr("Tax Selections..."),	SLOT(sTaxSelections()),		masterInfoMenu,	(_privleges->check("MaintainTaxSel") || _privleges->check("ViewTaxSel")),	NULL, NULL, true},
    { "gl.taxRegistatrions",	tr("Tax Registrations..."),	SLOT(sTaxRegistrations()),	masterInfoMenu,	_privleges->check("MaintainChartOfAccounts"),   NULL, NULL, true},
    { "separator",		NULL,				NULL,				masterInfoMenu,	true,						NULL, NULL, true},
    { "gl.adjustmentTypes",	tr("Adjustment Types..."),	SLOT(sAdjustmentTypes()),	masterInfoMenu,	(_privleges->check("MaintainAdjustmentTypes") || _privleges->check("ViewAdjustmentTypes")),	NULL, NULL, true},

    { "menu",				tr("Utilities"),			(char*)utilitiesMenu,		mainMenu,	true,	NULL, NULL, true},
    { "gl.forwardUpdateAccounts",	tr("Forward Update Accounts..."),	SLOT(sForwardUpdateAccounts()),	utilitiesMenu,	_privleges->check("ViewTrialBalances"),	NULL, NULL, true},
    { "gl.duplicateAccountNumbers",      tr("Duplicate Account Numbers..."),  SLOT(sDuplicateAccountNumbers()), utilitiesMenu,  _privleges->check("MaintainChartOfAccounts"), NULL, NULL, true},
// START_RW
    { "separator",			NULL,					NULL,				utilitiesMenu,	true,	NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") },
    { "gl.postTransactionsToExternalAccountingSystem",
					tr("Post Transactions to External Accounting System..."),
										SLOT(sPostTransactionsToExternal()), utilitiesMenu, _privleges->check("ViewGLTransactions"),
																	NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") },
    { "gl.dspRWTransactions",		tr("Display Exported Transactions..."),	SLOT(sDspRWTransactions()),	utilitiesMenu,	_privleges->check("ViewGLTransactions"),
  																	NULL, NULL, _metrics->boolean("EnableExternalAccountingInterface") },
  };
// END_RW

  for (unsigned int i = 0; i < sizeof(acts) / sizeof(acts[0]); i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == "menu")
    {
      acts[i].menu->insertItem(acts[i].actionTitle, (QMenu*)(acts[i].slot));
    }
    else if (acts[i].actionName == "separator")
    {
      acts[i].menu->addSeparator();
    }
    
    else if (acts[i].toolBar != NULL)
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv,
					  *(acts[i].pixmap),
					  acts[i].toolBar) );
    }
    
    else
    {
      parent->actions.append( new Action( parent,
					  acts[i].actionName,
					  acts[i].actionTitle,
					  this,
					  acts[i].slot,
					  acts[i].menu,
					  acts[i].priv ) );
    }
  }
  parent->populateCustomMenu(mainMenu, "G/L");
  parent->menuBar()->insertItem(tr("&G/L"), mainMenu);
}

void moduleGL::sEnterStandardJournal()
{
  ParameterList params;
  params.append("mode", "new");

  standardJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleGL::sStandardJournals()
{
  omfgThis->handleNewWindow(new standardJournals());
}

void moduleGL::sStandardJournalGroups()
{
  omfgThis->handleNewWindow(new standardJournalGroups());
}

void moduleGL::sPostStandardJournal()
{
  postStandardJournal(parent, "", TRUE).exec();
}

void moduleGL::sPostStandardJournalGroup()
{
  postStandardJournalGroup(parent, "", TRUE).exec();
}

void moduleGL::sSimpleEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glTransaction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleGL::sSeriesEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glSeries newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void moduleGL::sUnpostedEntries()
{
  omfgThis->handleNewWindow(new unpostedGlSeries());
}

//  Financial Layouts
void moduleGL::sFinancialReports()
{
  omfgThis->handleNewWindow(new financialLayouts());
}

void moduleGL::sViewFinancialReport()
{
  omfgThis->handleNewWindow(new dspFinancialReport());
}

void moduleGL::sNewFinancialReport()
{
  ParameterList params;
  params.append("mode", "new");

  financialLayout newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


//  Displays
void moduleGL::sDspGLTransactions()
{
  omfgThis->handleNewWindow(new dspGLTransactions());
}

void moduleGL::sDspSummarizedGLTransactions()
{
  omfgThis->handleNewWindow(new dspSummarizedGLTransactions());
}

void moduleGL::sDspGLSeries()
{
  omfgThis->handleNewWindow(new dspGLSeries());
}

void moduleGL::sDspStandardJournalHistory()
{
  omfgThis->handleNewWindow(new dspStandardJournalHistory());
}

void moduleGL::sDspTrialBalances()
{
  omfgThis->handleNewWindow(new dspTrialBalances());
}


//  Reports
void moduleGL::sRptSummarizedGLTransactions()
{
  rptSummarizedGLTransactions(parent, "", TRUE).exec();
}

void moduleGL::sRptStandardJournalHistory()
{
  rptStandardJournalHistory(parent, "", TRUE).exec();
}

void moduleGL::sRptTrialBalances()
{
  rptTrialBalances(parent, "", TRUE).exec();
}


//  Master Information
void moduleGL::sCompanies()
{
  omfgThis->handleNewWindow(new companies());
}

void moduleGL::sProfitCenters()
{
  omfgThis->handleNewWindow(new profitCenters());
}

void moduleGL::sSubaccounts()
{
  omfgThis->handleNewWindow(new subaccounts());
}

void moduleGL::sAccountNumbers()
{
  omfgThis->handleNewWindow(new accountNumbers());
}

void moduleGL::sDuplicateAccountNumbers()
{
  omfgThis->handleNewWindow(new duplicateAccountNumbers());
}

void moduleGL::sSubAccntTypes()
{
  omfgThis->handleNewWindow(new subAccntTypes());
}

void moduleGL::sAccountingPeriods()
{
  omfgThis->handleNewWindow(new accountingPeriods());
}

void moduleGL::sAccountingYearPeriods()
{
  omfgThis->handleNewWindow(new accountingYearPeriods());
}

void moduleGL::sReconcileBankaccount()
{
  omfgThis->handleNewWindow(new reconcileBankaccount());
}

void moduleGL::sEnterAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleGL::sAdjustmentEditList()
{
  omfgThis->handleNewWindow(new bankAdjustmentEditList());
}

void moduleGL::sAdjustmentTypes()
{
  omfgThis->handleNewWindow(new bankAdjustmentTypes());
}

void moduleGL::sTaxAuthorities()
{
  omfgThis->handleNewWindow(new taxAuthorities());
}

void moduleGL::sTaxAuthoritySearch()
{
  ParameterList params;
  params.append("crmaccnt_subtype", "taxauth");

  searchForCRMAccount *newdlg = new searchForCRMAccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void moduleGL::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void moduleGL::sTaxTypes()
{
  omfgThis->handleNewWindow(new taxTypes());
}

void moduleGL::sTaxSelections()
{
  omfgThis->handleNewWindow(new taxSelections());
}

void moduleGL::sTaxRegistrations()
{
  omfgThis->handleNewWindow(new taxRegistrations());
}

void moduleGL::sDspBankrecHistory()
{
  omfgThis->handleNewWindow(new dspBankrecHistory());
}

void moduleGL::sDspSummarizedBankrecHistory()
{
  omfgThis->handleNewWindow(new dspSummarizedBankrecHistory());
}

void moduleGL::sRptSummarizedBankrecHistory()
{
  rptSummarizedBankrecHistory(parent, "", TRUE).exec();
}

void moduleGL::sBudgets()
{
  omfgThis->handleNewWindow(new budgets());
}

void moduleGL::sMaintainBudget()
{
  ParameterList params;
  params.append("mode", "new");

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void moduleGL::sForwardUpdateAccounts()
{
  forwardUpdateAccounts(parent, "", TRUE).exec();
}

// START_RW
//  Utilities
void moduleGL::sPostTransactionsToExternal()
{
  postGLTransactionsToExternal(parent, "", TRUE).exec();
}

void moduleGL::sDspRWTransactions()
{
  omfgThis->handleNewWindow(new dspRWTransactions());
}
// END_RW

