/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistoryByCustomer.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspBriefSalesHistoryByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBriefSalesHistoryByCustomer::dspBriefSalesHistoryByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _sohist->addColumn(tr("Doc. #"),     _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _sohist->addColumn(tr("Cust. P/O #"), -1,             Qt::AlignLeft,   true,  "cohist_ponumber"   );
  _sohist->addColumn(tr("Invoice #"),  _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  _sohist->addColumn(tr("Ord. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  _sohist->addColumn(tr("Invc. Date"), _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _sohist->addColumn(tr("Total"),      _bigMoneyColumn, Qt::AlignRight,  true,  "extended"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBriefSalesHistoryByCustomer::~dspBriefSalesHistoryByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBriefSalesHistoryByCustomer::languageChange()
{
  retranslateUi(this);
}

void dspBriefSalesHistoryByCustomer::sPrint()
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
  params.append("cust_id", _cust->id());

  orReport report("BriefSalesHistoryByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefSalesHistoryByCustomer::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cust_id, cohist_ordernumber, cohist_ponumber, invoicenumber,"
               "       cohist_orderdate, cohist_invcdate,"
               "       SUM(baseextprice) AS extended,"
               "       'curr' AS extended_xtnumericrole,"
               "       0 AS extended_xttotalrole "
               "FROM saleshistory "
               "WHERE ( (cohist_invcdate BETWEEN :startDate AND :endDate)"
               "  AND   (cohist_cust_id=:cust_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  sql += ") "
         "GROUP BY cust_id, cohist_ordernumber, cohist_ponumber, invoicenumber,"
         "         cohist_orderdate, cohist_invcdate "
         "ORDER BY cohist_invcdate, cohist_orderdate;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":cust_id", _cust->id());
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  q.exec();
  _sohist->populate(q);
}

bool dspBriefSalesHistoryByCustomer::checkParameters()
{
    if (!_cust->isValid())
    {
        if(isVisible()) {
            QMessageBox::warning( this, tr("Enter Customer Number"),
                                  tr("Please enter a valid Customer Number.") );
            _cust->setFocus();
        }
        return FALSE;
    }

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

  return TRUE;
}
