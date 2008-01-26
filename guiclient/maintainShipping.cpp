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

#include "maintainShipping.h"

#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "distributeInventory.h"
#include "mqlutil.h"
#include "shippingInformation.h"
#include "shipOrder.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "transferOrder.h"
#include "printShippingForm.h"
#include "issueToShipping.h"
#include "storedProcErrorLookup.h"

maintainShipping::maintainShipping(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_ship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _ship->setRootIsDecorated(TRUE);
  _ship->addColumn(tr("Shipment #"),      _orderColumn, Qt::AlignLeft   );
  _ship->addColumn(tr("Order/Line #"),     _itemColumn, Qt::AlignRight  );
  _ship->addColumn(tr("Prnt'ed"),          _dateColumn, Qt::AlignCenter );
  _ship->addColumn(tr("Cust./Item #"),     _itemColumn, Qt::AlignLeft   );
  _ship->addColumn(tr("Name/Description"), -1,          Qt::AlignLeft   );
  _ship->addColumn(tr("Ship Via"),         _itemColumn, Qt::AlignLeft   );
  _ship->addColumn(tr("UOM"),              _uomColumn,  Qt::AlignLeft   );
  _ship->addColumn(tr("Qty. At Ship"),     _qtyColumn,  Qt::AlignRight  );
  _ship->addColumn(tr("Hold Type"),                 0,  Qt::AlignCenter );

  sFillList();
}

maintainShipping::~maintainShipping()
{
  // no need to delete child widgets, Qt does it all for us
}

void maintainShipping::languageChange()
{
  retranslateUi(this);
}

