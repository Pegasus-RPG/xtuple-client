/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByCustomer.h"

#include <QVariant>
#include <QWorkspace>
//#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "dspSalesHistoryByCustomer.h"
#include "mqlutil.h"

/*
 *  Constructs a dspSummarizedSalesByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesByCustomer::dspSummarizedSalesByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_cohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _productCategory->setType(ParameterGroup::ProductCategory);
  _currency->setType(ParameterGroup::Currency);

  _cohist->addColumn(tr("Customer"),    -1,               Qt::AlignLeft,   true,  "customer"   );
  _cohist->addColumn(tr("First Sale"),  _dateColumn,      Qt::AlignCenter, true,  "firstsale" );
  _cohist->addColumn(tr("Last Sale"),   _dateColumn,      Qt::AlignCenter, true,  "lastsale" );
  _cohist->addColumn(tr("Total Qty."),  _qtyColumn,       Qt::AlignRight,  true,  "qtyshipped"  );
  _cohist->addColumn(tr("Total Sales"), _bigMoneyColumn,  Qt::AlignRight,  true,  "extprice"  );
  _cohist->addColumn(tr("Currency"),    _currencyColumn,  Qt::AlignLeft,   true,  "currAbbr"  );

  if (omfgThis->singleCurrency())
    _cohist->hideColumn(5);
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesByCustomer::~dspSummarizedSalesByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesByCustomer::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesByCustomer::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View Sales Detail..."), this, SLOT(sViewDetail()), 0);
}

void dspSummarizedSalesByCustomer::sViewDetail()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _dates->appendValue(params);
  params.append("cust_id", _cohist->id());
  params.append("run");

  dspSalesHistoryByCustomer *newdlg = new dspSalesHistoryByCustomer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedSalesByCustomer::sPrint()
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }


  ParameterList params;
  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  orReport report("SummarizedSalesHistoryByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedSalesByCustomer::sFillList()
{
  _cohist->clear();

  if (!checkParameters())
    return;

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql = mqlLoad("summarizedSalesHistory", "detail");
  q = mql.toQuery(params);

  _cohist->populate(q);
}

void dspSummarizedSalesByCustomer::setParams(ParameterList & params)
{
  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  if (_productCategory->isSelected())
    params.append("prodcat_id", _productCategory->id());
  else if (_productCategory->isPattern())
    params.append("prodcat_pattern", _productCategory->pattern());

  if (_currency->isSelected())
    params.append("curr_id", _currency->id());
  else if (_currency->isPattern())
    params.append("currConcat_pattern", _currency->pattern());

  params.append("byCustomer");
}

bool dspSummarizedSalesByCustomer::checkParameters()
{
  if (!_dates->startDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  if (!_dates->endDate().isValid())
  {
    if(isVisible()) {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }

  if (_productCategory->isPattern())
  {
    QString pattern = _productCategory->pattern();
    if (pattern.length() == 0)
      return FALSE;
  }

  if (_currency->isPattern())
  {
    QString pattern = _currency->pattern();
    if (pattern.length() == 0)
      return FALSE;
  }

  return TRUE;
}
