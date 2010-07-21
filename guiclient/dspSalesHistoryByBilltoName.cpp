/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesHistoryByBilltoName.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "salesHistoryInformation.h"

dspSalesHistoryByBilltoName::dspSalesHistoryByBilltoName(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _sohist->addColumn(tr("Customer"),            -1,              Qt::AlignLeft,   true,  "cust_name");
  _sohist->addColumn(tr("Bill-To Name"),        -1,              Qt::AlignLeft,   true,  "cohist_billtoname"   );
  _sohist->addColumn(tr("Doc. #"),              _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _sohist->addColumn(tr("Invoice #"),           _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  _sohist->addColumn(tr("Ord. Date"),           _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  _sohist->addColumn(tr("Invc. Date"),          _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _sohist->addColumn(tr("Item Number"),         _itemColumn,     Qt::AlignLeft,   true,  "item_number"   );
  _sohist->addColumn(tr("Shipped"),             _qtyColumn,      Qt::AlignRight,  true,  "cohist_qtyshipped"  );
  if (_privileges->check("ViewCustomerPrices"))
  {
    _sohist->addColumn(tr("Unit Price"),        _priceColumn,    Qt::AlignRight,  true,  "cohist_unitprice" );
    _sohist->addColumn(tr("Ext. Price"),        _bigMoneyColumn, Qt::AlignRight,  true,  "extprice" );
    _sohist->addColumn(tr("Currency"),          _currencyColumn, Qt::AlignRight,  true,  "currAbbr" );
    _sohist->addColumn(tr("Base Unit Price"),   _bigMoneyColumn, Qt::AlignRight,  true,  "baseunitprice" );
    _sohist->addColumn(tr("Base Ext. Price"),   _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
  }
  if (_privileges->check("ViewCosts"))
  {
    _sohist->addColumn( tr("Unit Cost"),        _costColumn,     Qt::AlignRight,  true,  "cohist_unitcost" );
    _sohist->addColumn( tr("Ext. Cost"),        _bigMoneyColumn, Qt::AlignRight,  true,  "extcost" );
  }

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();
}

dspSalesHistoryByBilltoName::~dspSalesHistoryByBilltoName()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSalesHistoryByBilltoName::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSalesHistoryByBilltoName::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("billto_name", &valid);
  if (valid)
    _billtoName->setText(param.toString());

  param = pParams.value("prodcat_id", &valid);
  if (valid)
    _productCategory->setId(param.toInt());

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
    _productCategory->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

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

void dspSalesHistoryByBilltoName::sHandleParams()
{
  if (_showPrices->isChecked())
  {
    _sohist->showColumn("cohist_unitprice");
    _sohist->showColumn("extprice");
    _sohist->showColumn("currAbbr");
    _sohist->showColumn("baseunitprice");
    _sohist->showColumn("baseextprice");
  }
  else
  {
    _sohist->hideColumn("cohist_unitprice");
    _sohist->hideColumn("extprice");
    _sohist->hideColumn("currAbbr");
    _sohist->hideColumn("baseunitprice");
    _sohist->hideColumn("baseextprice");
  }

  if (_showCosts->isChecked())
  {
    _sohist->showColumn("cohist_unitcost");
    _sohist->showColumn("extcost");
  }
  else
  {
    _sohist->hideColumn("cohist_unitcost");
    _sohist->hideColumn("extcost");
  }
}

void dspSalesHistoryByBilltoName::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("EditSalesHistory"))
    menuItem->setEnabled(false);

  pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void dspSalesHistoryByBilltoName::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspSalesHistoryByBilltoName::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSalesHistoryByBilltoName::sPrint()
{
  if(!checkParameters())
    return;

  ParameterList params;
  params.append("billToName", _billtoName->text().toUpper().trimmed());

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("includeFormatted");

  if(_showCosts->isChecked())
      params.append("showCosts");
  if(_showPrices->isChecked())
      params.append("showPrices");

  orReport report("SalesHistoryByBilltoName", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSalesHistoryByBilltoName::sFillList()
{
  if (!checkParameters())
    return;

  _sohist->clear();
  
  MetaSQLQuery mql = mqlLoad("salesHistory", "detail");
  ParameterList params;
  _dates->appendValue(params);
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  params.append("billToName", _billtoName->text());
  params.append("credit", tr("Credit"));
  params.append("return", tr("Return"));
  params.append("orderByInvcdateBillto");
  q = mql.toQuery(params);
  _sohist->populate(q);
}

bool dspSalesHistoryByBilltoName::checkParameters()
{
  if (isVisible())
  {
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
