/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTaxReturn.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "currdisplay.h"
#include "mqlutil.h"

dspTaxReturn::dspTaxReturn(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_summary, SIGNAL(toggled(bool)), this, SLOT(sHandleType()));
  connect(_sales, SIGNAL(toggled(bool)), this, SLOT(sHandleType()));
  connect(_purchases, SIGNAL(toggled(bool)), this, SLOT(sHandleType()));
  
  QString base = CurrDisplay::baseCurrAbbr();
  _taxdet->addColumn(tr("Doc#"),                _orderColumn,    Qt::AlignLeft,  true,  "docnumber"   );
  _taxdet->addColumn(tr("Source"),              _orderColumn,    Qt::AlignLeft,  true,  "source"   );
  _taxdet->addColumn(tr("Doc. Type"),           _orderColumn,    Qt::AlignLeft,  true,  "doctype"   );
  _taxdet->addColumn(tr("Order#"),              _orderColumn,    Qt::AlignLeft,  false, "ordernumber"   );
  _taxdet->addColumn(tr("Doc. Date"),           _dateColumn,     Qt::AlignCenter,true,  "docdate"  );
  _taxdet->addColumn(tr("Dist. Date"),          _dateColumn,     Qt::AlignCenter,false, "taxhist_distdate"  );   
  _taxdet->addColumn(tr("Journal#"),            _orderColumn,    Qt::AlignRight, false, "taxhist_journalnumber"  );
  _taxdet->addColumn(tr("Name"),                _orderColumn,    Qt::AlignLeft,  false, "name"  );
  _taxdet->addColumn(tr("Tax Code"),            _itemColumn,     Qt::AlignLeft,  true,  "tax"  );
  _taxdet->addColumn(tr("Tax Type"),            _itemColumn,     Qt::AlignLeft,  false, "taxtype"  );
  _taxdet->addColumn(tr("Tax Authority"),       _itemColumn,     Qt::AlignLeft,  false, "taxauth"   );
  _taxdet->addColumn(tr("Item#"),               _itemColumn,     Qt::AlignLeft,  true,  "item_number"  );
  _taxdet->addColumn(tr("Description"),         -1,              Qt::AlignLeft,  true,  "description"  );
  _taxdet->addColumn(tr("Qty"),                 _qtyColumn,      Qt::AlignRight, false, "qty"  );
  _taxdet->addColumn(tr("Unit Price"),          _moneyColumn,    Qt::AlignRight, false, "unitprice"  );
  _taxdet->addColumn(tr("Extension"),           _moneyColumn,    Qt::AlignRight, false, "amount"  );
  _taxdet->addColumn(tr("Tax"),                 _moneyColumn,    Qt::AlignRight, true,  "taxlocal"  );
  _taxdet->addColumn(tr("Currency"),            _currencyColumn, Qt::AlignRight, true,  "curr_abbr"  ); 
  _taxdet->addColumn(tr("Tax %1").arg(base),    _moneyColumn,    Qt::AlignRight,  true, "taxbase"  );

  sHandleType();
}

dspTaxReturn::~dspTaxReturn()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTaxReturn::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspTaxReturn::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "detail")
    _detail->setChecked(true);
  }
  
  param = pParams.value("showSales", &valid);
  if (valid)
    _sales->setChecked(param.toBool());
  
  param = pParams.value("showPurchases", &valid);
  if (valid)
    _purchases->setChecked(param.toBool());
  
  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());
    
  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}


