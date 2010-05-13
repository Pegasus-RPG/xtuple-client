/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCashReceipts.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <datecluster.h>
#include <openreports.h>
#include "arOpenItem.h"
#include "cashReceipt.h"
#include "storedProcErrorLookup.h"

/*
 *  Constructs a dspCashReceipts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCashReceipts::dspCashReceipts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewCashrcpt()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPostCashrcpt()));
  connect(_reverse, SIGNAL(clicked()), this, SLOT(sReversePosted()));
  connect(_arapply, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
  connect(_arapply, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_applications, SIGNAL(toggled(bool)), _arapply, SLOT(clear()));
  connect(_applications, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartDate(QDate().currentDate().addDays(-90));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  
  _arapply->addColumn(tr("Number"),      _orderColumn,    Qt::AlignCenter, true,  "cashrcpt_number" );
  _arapply->addColumn(tr("Source"),      _itemColumn,     Qt::AlignLeft,   true,  "source" );
  _arapply->addColumn(tr("Cust. #"),     _orderColumn,    Qt::AlignCenter, true,  "cust_number" );
  _arapply->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _arapply->addColumn(tr("Posted"),      _ynColumn,       Qt::AlignCenter, true,  "posted" );
  _arapply->addColumn(tr("Voided"),      _ynColumn,       Qt::AlignCenter, true,  "voided" );
  _arapply->addColumn(tr("Date"),        _dateColumn,     Qt::AlignCenter, true,  "postdate" );
  _arapply->addColumn(tr("Apply-To"),    _itemColumn,     Qt::AlignCenter, true,  "target" );
  _arapply->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight,  true,  "applied"  );
  _arapply->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  _arapply->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_applied"  );
  
  _upgradeWarn = new XErrorMessage(this);
  
  _new->setEnabled(_privileges->check("MaintainCashReceipts"));
  
  sHandleButtons(false);
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCashReceipts::~dspCashReceipts()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCashReceipts::languageChange()
{
  retranslateUi(this);
}

void dspCashReceipts::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  params.append("includeFormatted");

  orReport report("CashReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCashReceipts::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  _arapply->clear();

  MetaSQLQuery mql = mqlLoad("cashReceipts", "detail");
  q = mql.toQuery(params);
  if (q.first())
    _arapply->populate(q, true);
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspCashReceipts::setParams(ParameterList &pParams)
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _customerSelector->appendValue(pParams);
  _dates->appendValue(pParams);
  pParams.append("creditMemo", tr("Credit Memo"));
  pParams.append("debitMemo", tr("Debit Memo"));
  pParams.append("cashdeposit", tr("Customer Deposit"));
  pParams.append("invoice", tr("Invoice"));
  pParams.append("cash", tr("Cash"));
  pParams.append("check", tr("Check"));
  pParams.append("certifiedCheck", tr("Cert. Check"));
  pParams.append("masterCard", tr("Master Card"));
  pParams.append("visa", tr("Visa"));
  pParams.append("americanExpress", tr("AmEx"));
  pParams.append("discoverCard", tr("Discover"));
  pParams.append("otherCreditCard", tr("Other C/C"));
  pParams.append("wireTransfer", tr("Wire Trans."));
  pParams.append("other", tr("Other"));
  pParams.append("unapplied", tr("Customer Deposit"));
  pParams.append("unposted", tr("Unposted"));
  pParams.append("voided", tr("Voided"));
    
  if (_applications->isChecked())
  {
    _arapply->hideColumn("cashrcpt_number");
    pParams.append("LegacyDisplayMode");
  }
  else
  { 
    _arapply->showColumn("cashrcpt_number");
    if(_metrics->boolean("LegacyCashReceipts"))
    {
      _upgradeWarn->showMessage(
        tr("This feature was introduced in version 3.3.0.  "
           "Cash Receipts prior to this version will not be displayed."));
    }
  }
  return true;
}

void dspCashReceipts::sPopulateMenu( QMenu * pMenu )
{
  int menuItem = -1;
  
  if (_arapply->id() > -1)
  {
    // Cash Receipt              
    if (!_arapply->currentItem()->rawValue("posted").toBool() && !_arapply->currentItem()->rawValue("voided").toBool())
    {
      menuItem = pMenu->insertItem(tr("Edit Cash Receipt..."), this, SLOT(sEditCashrcpt()), 0);
      pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCashReceipts"));
    }

    pMenu->insertItem(tr("View Cash Receipt..."), this, SLOT(sViewCashrcpt()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewCashReceipts") || _privileges->check("MaintainCashReceipts"));

    if (!_arapply->currentItem()->rawValue("voided").toBool())
    {      
      if (!_arapply->currentItem()->rawValue("posted").toBool())
      {   
        menuItem = pMenu->insertItem(tr("Post Cash Receipt"), this, SLOT(sPostCashrcpt()), 0);
        pMenu->setItemEnabled(menuItem, _privileges->check("PostCashReceipts"));
      }
      else if (!_arapply->altId())
      {
        menuItem = pMenu->insertItem(tr("Reverse Posted Cash Receipt"), this, SLOT(sReversePosted()), 0);
        pMenu->setItemEnabled(menuItem, _privileges->check("ReversePostedCashReceipt"));
      }
    }

    // Open Item
    if (_arapply->currentItem()->id("target") > -1 )
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("Edit Receivable Item..."), this, SLOT(sEditAropen()), 0);
      pMenu->setItemEnabled(menuItem, _privileges->check("EditAROpenItem"));
      menuItem = pMenu->insertItem(tr("View Receivable Item..."), this, SLOT(sViewAropen()), 0);
      pMenu->setItemEnabled(menuItem, _privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
    }
  }
}

void dspCashReceipts::sEditAropen()
{   
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _arapply->currentItem()->id("target"));
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCashReceipts::sViewAropen()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _arapply->currentItem()->id("target"));
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCashReceipts::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sEditCashrcpt()
{    
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", _arapply->currentItem()->id("source"));

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sViewCashrcpt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", _arapply->currentItem()->id("source"));

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sPostCashrcpt()
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

  q.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
  q.bindValue(":cashrcpt_id", _arapply->currentItem()->id("source"));
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
    
  tx.exec("COMMIT;");
  omfgThis->sCashReceiptsUpdated(_arapply->currentItem()->id("source"), TRUE);
  sFillList();
}

void dspCashReceipts::sReversePosted()
{
  if (QMessageBox::warning(this, tr("Reverse Entire Posting?"),
                                  tr("<p>This will reverse all applications related "
                                     "to this cash receipt.  Are you sure you want "
                                     "to do this?"),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
  {                     
    q.prepare("SELECT reverseCashReceipt(:cashrcpt_id, fetchJournalNumber('C/R')) AS result;");
    q.bindValue(":cashrcpt_id", _arapply->currentItem()->id("source"));
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("reverseCashReceipt", result),
                         __FILE__, __LINE__);
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
}

void dspCashReceipts::sHandleButtons(bool valid)
{      
  int menuItem = -1;
  QMenu * editMenu = new QMenu;
  QMenu * viewMenu = new QMenu;   

  _reverse->setVisible(!_applications->isChecked());

  if (valid && _arapply->id() > -1)
  {
    // Handle Edit Button
    // Cash Receipt
    if (_arapply->currentItem()->rawValue("voided").toBool())
    {
      _post->hide();
      _reverse->hide();
    }
    else if (!_arapply->currentItem()->rawValue("posted").toBool())
    {
      editMenu->insertItem(tr("Cash Receipt..."), this, SLOT(sEditCashrcpt()), 0);
      editMenu->setItemEnabled(menuItem, _privileges->check("MaintainCashReceipts"));
        
      _post->show();
      _reverse->setVisible(!_applications->isChecked());
      _reverse->setEnabled(false);
      _post->setEnabled(_privileges->check("PostCashReceipts"));
    }
    else
    {
      _post->show();
      _reverse->setVisible(!_applications->isChecked());
      _post->setEnabled(false);
      _reverse->setEnabled(_privileges->check("ReversePostedCashReceipt")
                           && _cashreceipts->isChecked()
                           && !_arapply->altId());
    } 
        
    if (_arapply->currentItem()->id("target") > -1)
    {
      editMenu->insertItem(tr("Receivable Item..."), this, SLOT(sEditAropen()), 0);
      editMenu->setItemEnabled(menuItem, _privileges->check("EditAROpenItem"));
    }
    
    // Handle View Button
    // Cash Receipt             
    viewMenu->insertItem(tr("Cash Receipt..."), this, SLOT(sViewCashrcpt()), 0);
    viewMenu->setItemEnabled(menuItem, _privileges->check("ViewCashReceipts") || _privileges->check("MaintainCashReceipts"));
   
    // Open Item
    if (_arapply->currentItem()->id("target") > -1)
    {
      viewMenu->insertItem(tr("Receivable Item..."), this, SLOT(sViewAropen()), 0);
      viewMenu->setItemEnabled(menuItem, _privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
    }   
    
    _edit->setMenu(editMenu);
    _edit->setEnabled(!editMenu->isEmpty());
    _view->setMenu(viewMenu);
    _view->setEnabled(!viewMenu->isEmpty());
  }
  else
  {
    _edit->setEnabled(false);
    _view->setEnabled(false);
    _post->setEnabled(false);
    _reverse->setEnabled(false);
  }
}
