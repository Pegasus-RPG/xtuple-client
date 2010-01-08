/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefSalesHistoryBySalesRep.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <openreports.h>

#include "salesHistoryInformation.h"

dspBriefSalesHistoryBySalesRep::dspBriefSalesHistoryBySalesRep(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
  connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));

  _salesrep->setType(XComboBox::SalesRepsActive);
  _productCategory->setType(ParameterGroup::ProductCategory);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _sohist->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _sohist->addColumn(tr("S/O #"),       _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _sohist->addColumn(tr("Invoice #"),   _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"   );
  _sohist->addColumn(tr("Ord. Date"),   _dateColumn,     Qt::AlignCenter, true,  "cohist_orderdate" );
  _sohist->addColumn(tr("Invc. Date"),  _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _sohist->addColumn( tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );
  _sohist->addColumn( tr("Ext. Cost"),  _bigMoneyColumn, Qt::AlignRight,  true,  "extcost"  );

  _showCosts->setEnabled(_privileges->check("ViewCosts"));
  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices"));

  sHandleParams();

  _salesrep->setFocus();
}

dspBriefSalesHistoryBySalesRep::~dspBriefSalesHistoryBySalesRep()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBriefSalesHistoryBySalesRep::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBriefSalesHistoryBySalesRep::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("salesrep_id", &valid);
  if (valid)
    _salesrep->setId(param.toInt());

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

void dspBriefSalesHistoryBySalesRep::sHandleParams()
{
  if (_showPrices->isChecked())
    _sohist->showColumn(5);
  else
    _sohist->hideColumn(5);

  if (_showCosts->isChecked())
    _sohist->showColumn(6);
  else
    _sohist->hideColumn(6);
}

void dspBriefSalesHistoryBySalesRep::sPopulateMenu(QMenu *)
{
}

void dspBriefSalesHistoryBySalesRep::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr("Please enter a valid Start and End Date.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  params.append("salesrep_id", _salesrep->id());

  _productCategory->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_showPrices->isChecked())
    params.append("showPrices");

  orReport report("SalesHistoryBySalesRep", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefSalesHistoryBySalesRep::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cohist_cust_id, cust_name, cohist_ordernumber,"
               "       invoicenumber,"
               "       cohist_orderdate, cohist_invcdate,"
               "       SUM(baseextprice) AS extprice,"
               "       SUM(extcost) AS extcost,"
               "       'curr' AS extprice_xtnumericrole,"
               "       'curr' AS extcost_xtnumericrole,"
               "       0 AS extprice_xttotalrole,"
               "       0 AS extcost_xttotalrole "
               "FROM saleshistory "
               "WHERE ( (cohist_salesrep_id=:salesrep_id)"
               " AND (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (prodcat_code ~ :prodcat_pattern)";

  sql += ") "
         "GROUP BY cohist_cust_id, cust_name, cohist_ordernumber, invoicenumber,"
         "         cohist_orderdate, cohist_invcdate "
         "ORDER BY cohist_invcdate;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  _sohist->populate(q);
}

bool dspBriefSalesHistoryBySalesRep::checkParameters()
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