bool dspTaxReturn::setParams(ParameterList &params)
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_taxAuth->isValid(), _taxAuth,
                          tr("You must select a Tax Authority.") );
  errors << GuiErrorCheck(!(_sales->isChecked() || _purchases->isChecked()), _sales,
                          tr("You must select sales or purchase items to show.") );
  errors << GuiErrorCheck(!_dates->startDate().isValid(), _dates,
                          tr("You must enter a valid Start Date to print this report.") );
  errors << GuiErrorCheck(!_dates->endDate().isValid(), _dates,
                          tr("You must enter a valid End Date to print this report.") );

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Execute Tax Return"), errors))
      return false;
  
  if (_metrics->boolean("CashBasedTax"))
    params.append("cashbasedtax", true);
  
  if (_summary->isChecked())
  {
    params.append("type", "summary");
    params.append("summary");
  }
  else
    params.append("type", "detail");
    
  if (_sales->isChecked())
    params.append("showSales");
  if (_purchases->isChecked())
    params.append("showPurchases");

  if (_distDate->isChecked())
    params.append("distDate");

  _dates->appendValue(params);
 
  params.append("taxauth_id",_taxAuth->id());
  params.append("taxauth", _taxAuth->currentText());
    
  params.append("invoice",tr("Invoice"));
  params.append("creditmemo",tr("Credit Memo"));
  params.append("debitmemo",tr("Debit Memo"));
  params.append("other",tr("Other"));
  params.append("none",tr("None"));
  params.append("sales",tr("Sales"));
  params.append("purchase",tr("Purchase"));
  params.append("voucher",tr("Voucher"));
  params.append("check",tr("Misc. Check"));

  return true;
}

void dspTaxReturn::sPrint()
{
  QString name;
  if (_summary->isChecked())
    name="TaxReturnSummary";
  else
    name="TaxReturnDetail";

  ParameterList params;
  if (! setParams(params))
    return;
    
  orReport report(name, params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspTaxReturn::sFillList()
{
  XSqlQuery dspFillList;
  ParameterList params;
  if (! setParams(params))
    return;

  _taxsum->clear();
  _taxdet->clear();

  MetaSQLQuery mql = mqlLoad("taxReturn", "detail");
  dspFillList = mql.toQuery(params);
  dspFillList.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Tax Return detail"),
                           dspFillList, __FILE__, __LINE__))
  {
    return;
  } 
  else 
  {
    if (_summary->isChecked())
      _taxsum->populate(dspFillList);
    else
      _taxdet->populate(dspFillList);
  }
}

void dspTaxReturn::sHandleFilter()
{ 
  QString base = CurrDisplay::baseCurrAbbr();
  _taxsum->clear();
  _taxsum->setColumnCount(0);

  _taxsum->addColumn(tr("Tax Authority"), _itemColumn,  Qt::AlignLeft, true, "taxauth" );
  _taxsum->addColumn(tr("Description"),            -1,  Qt::AlignLeft, true, "description" );

  if (_sales->isChecked())
  {
    _taxsum->addColumn(tr("Sales %1").arg(base),        _bigMoneyColumn,    Qt::AlignRight,  true,  "salesbase"  );
    _taxsum->addColumn(tr("Sales Freight %1").arg(base),_bigMoneyColumn,    Qt::AlignRight,  true,  "freightbase"  );
    _taxsum->addColumn(tr("Sales Tax %1").arg(base),    _bigMoneyColumn,    Qt::AlignRight,  true,  "salestaxbase"  );
  }
  if (_purchases->isChecked())
  {
    _taxsum->addColumn(tr("Purchases %1").arg(base),    _bigMoneyColumn,    Qt::AlignRight,  true,  "purchasebase"  );
    _taxsum->addColumn(tr("Purchases Freight %1").arg(base),_bigMoneyColumn,    Qt::AlignRight,  true,  "freightbase"  );
    _taxsum->addColumn(tr("Purchase Tax %1").arg(base), _bigMoneyColumn,    Qt::AlignRight,  true,  "purchasetaxbase"  );
  }
  if (_sales->isChecked() && _purchases->isChecked())
    _taxsum->addColumn(tr("Net Tax %1").arg(base),      _bigMoneyColumn,    Qt::AlignRight,  true,  "nettaxbase"  );
}

void dspTaxReturn::sHandleType()
{
  if (_summary->isChecked())
  {
    _taxdet->hide();
    _taxsum->show();
    sHandleFilter();
  }
  else
  {
    _taxsum->hide();
    _taxdet->show();
  }
}
