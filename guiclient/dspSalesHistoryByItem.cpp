/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspSalesHistoryByItem.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <qworkspace.h>
#include <qmessagebox.h>
#include "salesHistoryInformation.h"
#include "rptSalesHistoryByItem.h"

/*
 *  Constructs a dspSalesHistoryByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSalesHistoryByItem::dspSalesHistoryByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_sohist, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_showCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
    connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandleParams()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSalesHistoryByItem::~dspSalesHistoryByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSalesHistoryByItem::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspSalesHistoryByItem::init()
{
  statusBar()->hide();

  _item->setType(ItemLineEdit::cSold);
  _customerType->setType(CustomerType);

  _sohist->addColumn(tr("Customer"),   -1,           Qt::AlignLeft   );
  _sohist->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Ord. Date"),  _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignRight  );
  _sohist->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter );
  _sohist->addColumn(tr("Shipped"),    _qtyColumn,   Qt::AlignRight  );

  _showCosts->setEnabled(_privleges->check("ViewCosts"));
  _showPrices->setEnabled(_privleges->check("ViewCustomerPrices"));

  _item->setFocus();
}

enum SetResponse dspSalesHistoryByItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  param = pParams.value("custtype_id", &valid);
  if (valid)
    _customerType->setId(param.toInt());

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
    _customerType->setPattern(param.toString());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

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

void dspSalesHistoryByItem::sHandleParams()
{
  while (_sohist->columns() > 6)
    _sohist->removeColumn(6);

  if (_showPrices->isChecked())
  {
    _sohist->addColumn( tr("Unit Price"), _priceColumn,    Qt::AlignRight );
    _sohist->addColumn( tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight );
  }

  if (_showCosts->isChecked())
  {
    _sohist->addColumn( tr("Unit Cost"), _costColumn, Qt::AlignRight );
    _sohist->addColumn( tr("Ext. Cost"), _costColumn, Qt::AlignRight );
  }
}

void dspSalesHistoryByItem::sPopulateMenu(Q3PopupMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("EditSalesHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspSalesHistoryByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void dspSalesHistoryByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohist_id", _sohist->id());

  salesHistoryInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSalesHistoryByItem::sPrint()
{
  ParameterList params;
  _customerType->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());
  params.append("print");

  if (_showCosts->isChecked())
    params.append("showCosts");

  if (_showPrices->isChecked())
    params.append("showPrices");

  rptSalesHistoryByItem newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspSalesHistoryByItem::sFillList()
{
  _sohist->clear();

  if (!checkParameters())
    return;

  QString sql( "SELECT cohist_id, cust_name, cohist_ordernumber,"
               "       formatDate(cohist_orderdate) AS f_orderdate,"
               "       CASE WHEN (cohist_invcnumber='-1') THEN 'Credit'"
               "            ELSE TEXT(cohist_invcnumber)"
               "       END AS invoicenumber,"
               "       formatDate(cohist_invcdate, 'Return') AS f_invcdate,"
               "       cohist_qtyshipped, formatQty(cohist_qtyshipped) AS f_shipped " );

  if (_showPrices->isChecked())
    sql += ", formatSalesPrice(cohist_unitprice) AS f_price,"
           "  round(cohist_qtyshipped * cohist_unitprice, 2) AS extprice,"
           "  formatMoney(round(cohist_qtyshipped * cohist_unitprice, 2)) AS f_extprice ";

  if (_showCosts->isChecked())
    sql += ", formatSalesPrice(cohist_unitcost) AS f_cost,"
           "  (cohist_qtyshipped * cohist_unitcost) AS extcost,"
           "  formatMoney(cohist_qtyshipped * cohist_unitcost) AS f_extcost ";

  sql += "FROM cohist, cust, itemsite, item "
         "WHERE ( (cohist_itemsite_id=itemsite_id)"
         " AND (cohist_cust_id=cust_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (item_id=:item_id)"
         " AND (cohist_invcdate BETWEEN :startDate AND :endDate) ";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_customerType->isSelected())
    sql += " AND (cust_custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ :custtype_pattern)))";

  sql += ") "
         "ORDER BY cohist_invcdate, cust_number; ";

  q.prepare(sql);
  _customerType->bindValue(q);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    double totalSales = 0.0;
    double totalUnits = 0.0;
    double totalCost  = 0.0;

    do
    {
      XListViewItem *last = new XListViewItem( _sohist, _sohist->lastItem(),
                                               q.value("cohist_id").toInt(),
                                               q.value("cust_name"), q.value("cohist_ordernumber"),
                                               q.value("f_orderdate"), q.value("invoicenumber"),
                                               q.value("f_invcdate"), q.value("f_shipped") );

      if (_showPrices->isChecked())
      {
        last->setText(6, q.value("f_price"));
        last->setText(7, q.value("f_extprice"));

        if (_showCosts->isChecked())
        {
          last->setText(8, q.value("f_cost"));
          last->setText(9, q.value("f_extcost"));
        }
      }
      else if (_showCosts->isChecked())
      {
        last->setText(6, q.value("f_cost"));
        last->setText(7, q.value("f_extcost"));
      }
 
      totalUnits += q.value("cohist_qtyshipped").toDouble();
      totalSales += q.value("extprice").toDouble();
      totalCost  += q.value("extcost").toDouble();
    }
    while (q.next());

    XListViewItem *totals = new XListViewItem(_sohist, _sohist->lastItem(), -1, QVariant(tr("Totals")));
    totals->setText(5, formatQty(totalUnits));

    if (_showPrices->isChecked())
    {
      totals->setText(7, formatMoney(totalSales));

      if (_showCosts->isChecked())
        totals->setText(9, formatMoney(totalCost));
    }
    else if (_showCosts->isChecked())
      totals->setText(7, formatMoney(totalSales));
  }
}

bool dspSalesHistoryByItem::checkParameters()
{
  if (isVisible())
  {
    if (!_item->isValid())
    {
      QMessageBox::warning( this, tr("Enter Item Number"),
                            tr("Please enter a valid Item Number.") );
      _item->setFocus();
      return FALSE;
    }

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

