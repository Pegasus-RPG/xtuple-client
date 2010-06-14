/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistoryByCustomerType.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

#include <metasql.h>
#include "mqlutil.h"

/*
 *  Constructs a dspBriefSalesHistoryByCustomerType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBriefSalesHistoryByCustomerType::dspBriefSalesHistoryByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _customerType->setType(ParameterGroup::CustomerType);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _sohist->addColumn(tr("Cust. Type"),  _orderColumn,    Qt::AlignLeft,   true,  "custtype_code"   );
  _sohist->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _sohist->addColumn(tr("Doc. #"),      _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _sohist->addColumn(tr("Invoice #"),   _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  _sohist->addColumn(tr("Ord. Date"),   _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  _sohist->addColumn(tr("Invc. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _sohist->addColumn(tr("Total"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extended"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBriefSalesHistoryByCustomerType::~dspBriefSalesHistoryByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBriefSalesHistoryByCustomerType::languageChange()
{
  retranslateUi(this);
}

void dspBriefSalesHistoryByCustomerType::sPrint()
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
  _customerType->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  orReport report("BriefSalesHistoryByCustomerType", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefSalesHistoryByCustomerType::sFillList()
{
  _sohist->clear();
  MetaSQLQuery mql = mqlLoad("briefSalesHistory", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _sohist->populate(q);
}

bool dspBriefSalesHistoryByCustomerType::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid())
  {
    if(isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }
  else if (!_dates->endDate().isValid())
  {
    if(isVisible())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return FALSE;
  }
  else
    _dates->appendValue(params);

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  if (_customerType->isSelected())
    params.append("custtype_id", _customerType->id());
  else if (_customerType->isPattern())
    _customerType->appendValue(params); 

  return TRUE;
}
