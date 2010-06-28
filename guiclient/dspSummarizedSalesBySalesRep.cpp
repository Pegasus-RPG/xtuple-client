/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesBySalesRep.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "dspSalesHistoryBySalesrep.h"
#include "mqlutil.h"

/*
 *  Constructs a dspSummarizedSalesBySalesRep as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesBySalesRep::dspSummarizedSalesBySalesRep(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _sohist->addColumn(tr("Sales Rep."),  -1,              Qt::AlignLeft,   true,  "rep"   );
  _sohist->addColumn(tr("First Sale"),  _dateColumn,     Qt::AlignCenter, true,  "firstdate" );
  _sohist->addColumn(tr("Last Sale"),   _dateColumn,     Qt::AlignCenter, true,  "lastdate" );
  _sohist->addColumn(tr("Total Units"), _qtyColumn,      Qt::AlignRight,  true,  "totalunits"  );
  _sohist->addColumn(tr("Total Sales"), _bigMoneyColumn, Qt::AlignRight,  true,  "totalsales"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesBySalesRep::~dspSummarizedSalesBySalesRep()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesBySalesRep::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesBySalesRep::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View Sales Detail..."), this, SLOT(sViewDetail()), 0);
}

void dspSummarizedSalesBySalesRep::sViewDetail()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _dates->appendValue(params);
  params.append("salesrep_id", _sohist->id());
  params.append("run");

  dspSalesHistoryBySalesrep *newdlg = new dspSalesHistoryBySalesrep();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedSalesBySalesRep::sPrint()
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

  orReport report("SummarizedSalesHistoryBySalesRep", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedSalesBySalesRep::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  MetaSQLQuery mql = mqlLoad("summarizedSalesHistory", "detail");
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);
  _sohist->populate(q);
}

bool dspSummarizedSalesBySalesRep::checkParameters()
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

  return TRUE;
}

void dspSummarizedSalesBySalesRep::setParams(ParameterList & params)
{
  if (_productCategory->isSelected())
    params.append("prodcat_id", _productCategory->id());
  else if (_productCategory->isPattern())
    params.append("prodcat_pattern", _productCategory->pattern());

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());

  params.append("bySalesRep");
}
