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
#include <QMessageBox>

#include <openreports.h>
#include <metasql.h>
#include "mqlutil.h"
#include "returnAuthorization.h"

returnAuthorizationWorkbench::returnAuthorizationWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  _codeGroup = new QButtonGroup(this);
  _codeGroup->addButton(_cust);
  _codeGroup->addButton(_custtype);
  _custInfo->hide();
  _parameter->setType(CustomerType);

//  connect(_ra, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
//  connect(_radue, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_editdue, SIGNAL(clicked()), this, SLOT(sEditDue()));
  connect(_viewdue, SIGNAL(clicked()), this, SLOT(sViewDue()));
  connect(_printdue, SIGNAL(clicked()), this, SLOT(sPrintDue()));
  connect(_process, SIGNAL(clicked()), this, SLOT(sProcess()));
  connect(_radue, SIGNAL(valid(bool)), this, SLOT(sHandleButton()));
  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated()), this, SLOT(sFillList()));

  _ra->addColumn(tr("Auth. #"),       _orderColumn,   Qt::AlignLeft   );
  _ra->addColumn(tr("Customer"),     -1,              Qt::AlignLeft   );
  _ra->addColumn(tr("Authorized"),   _dateColumn,     Qt::AlignRight  );
  _ra->addColumn(tr("Expires"),      _dateColumn,     Qt::AlignRight  );
  _ra->addColumn(tr("Disposition"),  _itemColumn,     Qt::AlignRight  );
  _ra->addColumn(tr("Credit By"),    _itemColumn,     Qt::AlignRight  );
  _ra->addColumn(tr("Awaiting"),     _itemColumn,     Qt::AlignCenter );

  _radue->addColumn(tr("Auth. #"),       _orderColumn,   Qt::AlignLeft   );
  _radue->addColumn(tr("Customer"),     -1,              Qt::AlignLeft   );
  _radue->addColumn(tr("Authorized"),   _dateColumn,     Qt::AlignRight  );
  _radue->addColumn(tr("Eligible"),     _dateColumn,     Qt::AlignRight  );
  _radue->addColumn(tr("Amount"),       _moneyColumn,    Qt::AlignRight  );
  _radue->addColumn(tr("Credit By"),    _itemColumn,     Qt::AlignRight  );

  if (!_privleges->check("MaintainReturns"))
  {
    _edit->hide();
	_editdue->hide();
  }

  //Remove Credit Card related option if Credit Card not enabled
  QString key = omfgThis->_key;
  if(!_metrics->boolean("CCAccept") || key.length() == 0 || key.isNull() || key.isEmpty())
    _creditcard->hide();

  if (!_privleges->check("PostARDocuments"))
  {
    _postmemos->setChecked(false);
	_postmemos->setEnabled(false);
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

void returnAuthorizationWorkbench::sPrint()
{
}

void returnAuthorizationWorkbench::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sPrintDue()
{
}

void returnAuthorizationWorkbench::sEditDue()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", _radue->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sViewDue()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", _radue->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sHandleButton()
{
  _process->setEnabled(((_radue->altId() == 1 && _privleges->check("MaintainCreditMemos")) ||
	   (_radue->altId() == 2 && _privleges->check("MaintainPayments")) ||
	   (_radue->altId() == 3 && _privleges->check("PostARDocuments"))));  
}

void returnAuthorizationWorkbench::sProcess()
{
  if (_radue->altId() == 1)
  {
	q.prepare("SELECT createRaCreditMemo(:rahead_id,:post) AS result;");
	q.bindValue(":rahead_id",_radue->id());
	q.bindValue(":post",QVariant(_postmemos->isChecked()));
	q.exec();
	if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  _radue->clear();
      return;
    }
  }
  sFillList();
}

