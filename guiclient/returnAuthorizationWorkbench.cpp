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

#include "returnAuthorizationWorkbench.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "mqlutil.h"

returnAuthorizationWorkbench::returnAuthorizationWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

//  connect(_raopen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
//  connect(_radue, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_editopn, SIGNAL(clicked()), this, SLOT(sEditOpen()));
  connect(_viewopn, SIGNAL(clicked()), this, SLOT(sViewOpen()));
  connect(_printopn, SIGNAL(clicked()), this, SLOT(sPrintOpen()));
  connect(_editdue, SIGNAL(clicked()), this, SLOT(sEditDue()));
  connect(_viewdue, SIGNAL(clicked()), this, SLOT(sViewDue()));
  connect(_printdue, SIGNAL(clicked()), this, SLOT(sPrintDue()));

  _raopen->addColumn(tr("Auth. #"),       _orderColumn,   Qt::AlignLeft   );
  _raopen->addColumn(tr("Customer"),     -1,             Qt::AlignLeft   );
  _raopen->addColumn(tr("Auth. Date"),   _dateColumn,     Qt::AlignRight  );
  _raopen->addColumn(tr("Disposition"),  _itemColumn,     Qt::AlignRight  );
  _raopen->addColumn(tr("Amount"),       _moneyColumn,     Qt::AlignRight  );
  _raopen->addColumn(tr("Payment"),      _itemColumn,     Qt::AlignRight  );
  _raopen->addColumn(tr("Status"),       _itemColumn,    Qt::AlignCenter );

  if (!_privleges->check("MaintainReturns"))
  {
    _editopn->hide();
	_editdue->hide();
  }
}

returnAuthorizationWorkbench::~returnAuthorizationWorkbench()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnAuthorizationWorkbench::languageChange()
{
  retranslateUi(this);
}

/*
void returnAuthorizationWorkbench::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (QString(pSelected->text(0)) == "W/O")
  {
    menuItem = pMenu->insertItem(tr("View Work Order..."), this, SLOT(sViewWorkOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("ViewWorkOrders"));
  }
  else if (QString(pSelected->text(0)) == "S/O")
  {
    menuItem = pMenu->insertItem(tr("View Sales Order..."), this, SLOT(sViewCustomerOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("ViewSalesOrders"));

    pMenu->insertItem(tr("Edit Sales Order..."), this, SLOT(sEditCustomerOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("MaintainSalesOrders"));
  }
  else if (QString(pSelected->text(0)) == "T/O")
  {
    menuItem = pMenu->insertItem(tr("View Transfer Order..."), this, SLOT(sViewTransferOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("ViewTransferOrders"));

    pMenu->insertItem(tr("Edit Transfer Order..."), this, SLOT(sEditTransferOrder()), 0);
    pMenu->setItemEnabled(menuItem, _privleges->check("MaintainTransferOrders"));
  }
}
*/

void returnAuthorizationWorkbench::sPrintOpen()
{
}

void returnAuthorizationWorkbench::sEditOpen()
{
}

void returnAuthorizationWorkbench::sViewOpen()
{
}

void returnAuthorizationWorkbench::sPrintDue()
{
}

void returnAuthorizationWorkbench::sEditDue()
{
}

void returnAuthorizationWorkbench::sViewDue()
{
}

void returnAuthorizationWorkbench::sFillList()
{ 
	q.prepare("SELECT rahead_id, rahead_number, COALESCE(cust_name,:undefined), "
		      "rahead_authdate, "
		  	  "CASE "
			  "  WHEN rahead_disposition = 'C' THEN "
			  "    :credit "
			  "  WHEN rahead_disposition = 'R' THEN "
			  "    :return "
			  "  WHEN rahead_disposition = 'P' THEN "
			  "    :replace "
			  "  WHEN rahead_disposition = 'V' THEN "
			  "    :service "
			  "  WHEN rahead_disposition = 'M' THEN "
			  "    :mixed "
			  "  END AS disposition, "
              "  formatMoney(SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * "
			  "     (raitem_unitprice / raitem_price_invuomratio),2)-raitem_amtcredited)) AS subtotal, "
			  "CASE "
			  "  WHEN rahead_creditmethod = 'N' THEN "
			  "    :none "
			  "  WHEN rahead_creditmethod = 'M' THEN "
			  "    :creditmemo "
			  "  WHEN rahead_creditmethod = 'K' THEN "
			  "    :check "
			  "  WHEN rahead_creditmethod = 'C' THEN "
			  "    :creditcard "
			  "END AS creditmethod "
              "FROM rahead "
			  "  LEFT OUTER JOIN custinfo ON (rahead_cust_id=cust_id), "
			  " raitem, itemsite, item "
              "WHERE ( (rahead_id=raitem_rahead_id) "
              " AND (raitem_itemsite_id=itemsite_id)"
              " AND (itemsite_item_id=item_id) "
			  " AND (rahead_status = 'O') ) "
			  " GROUP BY rahead_id,rahead_number,cust_name, "
			  " rahead_authdate,rahead_disposition,rahead_creditmethod "
			  " ORDER BY rahead_authdate,rahead_number ;"); 
	q.bindValue(":undefined",tr("Undefined"));
	q.bindValue(":credit",tr("Credit"));
	q.bindValue(":return",tr("Return"));
	q.bindValue(":replace",tr("Replace"));
	q.bindValue(":service",tr("Service"));
	q.bindValue(":mixed",tr("Mixed"));
	q.bindValue(":none",tr("None"));
	q.bindValue(":creditmemo",tr("Credit Memo"));
	q.bindValue(":check",tr("Check"));
	q.bindValue(":creditcard",tr("Credit Card"));
	q.exec();
	if (q.first())
		_raopen->populate(q);
}

