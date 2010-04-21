/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoPriceVariancesByItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "mqlutil.h"

/*
 *  Constructs a dspPoPriceVariancesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoPriceVariancesByItem::dspPoPriceVariancesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _porecv->addColumn(tr("P/O #"),              _orderColumn,    Qt::AlignRight,  true,  "porecv_ponumber"  );
  _porecv->addColumn(tr("Dist. Date"),         _dateColumn,     Qt::AlignCenter, true,  "distdate" );
  _porecv->addColumn(tr("Recv. Date"),         _dateColumn,     Qt::AlignCenter, false, "receivedate" );
  _porecv->addColumn(tr("Vendor Number"),      _itemColumn,     Qt::AlignLeft,   true,  "vend_number"   );
  _porecv->addColumn(tr("Vendor Name"),        -1,              Qt::AlignLeft,   true,  "vend_name"   );
  _porecv->addColumn(tr("Qty."),               _qtyColumn,      Qt::AlignRight,  true,  "porecv_qty"  );
  _porecv->addColumn(tr("Purch. Cost"),        _priceColumn,    Qt::AlignRight,  false, "porecv_purchcost"  );
  if (!omfgThis->singleCurrency())
    _porecv->addColumn(tr("Purch. Curr."),       _priceColumn,    Qt::AlignRight,  false, "poCurrAbbr"  );
  _porecv->addColumn(tr("Rcpt. Cost"),         _priceColumn,    Qt::AlignRight,  false, "porecv_recvcost"  );
  _porecv->addColumn(tr("Received"),           _moneyColumn,    Qt::AlignRight,  true,  "porecv_value"  );
  _porecv->addColumn(tr("Vouch. Cost"),        _priceColumn,    Qt::AlignRight,  false, "vouchercost"  );
  _porecv->addColumn(tr("Vouchered"),          _moneyColumn,    Qt::AlignRight,  true,  "voucher_value"  );
  _porecv->addColumn(tr("Variance"),           _moneyColumn,    Qt::AlignRight,  true,  "variance"  );
  if (!omfgThis->singleCurrency())
    _porecv->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignRight,  true,  "currAbbr"  );
  _porecv->addColumn(tr("%"),                  _prcntColumn,    Qt::AlignRight,  true,  "varprcnt"  );

  _currencyGroup->setHidden(omfgThis->singleCurrency());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoPriceVariancesByItem::~dspPoPriceVariancesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoPriceVariancesByItem::languageChange()
{
  retranslateUi(this);
}

void dspPoPriceVariancesByItem::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  params.append("item_id", _item->id());

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("PurchasePriceVariancesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

bool dspPoPriceVariancesByItem::setParams(ParameterList &pParams)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  if (_notZero->isChecked())
    pParams.append("notZero");

  pParams.append("item_id", _item->id());
  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_baseCurr->isChecked())
    pParams.append("baseCurr");

  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  return true;
}

void dspPoPriceVariancesByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
  {
    _porecv->clear();
    return;
  }
  MetaSQLQuery mql = mqlLoad("poPriceVariances", "detail");
  q = mql.toQuery(params);
  _porecv->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
