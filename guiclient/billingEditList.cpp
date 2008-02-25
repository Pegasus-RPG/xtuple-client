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

#include "billingEditList.h"

#include <QVariant>
#include <QMenu>
#include <QMessageBox>

#include <openreports.h>

#include "selectOrderForBilling.h"
#include "selectBillingQty.h"
#include "printInvoices.h"

billingEditList::billingEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_cobill, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cobill->setRootIsDecorated(TRUE);
  _cobill->addColumn(tr("Document #"),       (_itemColumn+ _cobill->indentation()),  Qt::AlignLeft   );
  _cobill->addColumn(tr("Order #"),          _orderColumn,                            Qt::AlignLeft   );
  _cobill->addColumn(tr("Cust./Item #"),     _itemColumn,                             Qt::AlignLeft   );
  _cobill->addColumn(tr("Name/Description"), -1,                                      Qt::AlignLeft   );
  _cobill->addColumn(tr("UOM"),              _uomColumn,                              Qt::AlignCenter );
  _cobill->addColumn(tr("Qty. to Bill"),     _qtyColumn,                              Qt::AlignRight  );
  _cobill->addColumn(tr("Price"),            _costColumn,                             Qt::AlignRight  );
  _cobill->addColumn(tr("Ext. Price"),       _moneyColumn,                            Qt::AlignRight  );

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

billingEditList::~billingEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void billingEditList::languageChange()
{
  retranslateUi(this);
}

void billingEditList::sEditBillingOrd()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _cobill->id());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void billingEditList::sCancelBillingOrd()
{
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _cobill->id());
    q.exec();

    sFillList();
  }
}

void billingEditList::sEditBillingQty()
{ 
  ParameterList params;
  params.append("soitem_id", _cobill->altId());

  selectBillingQty newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void billingEditList::sCancelBillingQty()
{
}

void billingEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  _orderid = _cobill->id();

  if (((XTreeWidgetItem *)pSelected)->altId() == -2)
  {
    _itemtype = 1;
    pMenu->insertItem("Edit Billing...", this, SLOT(sEditBillingOrd()), 0);
    pMenu->insertItem("Cancel Billing...", this, SLOT(sCancelBillingOrd()), 0);
  }
  else if (((XTreeWidgetItem *)pSelected)->altId() != -1)
  {
    _itemtype = 2;
    pMenu->insertItem("Edit Billing...", this, SLOT(sEditBillingQty()), 0);
    pMenu->insertItem("Cancel Billing...", this, SLOT(sCancelBillingQty()), 0);
  }
}

void billingEditList::sFillList()
{
  _cobill->clear();

  q.exec("SELECT * FROM billingEditList;");
  if (q.first())
  {
    int           thisOrderid;
    int           orderid    = -9999;
    XTreeWidgetItem *orderLine = NULL;
    XTreeWidgetItem *lastLine  = NULL;
    XTreeWidgetItem *selected  = NULL;

//  Fill the list with the query contents
    do
    {
      thisOrderid = q.value("orderid").toInt();

//  Check to see if this a new order number
      if (thisOrderid != orderid)
      {
//  New order number, make a new list item header
        orderid  = thisOrderid;
        lastLine = NULL;

        XTreeWidgetItem *thisLine = new XTreeWidgetItem( _cobill, orderLine, q.value("orderid").toInt(), q.value("itemid").toInt(),
                                                     q.value("documentnumber"), q.value("ordernumber"),
                                                     q.value("cust_number"), q.value("billtoname") );
        orderLine = thisLine;

//  Add the distribution line
        thisLine = new XTreeWidgetItem( orderLine, q.value("orderid").toInt(), -1,
                                      "", "",
                                      q.value("sence"), q.value("account"),
                                      "", "", "", q.value("extprice") );
        if (q.value("account") == "Not Assigned")
        {
          thisLine->setTextColor("red");
          orderLine->setTextColor("red");
        }

//  If we are looking for a selected order and this is it, cache it
        if (thisOrderid == _orderid)
          selected = orderLine;
      }
      else
      {
        XTreeWidgetItem *itemLine = new XTreeWidgetItem( orderLine, lastLine, q.value("orderid").toInt(), q.value("itemid").toInt(),
                                                     "", "",
                                                     q.value("item"), q.value("itemdescrip"),
                                                     q.value("iteminvuom"), q.value("qtytobill"),
                                                     q.value("price"), q.value("extprice") );

//  Add the distribution line
        XTreeWidgetItem *thisLine = new XTreeWidgetItem( itemLine, q.value("orderid").toInt(), -1,
                                                     "", "",
                                                     q.value("sence"), q.value("account"),
                                                     "", "", "", q.value("extprice") );
        if (q.value("account") == "Not Assigned")
        {
          thisLine->setTextColor("red");
          itemLine->setTextColor("red");
          orderLine->setTextColor("red");
        }

        lastLine = itemLine;
      }
    }
    while (q.next());

//  Select and show the select item, if any
    if (selected != NULL)
    {
      _cobill->setCurrentItem(selected);
      _cobill->scrollToItem(selected);
    }
  }
}

void billingEditList::sPrint()
{
  orReport report("BillingEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
