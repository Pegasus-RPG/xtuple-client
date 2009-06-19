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
  
  QString base = CurrDisplay::baseCurrAbbr();
  _taxdet->addColumn(tr("Doc#"),                _orderColumn,    Qt::AlignLeft,  true,  "docnumber"   );
  _taxdet->addColumn(tr("Doc. Type"),           _orderColumn,    Qt::AlignLeft,  true,  "doctype"   );
  _taxdet->addColumn(tr("Order#"),              _orderColumn,    Qt::AlignLeft,  true,  "ordernumber"   );
  _taxdet->addColumn(tr("Doc. Date"),           _dateColumn,     Qt::AlignLeft,  true,  "docdate"  );
  _taxdet->addColumn(tr("Dist. Date"),          _dateColumn,     Qt::AlignLeft,  false, "taxhist_distdate"  );   
  _taxdet->addColumn(tr("Name"),                -1,              Qt::AlignLeft,  false, "name"  );
  _taxdet->addColumn(tr("Tax Code"),            _itemColumn,     Qt::AlignLeft,  true,  "tax"  );
  _taxdet->addColumn(tr("Tax Type"),            _itemColumn,     Qt::AlignLeft,  false, "taxtype"  );
  _taxdet->addColumn(tr("Tax Zone"),            _itemColumn,     Qt::AlignLeft,  false, "taxzone"  );
  _taxdet->addColumn(tr("Tax Class"),           _itemColumn,     Qt::AlignLeft,  false, "taxclass"  );
  _taxdet->addColumn(tr("Tax Authority"),       _itemColumn,     Qt::AlignLeft,  false, "taxauth"   );
  _taxdet->addColumn(tr("Item#"),               _itemColumn,     Qt::AlignLeft,  true,  "item_number"  );
  _taxdet->addColumn(tr("Description"),         -1,              Qt::AlignLeft,  true,  "description"  );
  _taxdet->addColumn(tr("Qty"),                 _qtyColumn,      Qt::AlignRight, false, "qty"  );
  _taxdet->addColumn(tr("Unit Price"),          _moneyColumn,    Qt::AlignRight, false, "unitprice"  );
  _taxdet->addColumn(tr("Extension"),           _moneyColumn,    Qt::AlignRight, false, "extension"  );
  _taxdet->addColumn(tr("Tax"),                 _moneyColumn,    Qt::AlignRight, true,  "tax_local"  );
  _taxdet->addColumn(tr("Currency"),            _currencyColumn, Qt::AlignRight, true,  "curr_abbr"  ); 
  _taxdet->addColumn(tr("Tax %1").arg(base),    _moneyColumn,    Qt::AlignRight,  true,  "tax_base"  );

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
  
  if (((_showOnlyGroup->isCheckable() && _showOnlyGroup->isChecked()) || 
       !_showOnlyGroup->isCheckable()) &&
        _filterOn->id() != -1)
  {
    switch (_filterOn->currentIndex())
    {
      case 0:
        params.append("tax_id",_selection->id());
        params.append("groupBy","taxcode");
        break;

      case 1:
        params.append("taxtype_id",_selection->id());
        params.append("groupBy","taxcode");
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
    
  if (_summary->isChecked())
  {
    params.append("summary");
    switch (_filterOn->currentIndex())
    {
      case 0:
        params.append("groupBy","tax");
        break;

      case 1:
        params.append("groupBy","taxtype");
        break;

      case 2:
        params.append("groupBy","taxclass");
        break;
 
      case 3:
        params.append("groupBy","taxauth");
        break;
      
      case 4:
        params.append("groupBy","taxzone");
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

  _taxsum->clear();
  _taxdet->clear();

  MetaSQLQuery mql = mqlLoad("taxHistory", "detail");
  q = mql.toQuery(params);
  q.exec();
  if (_summary->isChecked())
    _taxsum->populate(q);
  else
    _taxdet->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTaxHistory::sHandleFilter()
{ 
  _selection->setType(XComboBox::Adhoc); // Force to re-evaluate if null state changed
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
  if (_summary->isChecked())
  {
    QString base = CurrDisplay::baseCurrAbbr();
    _taxsum->clear();
    _taxsum->setColumnCount(0);
    
    switch (_filterOn->currentIndex())
    {
      case 0:
        _taxsum->addColumn(tr("Tax Code"), _itemColumn,  Qt::AlignLeft, true, "tax"   );
        break;

      case 1:
        _taxsum->addColumn(tr("Tax Type"), _itemColumn,  Qt::AlignLeft, true, "taxtype"   );
        break;

      case 2:
        _taxsum->addColumn(tr("Tax Class"), _itemColumn,  Qt::AlignLeft, true, "taxclass"   );
        break;
 
      case 3:
        _taxsum->addColumn(tr("Tax Authority"), _itemColumn,  Qt::AlignLeft, true, "taxauth"   );
        break;
      
      case 4:
        _taxsum->addColumn(tr("Tax Zone"), _itemColumn,  Qt::AlignLeft, true, "taxzone"   );
        break;
    }

    _taxsum->addColumn(tr("Description"),         -1,              Qt::AlignLeft,   true,  "description"   );
    _taxsum->addColumn(tr("Sales %1").arg(base),  _itemColumn,     Qt::AlignRight,  true,  "salesbase"  );
    _taxsum->addColumn(tr("Sales Freight %1").arg(base),_moneyColumn,    Qt::AlignRight,  true,  "freightbase"  );
    _taxsum->addColumn(tr("Freight Taxed"),       _itemColumn,     Qt::AlignCenter, true,  "freighttax"  );
    _taxsum->addColumn(tr("Sales Tax %1").arg(base), _moneyColumn,    Qt::AlignRight,  true,  "salestaxbase"  );
    _taxsum->addColumn(tr("Purchases %1").arg(base),  _itemColumn,     Qt::AlignRight,  true,  "purhasebase"  );
    _taxsum->addColumn(tr("Purchase Tax %1").arg(base), _moneyColumn,    Qt::AlignRight,  true,  "purchasetaxbase"  );
    _taxsum->addColumn(tr("Net Tax %1").arg(base),_moneyColumn,    Qt::AlignRight,  true,  "nettaxbase"  );
  }
}

void dspTaxHistory::sHandleType()
{ 
  if (_summary->isChecked())
  {
    _showOnlyGroup->setTitle(tr("Summarize by"));
    _showOnlyGroup->setCheckable(false);
    _showOnlyGroup->setEnabled(true);
    _taxdet->hide();
    _taxsum->show();
    _selection->setAllowNull(true);
    _selectionLit->setText(tr("Filter on:"));
  }
  else
  {
    _showOnlyGroup->setTitle(tr("Show only tax"));
    _selectionLit->setText(tr("Of"));
    _showOnlyGroup->setCheckable(true);
    _showOnlyGroup->setChecked(false);
    _taxsum->hide();
    _taxdet->show();
    _selection->setAllowNull(false);
  }
  sHandleFilter();
}


