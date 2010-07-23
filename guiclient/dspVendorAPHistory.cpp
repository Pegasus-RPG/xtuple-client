/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspVendorAPHistory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "apOpenItem.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "dspGLSeries.h"
#include "mqlutil.h"

dspVendorAPHistory::dspVendorAPHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_vendhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_searchInvoiceNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchInvoiceNum()));

  _vendhist->setRootIsDecorated(TRUE);
  _vendhist->addColumn(tr("Open"),         _dateColumn,     Qt::AlignCenter, true,  "f_open" );
  _vendhist->addColumn(tr("Doc. Type"),    _itemColumn,     Qt::AlignCenter, true,  "documenttype" );
  _vendhist->addColumn(tr("Doc. #"),       _orderColumn,    Qt::AlignRight,  true,  "docnumber"  );
  _vendhist->addColumn(tr("Invoice #"),    _orderColumn,    Qt::AlignRight,  true,  "invoicenumber"  );
  _vendhist->addColumn(tr("P/O #"),        _orderColumn,    Qt::AlignRight,  true,  "ponumber"  );
  _vendhist->addColumn(tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter, true,  "docdate" );
  _vendhist->addColumn(tr("Due Date"),     _dateColumn,     Qt::AlignCenter, true,  "duedate" );
  _vendhist->addColumn(tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _vendhist->addColumn(tr("Amount (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, false, "base_amount"  );
  _vendhist->addColumn(tr("Balance"),      _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _vendhist->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _vendhist->addColumn(tr("Base Balance"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_balance"  );

  _vend->setFocus();
}

dspVendorAPHistory::~dspVendorAPHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspVendorAPHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspVendorAPHistory::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

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

void dspVendorAPHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  QAction *menuItem;

  XTreeWidgetItem * item = (XTreeWidgetItem*)pSelected;
  if (item->id() != -1)
  {
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("EditSalesHistory"));

    pMenu->addAction(tr("View A/P Open..."), this, SLOT(sView()));

    menuItem = pMenu->addAction(tr("View G/L Series..."), this, SLOT(sViewGLSeries()));
    menuItem->setEnabled(_privileges->check("ViewGLTransactions"));

    if(item->altId() == -1 && item->text(1)==tr("Voucher"))
    {
      menuItem = pMenu->addAction(tr("View Voucher..."), this, SLOT(sViewVoucher()));
      menuItem->setEnabled(_privileges->check("ViewVouchers"));

      if(item->rawValue("amount")==item->rawValue("balance"))
      {
        pMenu->addSeparator();
  
        menuItem = pMenu->addAction(tr("Void"), this, SLOT(sVoidVoucher()));
        menuItem->setEnabled(_privileges->check("MaintainAPMemos"));
      }
    } 
  }
}

void dspVendorAPHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", _vendhist->id());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  sFillList();
}

void dspVendorAPHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _vendhist->id());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspVendorAPHistory::sPrint()
{
  if (checkParameters())
  {
    ParameterList params;
    params.append("vend_id", _vend->id());
    _dates->appendValue(params);

    orReport report("VendorAPHistory", params);
    if(report.isValid())
      report.print();
    else
      report.reportError(this);
  }
}

void dspVendorAPHistory::sFillList()
{
  _vendhist->clear();

  if (!checkParameters())
    return;

  ParameterList params;

  setParams(params);
  MetaSQLQuery mql = mqlLoad("vendorAPHistory", "detail");
  q = mql.toQuery(params);
  if (q.first())
    _vendhist->populate(q, true);

  sSearchInvoiceNum();
}

bool dspVendorAPHistory::setParams(ParameterList &params)
{
  params.append("creditMemo", "Credit Memo");
  params.append("debitMemo", "Debit Memo");
  params.append("check", "Check");
  params.append("voucher", "Voucher");
  params.append("other", "Other");
  params.append("vend_id", _vend->id());
  _dates->appendValue(params);

  return true;
}

bool dspVendorAPHistory::checkParameters()
{
  if (isVisible())
  {
    if (!_vend->isValid())
    {
      QMessageBox::warning( this, tr("Select Number"),
                            tr("Please select a valid Vendor.") );
      _vend->setFocus();
      return FALSE;
    }

    if (!_dates->startDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return FALSE;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return FALSE;
    }
  }

  return TRUE;
}


void dspVendorAPHistory::sSearchInvoiceNum()
{
  QString sub = _searchInvoiceNum->text().trimmed();
  if(sub.isEmpty())
    return;

  XTreeWidgetItem *item     = 0;
  XTreeWidgetItem *foundSub = 0;
  for (int i = 0; i < _vendhist->topLevelItemCount(); i++)
  {
    item = _vendhist->topLevelItem(i);
    if (item->text(3) == sub)
    {
      foundSub = item;
      break;
    }
    else if (foundSub == 0 && item->text(3).startsWith(sub))
      foundSub = item;
  }
  if (foundSub)
  {
    _vendhist->setCurrentItem(foundSub);
    _vendhist->scrollToItem(foundSub);
  }
}

void dspVendorAPHistory::sVoidVoucher()
{
  q.prepare("SELECT voidApopenVoucher(:apopen_id) AS result;");
  q.bindValue(":apopen_id", _vendhist->id());
  q.exec();

  if(q.first())
  {
    if(q.value("result").toInt() < 0)
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );
    else
      sFillList();
  }
  else
    systemError( this, q.lastError().databaseText(), __FILE__, __LINE__);

}

void dspVendorAPHistory::sViewVoucher()
{
  q.prepare("SELECT vohead_id, COALESCE(pohead_id, -1) AS pohead_id"
            "  FROM apopen, vohead LEFT OUTER JOIN pohead ON (vohead_pohead_id=pohead_id)"
            " WHERE((vohead_number=apopen_docnumber)"
            "   AND (apopen_id=:apopen_id));");
  q.bindValue(":apopen_id", _vendhist->id());
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vohead_id", q.value("vohead_id").toInt());
  
    if (q.value("pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else
    systemError( this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void dspVendorAPHistory::sViewGLSeries()
{
  q.prepare("SELECT apopen_docdate, apopen_journalnumber"
            "  FROM apopen"
            " WHERE(apopen_id=:apopen_id);");
  q.bindValue(":apopen_id", _vendhist->id());
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("startDate", q.value("apopen_docdate").toDate());
    params.append("endDate", q.value("apopen_docdate").toDate());
    params.append("journalnumber", q.value("apopen_journalnumber").toInt());
  
    dspGLSeries *newdlg = new dspGLSeries();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
    systemError( this, q.lastError().databaseText(), __FILE__, __LINE__);
}
