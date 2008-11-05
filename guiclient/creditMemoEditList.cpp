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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "creditMemoEditList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "selectOrderForBilling.h"
#include "selectBillingQty.h"
#include "printInvoices.h"
#include "creditMemo.h"
#include "creditMemoItem.h"

creditMemoEditList::creditMemoEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cmhead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cmhead->addColumn(tr("Document #"), (_itemColumn+_cmhead->indentation()), Qt::AlignLeft,  true, "docnumber");
  _cmhead->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,  true, "ordernumber");
  _cmhead->addColumn(tr("Cust./Item #"), _itemColumn, Qt::AlignLeft,  true, "custitemnumber");
  _cmhead->addColumn(tr("Name/Description"),      -1, Qt::AlignLeft,  true, "namedescrip");
  _cmhead->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "uom_name");
  _cmhead->addColumn(tr("Qty. to Bill"),  _qtyColumn, Qt::AlignRight, true, "qtytobill");
  _cmhead->addColumn(tr("Price"),        _costColumn, Qt::AlignRight, true, "price");
  _cmhead->addColumn(tr("Ext. Price"),  _moneyColumn, Qt::AlignRight, true, "extprice");

  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));

  sFillList();
}

creditMemoEditList::~creditMemoEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemoEditList::languageChange()
{
  retranslateUi(this);
}

void creditMemoEditList::sEditCreditMemo()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void creditMemoEditList::sEditCreditMemoItem()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmitem_id", _cmhead->altId());

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void creditMemoEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  _orderid = _cmhead->id();

  pMenu->insertItem("Edit Credit Memo...", this, SLOT(sEditCreditMemo()), 0);

  if (((XTreeWidgetItem *)pSelected)->altId() != -1)
    pMenu->insertItem("Edit Credit Memo Item...", this, SLOT(sEditCreditMemoItem()), 0);
}

