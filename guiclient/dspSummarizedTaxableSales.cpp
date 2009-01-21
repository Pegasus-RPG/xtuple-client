/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedTaxableSales.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "currdisplay.h"
#include "mqlutil.h"

dspSummarizedTaxableSales::dspSummarizedTaxableSales(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _dates->setStartCaption(tr("Start Ship Date:"));
  _dates->setEndCaption(tr("End Ship Date:"));

  _taxCode->setType(XComboBox::TaxTypes);

  QString base = CurrDisplay::baseCurrAbbr();
  _invchead->addColumn(tr("Tax Code"),            _itemColumn,     Qt::AlignLeft,   true,  "tax_code"   );
  _invchead->addColumn(tr("Description"),         -1,              Qt::AlignLeft,   true,  "tax_descrip"   );
  _invchead->addColumn(tr("Sales %1").arg(base),  _itemColumn,     Qt::AlignRight,  true,  "salesbase"  );
  _invchead->addColumn(tr("Freight %1").arg(base),_moneyColumn,    Qt::AlignRight,  true,  "freightbase"  );
  _invchead->addColumn(tr("Freight Taxed"),       _itemColumn,     Qt::AlignCenter, true,  "freighttax"  );
  _invchead->addColumn(tr("Tax %1").arg(base),    _moneyColumn,    Qt::AlignRight,  true,  "taxbase"  );
  _invchead->addColumn(tr("Tax"),                 _moneyColumn,    Qt::AlignRight,  true,  "tax"  );
  _invchead->addColumn(tr("Currency"),            _currencyColumn, Qt::AlignRight,  true,  "currAbbr"  );
}

dspSummarizedTaxableSales::~dspSummarizedTaxableSales()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSummarizedTaxableSales::languageChange()
{
  retranslateUi(this);
}

bool dspSummarizedTaxableSales::setParams(ParameterList &params)
{
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

  _dates->appendValue(params);

  if (_selectedTaxCode->isChecked())
    params.append("tax_id", _taxCode->id());

  return true;
}

void dspSummarizedTaxableSales::sPrint()
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

void dspSummarizedTaxableSales::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  _invchead->clear();

  MetaSQLQuery mql = mqlLoad("summarizedTaxableSales", "detail");
  q = mql.toQuery(params);
  q.exec();
  _invchead->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
