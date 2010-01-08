/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedSalesByItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "dspSalesHistoryByItem.h"

/*
 *  Constructs a dspSummarizedSalesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedSalesByItem::dspSummarizedSalesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sohist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _customerType->setType(ParameterGroup::CustomerType);
  _productCategory->setType(ParameterGroup::ProductCategory);

  _sohist->addColumn(tr("Item"),        _itemColumn,      Qt::AlignLeft,   true,  "item_number"   );
  _sohist->addColumn(tr("Description"), -1,               Qt::AlignLeft,   true,  "itemdescription"   );
  _sohist->addColumn(tr("First Sale"),  _dateColumn,      Qt::AlignCenter, true,  "firstdate" );
  _sohist->addColumn(tr("Last Sale"),   _dateColumn,      Qt::AlignCenter, true,  "lastdate" );
  _sohist->addColumn(tr("Total Units"), _qtyColumn,       Qt::AlignRight,  true,  "totalunits"  );
  _sohist->addColumn(tr("Total Sales"), _bigMoneyColumn,  Qt::AlignRight,  true,  "totalsales"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedSalesByItem::~dspSummarizedSalesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedSalesByItem::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedSalesByItem::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View Sales Detail..."), this, SLOT(sViewDetail()), 0);
}

void dspSummarizedSalesByItem::sViewDetail()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _productCategory->appendValue(params);
  _customerType->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _sohist->id());
  params.append("run");

  dspSalesHistoryByItem *newdlg = new dspSalesHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSummarizedSalesByItem::sPrint()
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
  _customerType->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  orReport report("SummarizedSalesHistoryByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedSalesByItem::sFillList()
{
  if (!checkParameters())
    return;

  QString sql( "SELECT itemsite_item_id, item_number, itemdescription,"
               "       MIN(cohist_invcdate) AS firstdate,"
               "       MAX(cohist_invcdate) AS lastdate,"
               "       SUM(cohist_qtyshipped) AS totalunits,"
               "       SUM(baseextprice) AS totalsales,"
               "       'qty' AS totalunits_xtnumericrole,"
               "       'curr' AS totalsales_xtnumericrole,"
               "       0 AS totalunits_xttotalrole,"
               "       0 AS totalsales_xttotalrole "
               "FROM saleshistory "
               "WHERE ( (cohist_invcdate BETWEEN :startDate AND :endDate)" );

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  if (_customerType->isSelected())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY itemsite_item_id, item_number, itemdescription ";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerType->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  _sohist->populate(q);
}

bool dspSummarizedSalesByItem::checkParameters()
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

  return TRUE;
}

