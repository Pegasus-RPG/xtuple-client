/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTaxHistory.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "currdisplay.h"
#include "mqlutil.h"

dspTaxHistory::dspTaxHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_filterOn, SIGNAL(currentIndexChanged(int)), this, SLOT(sHandleFilter()));
  connect(_showOnlyGroup, SIGNAL(toggled(bool)), this, SLOT(sHandleFilter()));
  connect(_summary, SIGNAL(toggled(bool)), this, SLOT(sHandleType()));

  sHandleType();
}

dspTaxHistory::~dspTaxHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTaxHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspTaxHistory::set(const ParameterList &pParams)
{
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


bool dspTaxHistory::setParams(ParameterList &params)
{
  if (!(_sales->isChecked() || _purchases->isChecked()))
  {
    QMessageBox::warning( this, tr("Select items to show"),
                          tr("You must select sales or purchase items to show.") );
    return false;
  }
  
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Start Date"),
                          tr("You must enter a valid Start Date to print this report.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Valid End Date"),
                          tr("You must enter a valid End Date to print this report.") );
    _dates->setFocus();
    return false;
  }
  
  if (_summary->isChecked())
    params.append("type", "summary");
  else
    params.append("type", "detail");
    
  if (_sales->isChecked())
    params.append("showSales");
  if (_purchases->isChecked())
    params.append("showPurchases");
    
  if (_distDate->isChecked())
    params.append("distDate");

  _dates->appendValue(params);
  
  if (_showOnlyGroup->isChecked())
  {
    switch (_filterOn->currentIndex())
    {
      case 0:
        params.append("tax_id",_selection->id());
        break;

      case 1:
        params.append("taxtype_id",_selection->id());
        break;

      case 2:
        params.append("taxclass_id",_selection->id());
        break;
 
      case 3:
        params.append("taxauth_id",_selection->id());
        break;
      
      case 4:
        params.append("taxzone_id",_selection->id());
        break;
    }
  }
  params.append("invoice",tr("Invoice"));
  params.append("creditmemo",tr("Credit Memo"));
  params.append("debitmemo",tr("Debit Memo"));
  params.append("other",tr("Other"));
  params.append("none",tr("None"));

  return true;
}

void dspTaxHistory::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("SummarizedTaxableSales", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspTaxHistory::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  _taxhist->clear();

  MetaSQLQuery mql = mqlLoad("taxHistory", "detail");
  q = mql.toQuery(params);
  q.exec();
  _taxhist->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTaxHistory::sHandleFilter()
{
  switch (_filterOn->currentIndex())
  {
    case 0:
      _selection->setType(XComboBox::TaxCodes);
      break;

    case 1:
      _selection->setType(XComboBox::TaxTypes);
      break;

    case 2:
      _selection->setType(XComboBox::TaxClasses);
      break;
 
    case 3:
      _selection->setType(XComboBox::TaxAuths);
      break;
      
    case 4:
      _selection->setType(XComboBox::TaxZones);
      break;
  }
}

void dspTaxHistory::sHandleType()
{
  _taxhist->clear();
  _taxhist->setColumnCount(0);
  
  if (_summary->isChecked())
  {
  
  }
  else
  {
    QString base = CurrDisplay::baseCurrAbbr();
    _taxhist->addColumn(tr("Doc#"),                _orderColumn,    Qt::AlignLeft,  true,  "docnumber"   );
    _taxhist->addColumn(tr("Doc. Type"),           _orderColumn,    Qt::AlignLeft,  true,  "doctype"   );
    _taxhist->addColumn(tr("Order#"),              _orderColumn,    Qt::AlignLeft,  true,  "ordernumber"   );
    _taxhist->addColumn(tr("Doc. Date"),           _dateColumn,     Qt::AlignLeft,  true,  "docdate"  );
    _taxhist->addColumn(tr("Dist. Date"),          _dateColumn,     Qt::AlignLeft,  false, "taxhist_distdate"  );   
    _taxhist->addColumn(tr("Name"),                -1,              Qt::AlignLeft,  false, "name"  );
    _taxhist->addColumn(tr("Tax Code"),            _itemColumn,     Qt::AlignLeft,  true,  "tax_code"  );
    _taxhist->addColumn(tr("Tax Type"),            _itemColumn,     Qt::AlignLeft,  false, "taxtype"  );
    _taxhist->addColumn(tr("Tax Zone"),            _itemColumn,     Qt::AlignLeft,  false, "taxzone"  );
    _taxhist->addColumn(tr("Tax Class"),           _itemColumn,     Qt::AlignLeft,  false, "taxclass"  );
    _taxhist->addColumn(tr("Tax Authority"),       _itemColumn,     Qt::AlignLeft,  false, "taxauth"   );
    _taxhist->addColumn(tr("Item#"),               _itemColumn,     Qt::AlignLeft,  true,  "item_number"  );
    _taxhist->addColumn(tr("Description"),         -1,              Qt::AlignLeft,  true,  "description"  );
    _taxhist->addColumn(tr("Qty"),                 _qtyColumn,      Qt::AlignRight, false, "qty"  );
    _taxhist->addColumn(tr("Unit Price"),          _moneyColumn,    Qt::AlignRight, false, "unitprice"  );
    _taxhist->addColumn(tr("Extension"),           _moneyColumn,    Qt::AlignRight, false, "extension"  );
    _taxhist->addColumn(tr("Tax"),                 _moneyColumn,    Qt::AlignRight, true,  "tax_local"  );
    _taxhist->addColumn(tr("Currency"),            _currencyColumn, Qt::AlignRight, true,  "curr_abbr"  ); 
    _taxhist->addColumn(tr("Tax %1").arg(base),    _moneyColumn,    Qt::AlignLeft,  true,  "tax_base"  );
  }
}