void maintainShipping::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  int menuItem;

  if (selected->text(0) != "")
    _itemtype = 1;
  else if (selected->text(1) != "")
    _itemtype = 2;
  else
    _itemtype = 3;

  switch (_itemtype)
  {
    case 1:
      menuItem = pMenu->insertItem("Shipping Information...", this, SLOT(sShippingInformation()), 0);
      if (!_privleges->check("EnterShippingInformation"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("Return ALL Stock Issued to Order...", this, SLOT(sReturnAllOrderStock()), 0);
      if (!_privleges->check("ReturnStockFromShipping"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("View Order...", this, SLOT(sViewOrder()), 0);
      if (!_privleges->check("ViewSalesOrders"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("Print Shipping Form...", this, SLOT(sPrintShippingForm()), 0);
      if (!_privleges->check("PrintBillsOfLading"))
        pMenu->setItemEnabled(menuItem, FALSE);

      if (selected->text(6) != "S")
      {
        menuItem = pMenu->insertItem("Ship Order...", this, SLOT(sShipOrder()), 0);
        if (!_privleges->check("ShipOrders"))
          pMenu->setItemEnabled(menuItem, FALSE);
      }

      break;

    case 2:
      menuItem = pMenu->insertItem("Issue Additional Stock to Order Line...", this, SLOT(sIssueStock()), 0);
      if (!_privleges->check("IssueStockToShipping"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("Return ALL Stock Issued to Order Line...", this, SLOT(sReturnAllLineStock()), 0);
      if (!_privleges->check("ReturnStockFromShipping"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("View Order Line...", this, SLOT(sViewLine()), 0);
      if (!_privleges->check("ViewSalesOrders"))
        pMenu->setItemEnabled(menuItem, FALSE);

      break;

    case 3:
      menuItem = pMenu->insertItem("Return ALL of this Stock Issued in this Transaction...", this, SLOT(sReturnAllStock()), 0);
      if (!_privleges->check("ReturnStockFromShipping"))
        pMenu->setItemEnabled(menuItem, FALSE);

      break;
  }
}

void maintainShipping::sShippingInformation()
{
  ParameterList params;
  params.append("shiphead_id", _ship->id());
  
  shippingInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void maintainShipping::sShipOrder()
{
  ParameterList params;
  params.append("shiphead_id", _ship->id());
  
  shipOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void maintainShipping::sReturnAllOrderStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN");
  q.prepare("SELECT returnCompleteShipment(:ship_id) AS result;");
  q.bindValue(":ship_id", _ship->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("returnCompleteShipment", result),
		  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == QDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
      return;
    }
    q.exec("COMMIT;"); 
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sViewOrder()
{
  q.prepare( "SELECT shiphead_order_id, shiphead_order_type "
             "FROM shiphead "
             "WHERE (shiphead_id=:shiphead_id);" );
  q.bindValue(":shiphead_id", _ship->id());
  q.exec();
  if (q.first())
  {
    if (q.value("shiphead_order_type").toString() == "SO")
      salesOrder::viewSalesOrder(q.value("shiphead_order_id").toInt());
    else if (q.value("shiphead_order_type").toString() == "TO")
      transferOrder::viewTransferOrder(q.value("shiphead_order_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sPrintShippingForm()
{
  ParameterList params;
  params.append("shiphead_id", _ship->id());

  printShippingForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void maintainShipping::sIssueStock()
{
  ParameterList params;
  params.append("sohead_id", _ship->altId());

  issueToShipping *newdlg = new issueToShipping();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void maintainShipping::sReturnAllLineStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN");
  q.prepare("SELECT returnItemShipments(:ship_id) AS result;");
  q.bindValue(":ship_id", _ship->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("returnItemShipments", result),
		  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == QDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
      return;
    }    
    q.exec("COMMIT;"); 
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sViewLine()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _ship->altId());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void maintainShipping::sReturnAllStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN");
  q.prepare("SELECT returnShipmentTransaction(:ship_id) AS result;");
  q.bindValue(":ship_id", _ship->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError(this,
		  storedProcErrorLookup("returnShipmentTransaction", result),
		  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(result, this) == QDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Issue to Shipping"), tr("Return Canceled") );
      return;
    }    
    q.exec("COMMIT;"); 
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void maintainShipping::sFillList()
{
  int currentId = _ship->altId();

  _ship->clear();

//  Grab the contents of shipping for the selected warehous
  ParameterList params;

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  params.append("notPrinted",	tr("No"));
  params.append("dirty",	tr("Dirty"));
  params.append("printed",	tr("Yes"));

  MetaSQLQuery mql = mqlLoad(":/sr/maintainShipping/FillListDetail.mql");
  q = mql.toQuery(params);
  q.exec();
  if (q.first())
  {
    double        atShipping   = 0.0;
    int           shipheadid   = -1;
    int           lineitemid   = -1;
    XTreeWidgetItem *order     = NULL;
    XTreeWidgetItem *line      = NULL;
    XTreeWidgetItem *ship      = NULL;
    XTreeWidgetItem *selected  = NULL;

    do
    {
      if (q.value("shiphead_id").toInt() != shipheadid)
      {
	//  if new order number, make a new list item header and
	//  update the running qty at ship value
        shipheadid = q.value("shiphead_id").toInt();
        if (line != NULL)
        {
          line->setText(7, formatQty(atShipping));
          atShipping = 0;
          line = NULL;
        }

        order = new XTreeWidgetItem( _ship, order, shipheadid, shipheadid,
                                   q.value("shiphead_number"),
				   q.value("order_number"), q.value("sfstatus"),
                                   q.value("dest"), q.value("destcntct"),
                                   q.value("shiphead_shipvia"), "",
                                   "", q.value("holdtype") );

	//  If we are looking for a selected order and this is it, cache it
        if ((_itemtype == 1) && (currentId == shipheadid))
          selected = order;
      }

      //  if this is a new lineitem
      if ((line == NULL) || (q.value("lineitem_id").toInt() != lineitemid))
      {
        lineitemid = q.value("lineitem_id").toInt();

        if (line != NULL)
        {
          line->setText(7, formatQty(atShipping));
          atShipping = 0;
        }

        line = new XTreeWidgetItem( order, line, shipheadid, lineitemid,
                                  "", q.value("linenumber"), "",
                                  q.value("item_number"), q.value("description"),
                                  "", q.value("uom_name"),
                                  "", q.value("holdtype") );

	//  If we are looking for a selected order and this is it, cache it
        if ((_itemtype == 2) && (currentId == lineitemid))
          selected = line;
      }

	//  Add the shipping detail for the current lineitem
        atShipping += q.value("shipqty").toDouble();

        ship = new XTreeWidgetItem( line, ship, shipheadid,
				  q.value("shipitem_id").toInt(),
                                  "", "", "",
                                  "",
				  q.value("shipitem_transdate").toString() + " by " +
				  q.value("shipitem_trans_username").toString(),
                                  "", q.value("uom_name"),
                                  formatQty(q.value("shipqty").toDouble()),
				  q.value("holdtype") );

	//  If we are looking for a selected shipping detail and this is it, cache it
        if ((_itemtype == 3) && (currentId == q.value("shipitem_id").toInt()))
          selected = ship;
    }
    while (q.next());

    line->setText(7, formatQty(atShipping));

    //  Select and show the select item, if any
    if (selected != NULL)
    {
      _ship->setItemSelected(selected, TRUE);
      _ship->scrollToItem(selected);
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