void returnAuthorizationWorkbench::sFillList()
{ 
  _ra->clear();
  _radue->clear();
  if (_cust->isChecked() && !_custInfo->isValid())
  {
    QMessageBox::information( this, tr("Customer not selected"),
			      tr("<p>Please select a customer.") );
    _custInfo->setFocus();
    return;
  }

  //Fill Review List
  if (_closed->isChecked() && !_dates->allValid())
  {
    QMessageBox::information( this, tr("Invalid Dates"),
			      tr("<p>Invalid dates specified. Please specify a "
				 "valid date range.") );
    _dates->setFocus();
    return;
  }
  else if ((_receipts->isChecked()) || (_shipments->isChecked()) || 
	        (_payment->isChecked()) || (_closed->isChecked()))
  {
	bool bw;
	bw = false;
	QString sql (" SELECT * FROM ( "
			  "SELECT rahead_id, rahead_number, COALESCE(cust_name,:undefined), "
			  "formatDate(rahead_authdate), COALESCE(formatDate(rahead_expiredate),:never), "
	  		  "CASE "
			  "  WHEN raitem_disposition = 'C' THEN "
			  "    :credit "
			  "  WHEN raitem_disposition = 'R' THEN "
			  "    :return "
			  "  WHEN raitem_disposition = 'P' THEN "
			  "    :replace "
			  "  WHEN raitem_disposition = 'V' THEN "
			  "    :service "
			  "  WHEN raitem_disposition = 'S' THEN "
			  "    :ship "
			  "  END AS disposition, "
			  "CASE "
			  "  WHEN rahead_creditmethod = 'N' THEN "
			  "    :none "
			  "  WHEN rahead_creditmethod = 'M' THEN "
			  "    :creditmemo "
			  "  WHEN rahead_creditmethod = 'K' THEN "
			  "    :check "
			  "  WHEN rahead_creditmethod = 'C' THEN "
			  "    :creditcard "
			  "END AS creditmethod, "
			  "CASE "
			  "  WHEN raitem_status = 'C' THEN "
			  "    :closed "
			  "  WHEN raitem_disposition = 'C' THEN "
			  "    :payment "
			  "  WHEN raitem_disposition = 'R' "
			  "    AND SUM(raitem_qtyreceived-raitem_qtycredited) > 0 "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt || ',' || :payment "
			  "  WHEN raitem_disposition = 'R' "
			  "    AND SUM(raitem_qtyreceived-raitem_qtycredited) > 0 THEN "
			  "    :payment "
			  "  WHEN raitem_disposition = 'R' "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt "
			  "  WHEN raitem_disposition IN ('P','V') "
			  "    AND SUM(raitem_qtyauthorized-COALESCE(coitem_qtyshipped,0)) > 0 "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt || ',' || :ship "
			  "  WHEN raitem_disposition IN ('P','V') "
			  "    AND SUM(raitem_qtyauthorized-COALESCE(coitem_qtyshipped,0)) > 0 THEN "
			  "    :ship "
			  "  WHEN raitem_disposition IN ('P','V') "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt "
			  "  WHEN raitem_disposition = 'S' THEN "
			  "    :ship "
			  "  ELSE '' "
			  "END AS awaiting "
			  "FROM rahead "
			  "  LEFT OUTER JOIN custinfo ON (rahead_cust_id=cust_id) "
			  "  LEFT OUTER JOIN custtype ON (cust_custtype_id=custtype_id), "
			  " raitem "
			  "  LEFT OUTER JOIN coitem ON (raitem_new_coitem_id=coitem_id) "
			  "WHERE ( (rahead_id=raitem_rahead_id)");

    if ((_cust->isChecked()))
	  sql += " AND (cust_id=:cust_id) ";
    else if (_parameter->isSelected())
	  sql += " AND (custtype_id=:custtype_id) ";
    else if (_parameter->isPattern())
	  sql += " AND (custtype_code ~ :custtype_pattern) ";

	if (!_expired->isChecked())
	  sql +=  " AND (COALESCE(rahead_expiredate,current_date) >= current_date)";
	if (_closed->isChecked())
	  sql +=  " AND (raitem_status='O' OR rahead_authdate BETWEEN :startDate AND :endDate)";
	else
      sql +=  " AND (raitem_status = 'O') ";

  	sql +=    " ) GROUP BY rahead_id,rahead_number,cust_name,rahead_expiredate, "
			  " rahead_authdate,raitem_status,raitem_disposition,rahead_creditmethod, "
			  " rahead_curr_id "
			  " ORDER BY rahead_authdate,rahead_number "
			  ") as data ";

	if (_receipts->isChecked())
	{
	  sql +=  " WHERE ((disposition IN (:return,:replace,:service)) "
			  " AND (awaiting ~ :receipt)) "; 
	  bw = true;
	}
	if (_shipments->isChecked())
	{
	  if (bw)
		sql += "OR (";
	  else
		sql += "WHERE (";
	  sql +=  " (disposition IN (:replace,:service,:ship))"
			  " AND (awaiting ~ :ship)) ";  
	  bw = true;
	}
	if (_payment->isChecked())
	{
	  if (bw)
		sql += "OR (";
	  else
		sql += "WHERE (";
	  sql +=  " (disposition IN (:credit,:return))"
			  " AND (awaiting ~ :payment)) "; 
	  bw = true;
	}
	if (_closed->isChecked())
	{
	  if (bw)
		sql += "OR (";
	  else
		sql += "WHERE (";
	  sql +=  "(awaiting = :closed)) "; 
	}
    
	XSqlQuery ra;
	ra.prepare(sql);
    _parameter->bindValue(ra);
	ra.bindValue(":cust_id", _custInfo->id());
	ra.bindValue(":undefined",tr("Undefined"));
	ra.bindValue(":credit",tr("Credit"));
	ra.bindValue(":return",tr("Return"));
	ra.bindValue(":replace",tr("Replace"));
	ra.bindValue(":service",tr("Service"));
	ra.bindValue(":none",tr("None"));
	ra.bindValue(":creditmemo",tr("Memo"));
	ra.bindValue(":check",tr("Check"));
	ra.bindValue(":creditcard",tr("Card"));
	ra.bindValue(":payment",tr("Payment"));
	ra.bindValue(":receipt",tr("Receipt"));
	ra.bindValue(":ship",tr("Shipment"));
	ra.bindValue(":never",tr("Never"));
	ra.bindValue(":closed",tr("Closed"));
    _dates->bindValue(ra);
	ra.exec();
	if (ra.first())
		_ra->populate(ra);
	else if (ra.lastError().type() != QSqlError::None)
    {
      systemError(this, ra.lastError().databaseText(), __FILE__, __LINE__);
	  _ra->clear();
      return;
    }
  }

  //Fill Due Credit List
  if ((_creditmemo->isChecked()) || (_check->isChecked()) || (_creditcard->isChecked()))
  {
	bool bc;
	bc = false;
	QString sql (" SELECT * FROM ( "
			  "SELECT DISTINCT rahead_id, "
		      "CASE "
  			  "  WHEN rahead_creditmethod = 'M' THEN "
			  "    1 "
			  "  WHEN rahead_creditmethod = 'K' THEN "
			  "    2 "
			  "  WHEN rahead_creditmethod = 'C' THEN "
			  "    3 "
			  "END, "
			  "rahead_number, cust_name, "
			  "formatDate(rahead_authdate), formatDate(NULL), "
			  "formatMoney(currtobase(rahead_curr_id,calcradueamt(rahead_id),current_date)), "
			  "CASE "
			  "  WHEN rahead_creditmethod = 'M' THEN "
			  "    :creditmemo "
			  "  WHEN rahead_creditmethod = 'K' THEN "
			  "    :check "
			  "  WHEN rahead_creditmethod = 'C' THEN "
			  "    :creditcard "
			  "END AS creditmethod, rahead_authdate "
			  "FROM rahead,custinfo,raitem,custtype "
			  "WHERE ( (rahead_id=raitem_rahead_id) "
			  " AND (rahead_cust_id=cust_id) "
			  " AND (cust_custtype_id=custtype_id) "
			  " AND ((raitem_disposition = 'R' AND raitem_qtyreceived > raitem_qtycredited) "
			  " OR (raitem_disposition = 'C' AND raitem_qtyauthorized > raitem_qtycredited)) "
			  " AND (raitem_status = 'O') "
			  " AND (rahead_creditmethod != 'N') "
			  " AND (calcradueamt(rahead_id) > 0) "
			  " AND (raitem_disposition IN ('C','R')) ");

    if ((_cust->isChecked()))
	  sql += " AND (cust_id=:cust_id) ";
    else if (_parameter->isSelected())
	  sql += " AND (custtype_id=:custtype_id) ";
    else if (_parameter->isPattern())
	  sql += " AND (custtype_code ~ :custtype_pattern) ";

  	sql +=    " ) ORDER BY rahead_authdate,rahead_number "
			  ") as data "
              " WHERE (creditmethod IN ( "; 

	if (_creditmemo->isChecked())
	{
	  sql += ":creditmemo";
	  bc = true;
	}
	if (_check->isChecked())
	{
	  if (bc)
		sql += ",";
	  sql +=  ":check "; 
	  bc = true;
	}
	if (_creditcard->isChecked())
	{
	  if (bc)
		sql += ",";
	  sql +=  ":creditcard"; 
	}
    
	sql += "));";

	XSqlQuery radue;
	radue.prepare(sql);
    _parameter->bindValue(radue);
	radue.bindValue(":cust_id", _custInfo->id());
	radue.bindValue(":credit",tr("Credit"));
	radue.bindValue(":return",tr("Return"));
	radue.bindValue(":none",tr("None"));
	radue.bindValue(":creditmemo",tr("Memo"));
	radue.bindValue(":check",tr("Check"));
	radue.bindValue(":creditcard",tr("Card"));
    _dates->bindValue(radue);
	radue.exec();
	if (radue.first())
		_radue->populate(radue,TRUE);
	else if (radue.lastError().type() != QSqlError::None)
    {
      systemError(this, radue.lastError().databaseText(), __FILE__, __LINE__);
	  _radue->clear();
      return;
    }
  }
  else
    _radue->clear();
}

void returnAuthorizationWorkbench::sParameterTypeChanged()
{
  if(_cust->isChecked())
  {
    _parameter->hide();
	_custInfo->show();
  }
  else //if(_custtype->isChecked())
  {
    _parameter->show();
	_custInfo->hide();
  }
}

