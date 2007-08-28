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

#include "voucheringEditList.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include "voucher.h"
#include "miscVoucher.h"
/*
 *  Constructs a voucheringEditList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
voucheringEditList::voucheringEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vo, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  statusBar()->hide();
  
  _vo->setRootIsDecorated(TRUE);
  _vo->addColumn(tr("Vchr. #"),          (_orderColumn + _vo->indentation()), Qt::AlignRight  );
  _vo->addColumn(tr("P/O #"),            _orderColumn,                         Qt::AlignRight  );
  _vo->addColumn(tr("Invc./Item #"),     _itemColumn,                          Qt::AlignRight  );
  _vo->addColumn(tr("Vendor #"),         _itemColumn,                          Qt::AlignRight  );
  _vo->addColumn(tr("Name/Description"), -1,                                   Qt::AlignLeft   );
  _vo->addColumn(tr("Vend. Type"),       _itemColumn,                          Qt::AlignLeft   );
  _vo->addColumn(tr("UOM"),              _uomColumn,                           Qt::AlignCenter );
  _vo->addColumn(tr("Qty. Vchrd."),      _qtyColumn,                           Qt::AlignRight  );
  _vo->addColumn(tr("Cost"),             _moneyColumn,                         Qt::AlignRight  );

  connect(omfgThis, SIGNAL(vouchersUpdated()), this, SLOT(sFillList()));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
voucheringEditList::~voucheringEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void voucheringEditList::languageChange()
{
  retranslateUi(this);
}

void voucheringEditList::sPrint()
{
  orReport report("VoucheringEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void voucheringEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  if (_vo->altId() == -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privleges->check("MaintainVouchers"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  }
}

void voucheringEditList::sEdit()
{
  q.prepare( "SELECT vohead_pohead_id "
             "FROM vohead "
             "WHERE (vohead_id=:vohead_id);" );
  q.bindValue(":vohead_id", _vo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("vohead_id", _vo->id());

    if (q.value("vohead_pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void voucheringEditList::sView()
{
  q.prepare( "SELECT vohead_pohead_id "
             "FROM vohead "
             "WHERE (vohead_id=:vohead_id);" );
  q.bindValue(":vohead_id", _vo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vohead_id", _vo->id());

    if (q.value("vohead_pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void voucheringEditList::sFillList()
{
  _vo->clear();

  q.exec("SELECT * FROM voucheringEditList;");
  if (q.first())
  {
    int           thisOrderid;
    int           orderid    = -9999;
    int           vendid     = -1;
    double        amount     = 0;
    double        thisAmount = 0;
    XTreeWidgetItem *orderLine = NULL;
    XTreeWidgetItem *lastLine  = NULL;

//  Fill the list with the query contents
    do
    {
      thisOrderid = q.value("orderid").toInt();

//  Check to see if this a new order number
      if (thisOrderid != orderid)
      {
//  If there was a previous line, add the Debit distribution
        if (orderLine)
        {
          QString   account;
          bool      notAssigned = TRUE;
          XSqlQuery debit;
          debit.prepare( "SELECT formatGLAccountLong(accnt_id) AS account "
                         "FROM accnt "
                         "WHERE (accnt_id=findAPAccount(:vend_id));" );
          debit.bindValue(":vend_id", vendid);
          debit.exec();
          if (debit.first())
          {
            notAssigned = FALSE;
            account = debit.value("account").toString();
          }
          else
            account = tr("Not Assigned");

          XTreeWidgetItem *debitLine = new XTreeWidgetItem(orderLine, lastLine, q.value("orderid").toInt(), -1);
          debitLine->setText(2, tr("Debit"));
          debitLine->setText(4, account);
          debitLine->setText(8, formatMoney(thisAmount));

          if (notAssigned)
          {
            debitLine->setTextColor("red");
            orderLine->setTextColor("red");
          }
        }

//  New order number, make a new list item header
        orderid  = thisOrderid;
        vendid   = q.value("vendid").toInt();
        lastLine = NULL;

        orderLine = new XTreeWidgetItem( _vo, orderLine, q.value("orderid").toInt(), -1,
                                       q.value("vouchernumber"), q.value("ponumber"),
                                       q.value("invoicenumber"), q.value("itemnumber"),
                                       q.value("description"), q.value("itemtype") );
        orderLine->setText(8, q.value("f_cost").toString());

        thisAmount = 0;
        amount += q.value("cost").toDouble();
      }
      else
      {
        XTreeWidgetItem *itemLine = new XTreeWidgetItem( orderLine, lastLine, q.value("orderid").toInt(), q.value("itemid").toInt(),
                                                     "", "",
                                                     q.value("itemnumber"), q.value("invoicenumber"), q.value("description"), q.value("itemtype"),
                                                     q.value("item_invuom"), q.value("f_qty"),
                                                     q.value("f_cost") );

//  Add the distribution lines
        XTreeWidgetItem *thisLine = new XTreeWidgetItem(itemLine, q.value("orderid").toInt(), q.value("itemid").toInt());
        thisLine->setText(2, tr("Credit"));
        thisLine->setText(4, q.value("account").toString());
        thisLine->setText(8, q.value("f_cost").toString());
        thisAmount += q.value("cost").toDouble();

        if (q.value("account").toString() == "Not Assigned")
        {
          thisLine->setTextColor("red");
          itemLine->setTextColor("red");
          orderLine->setTextColor("red");
        }

        lastLine = itemLine;
      }
    }
    while (q.next());

//  If there was a previous line, add the Debit distribution
    if (orderLine)
    {
      q.prev();
      QString   account;
      bool      notAssigned = TRUE;
      XSqlQuery debit;
      debit.prepare( "SELECT formatGLAccountLong(accnt_id) AS account "
                     "FROM accnt "
                     "WHERE (accnt_id=findAPAccount(:vend_id));" );
      debit.bindValue(":vend_id", vendid);
      debit.exec();
      if (debit.first())
      {
        notAssigned = FALSE;
        account = debit.value("account").toString();
      }
      else
        account = tr("Not Assigned");

      XTreeWidgetItem *debitLine = new XTreeWidgetItem(orderLine, lastLine, q.value("orderid").toInt(), -1);
      debitLine->setText(2, tr("Debit"));
      debitLine->setText(4, account);
      debitLine->setText(8, formatMoney(thisAmount));

      if (notAssigned)
      {
        debitLine->setTextColor("red");
        orderLine->setTextColor("red");
      }
    }

    XTreeWidgetItem *totals = new XTreeWidgetItem(_vo, orderLine, -1, -1, tr("Total"));
    totals->setText(8, formatMoney(amount));
  }
}

