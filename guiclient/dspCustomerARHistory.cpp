/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomerARHistory.h"

#include <QVariant>
#include <QMessageBox>
#include <QMenu>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include "arOpenItem.h"

/*
 *  Constructs a dspCustomerARHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCustomerARHistory::dspCustomerARHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_custhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem *)));

  _custhist->setRootIsDecorated(TRUE);
  _custhist->addColumn(tr("Open"),          _dateColumn,     Qt::AlignCenter, true,  "open" );
  _custhist->addColumn(tr("Doc. Type"),     _itemColumn,     Qt::AlignCenter, true,  "documenttype" );
  _custhist->addColumn(tr("Doc. #"),        _orderColumn,    Qt::AlignRight,  true,  "docnumber"  );
  _custhist->addColumn(tr("Doc. Date"),     _dateColumn,     Qt::AlignCenter, true,  "docdate" );
  _custhist->addColumn(tr("Due/Dist Date"), _dateColumn,     Qt::AlignCenter, true,  "duedate" );
  _custhist->addColumn(tr("Amount"),        _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _custhist->addColumn(tr("Balance"),       _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _custhist->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignCenter, true,  "currAbbr"  );
  _custhist->addColumn(tr("Base Balance"),  _bigMoneyColumn, Qt::AlignRight,  true,  "base_balance"  );

  _cust->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCustomerARHistory::~dspCustomerARHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCustomerARHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCustomerARHistory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspCustomerARHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  }
}

void dspCustomerARHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _custhist->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerARHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");

  if (_custhist->id() == -1)
    params.append("aropen_id", _custhist->altId());
  else
    params.append("aropen_id", _custhist->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerARHistory::sPrint()
{
  if(!checkParameters())
    return;

  ParameterList params;
  params.append("cust_id", _cust->id());
  _dates->appendValue(params);

  orReport report("CustomerARHistory", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspCustomerARHistory::sFillList()
{
  _custhist->clear();

  if (!checkParameters())
    return;

  MetaSQLQuery mql = mqlLoad("arHistory", "detail");
  ParameterList params;
  _dates->appendValue(params);
  params.append("invoice", tr("Invoice"));
  params.append("zeroinvoice", tr("Zero Invoice"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("cashdeposit", tr("Customer Deposit"));
  params.append("other", tr("Other"));
  params.append("cust_id", _cust->id());
  q = mql.toQuery(params);
  _custhist->populate(q, true);
}

bool dspCustomerARHistory::checkParameters()
{
  if (!_cust->isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Customer Number"),
                            tr("Please enter a valid Customer Number.") );
      _cust->setFocus();
    }
    return FALSE;
  }

  if (!_dates->startDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  if (!_dates->endDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  return TRUE;
}