void creditMemoEditList::sFillList()
{
  q.exec("SELECT *,"
         "       ('C/M-' || formatCreditMemoNumber(cmhead_id)) AS docnumber,"
         "       CASE WHEN seq != 0 THEN ''"
         "       END AS docnumber_qtdisplayrole,"
         "       'qty' AS qtytobill_xtnumericrole,"
         "       'price' AS price_xtnumericrole,"
         "       'curr' AS extprice_xtnumericrole,"
         "       CASE WHEN namedescrip = 'Not Assigned' THEN 'error'"
         "       END AS qtforegroundrole "
         "FROM ("
         "SELECT cmhead_id, -1 AS altid,"
         "       cmhead_number AS ordernumber,"
         "       cust_number AS custitemnumber,"
         "       cmhead_billtoname AS namedescrip,"
         "       CAST(NULL AS TEXT) AS uom_name,"
         "       CAST(NULL AS NUMERIC) AS qtytobill,"
         "       CAST(NULL AS NUMERIC) AS price,"
         "       CAST(NULL AS NUMERIC) AS extprice,"
         "       0 AS cmitem_linenumber, 0 AS xtindentrole, 0 AS seq "
         "FROM cmhead JOIN custinfo ON (cmhead_cust_id=cust_id) "
         "WHERE ((NOT cmhead_posted)"
         "  AND  (NOT cmhead_hold)) "
         "UNION "
         "SELECT cmhead_id, cmitem_id,"
         "       NULL, item_number,"
         "       item_descrip1,"
         "       uom_name, cmitem_qtycredit * cmitem_qty_invuomratio,"
         "       cmitem_unitprice / cmitem_price_invuomratio,"
         "       cmitem_qtycredit * cmitem_qty_invuomratio *"
         "                         cmitem_unitprice / cmitem_price_invuomratio,"
         "       cmitem_linenumber, 1 AS xtindentrole, 1 AS seq "
         "FROM cmhead, cmitem, itemsite, item, uom "
         "WHERE ((NOT cmhead_posted)"
         "   AND (NOT cmhead_hold)"
         "   AND (cmhead_id=cmitem_cmhead_id)"
         "   AND (cmitem_itemsite_id=itemsite_id)"
         "   AND (itemsite_item_id=item_id)"
         "   AND (item_inv_uom_id=uom_id)) "
         "UNION "
         "SELECT cmhead_id, -1,"
         "       NULL, 'Debit',"
         "       COALESCE((SELECT formatGLAccountLong(accnt_id)"
         "                 FROM accnt, salesaccnt"
         "                 WHERE ((salesaccnt_sales_accnt_id=accnt_id)"
         "                    AND (salesaccnt_id=findSalesAccnt(itemsite_id, "
         "                               cmhead_cust_id)))), 'Not Assigned'),"
         "       NULL, NULL, NULL,"
         "       cmitem_qtycredit * cmitem_qty_invuomratio *"
         "                         cmitem_unitprice / cmitem_price_invuomratio,"
         "       cmitem_linenumber, 2 AS xtindentrole, 2 AS seq "
         "FROM cmhead, cmitem, itemsite, item, uom "
         "WHERE ((NOT cmhead_posted)"
         "   AND (NOT cmhead_hold)"
         "   AND (cmhead_id=cmitem_cmhead_id)"
         "   AND (cmitem_itemsite_id=itemsite_id)"
         "   AND (itemsite_item_id=item_id)"
         "   AND (item_inv_uom_id=uom_id)) "

         "UNION "
         "SELECT cmhead_id, -1,"
         "       NULL, 'Debit',"
         "       CASE WHEN (accnt_id IS NULL) THEN 'Not Assigned'"
         "            ELSE formatGLAccountLong(accnt_id)"
         "       END,"
         "       NULL, NULL, cmhead_freight,"
         "       cmhead_freight,"
         "       99999, 1 AS xtindentrole, 3 AS seq "
         "FROM cmhead LEFT OUTER JOIN"
         "     accnt ON (accnt_id=findFreightAccount(cmhead_cust_id)) "
         "WHERE ((NOT cmhead_posted)"
         "   AND (NOT cmhead_hold)"
         "   AND (cmhead_freight <> 0))"

         "UNION "
         "SELECT cmhead_id, -1,"
         "       NULL, 'Debit',"
         "       formatGLAccountLong(cmhead_misc_accnt_id),"
         "       NULL, NULL, cmhead_misc,"
         "       cmhead_misc,"
         "       99999, 1 AS xtindentrole, 4 AS seq "
         "FROM cmhead LEFT OUTER JOIN"
         "     accnt ON (accnt_id=findFreightAccount(cmhead_cust_id)) "
         "WHERE ((NOT cmhead_posted)"
         "   AND (NOT cmhead_hold)"
         "   AND (cmhead_misc <> 0))"

         "UNION "
         "SELECT cmhead_id, -1,"
         "       NULL, 'Debit',"
         "       CASE WHEN (tax_sales_accnt_id IS NULL) THEN 'Taxes'"
         "            ELSE formatGLAccountLong(tax_sales_accnt_id)"
         "       END,"
         "       NULL, NULL, cmhead_tax,"
         "       cmhead_tax,"
         "       99999, 1 AS xtindentrole, 5 AS seq "
         "FROM cmhead LEFT OUTER JOIN"
         "     tax ON (cmhead_tax_id=tax_id)"
         "WHERE ((NOT cmhead_posted)"
         "   AND (NOT cmhead_hold)"
         "   AND (cmhead_tax <> 0))"

         "UNION "
         "SELECT cmhead_id, -1,"
         "       NULL, 'Credit',"
         "       CASE WHEN (accnt_id IS NULL) THEN 'Not Assigned'"
         "            ELSE formatGLAccountLong(accnt_id)"
         "       END,"
         "       NULL, NULL, NULL,"
         "       SUM((cmitem_qtycredit * cmitem_qty_invuomratio) *"
         "                  (cmitem_unitprice / cmitem_price_invuomratio)) +"
         "           cmhead_freight + cmhead_misc + cmhead_tax AS extprice,"
         "       99999, 1 AS xtindentrole, 6 AS seq "
         "FROM cmhead LEFT OUTER JOIN"
         "     cmitem ON (cmhead_id=cmitem_cmhead_id) LEFT OUTER JOIN"
         "     accnt ON (accnt_id=findARAccount(cmhead_cust_id))"
         "WHERE ((NOT cmhead_posted)"
         "  AND  (NOT cmhead_hold)) "
         "GROUP BY cmhead_id, cmhead_cust_id, cmhead_freight, cmhead_misc,"
         "         cmhead_tax, accnt_id "
         ") AS dummy "
         "ORDER BY docnumber, cmitem_linenumber, seq;"
        );
  _cmhead->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void creditMemoEditList::sPrint()
{
  orReport report("CreditMemoEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

bool creditMemoEditList::checkSitePrivs(int ordid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkCreditMemoSitePrivs(:cmheadid) AS result;");
    check.bindValue(":cmheadid", ordid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view or edit this Credit Memo as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
