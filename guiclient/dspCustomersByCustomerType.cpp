/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomersByCustomerType.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "customer.h"
#include "customerTypeList.h"

dspCustomersByCustomerType::dspCustomersByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cust, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_autoRefresh, SIGNAL(toggled(bool)), this, SLOT(sHandleRefreshButton(bool)));

  _customerType->setType(ParameterGroup::CustomerType);

  _cust->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "custtype_code");
  _cust->addColumn(tr("Active"),  _ynColumn,   Qt::AlignLeft,  true, "cust_active");
  _cust->addColumn(tr("Number"),  _itemColumn, Qt::AlignLeft, true, "cust_number");
  _cust->addColumn(tr("Name"),    200,         Qt::AlignLeft, true, "cust_name");
  _cust->addColumn(tr("Address"), -1,          Qt::AlignLeft, true, "cust_address1");
  _cust->setDragString("custid=");

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), SLOT(sFillList()));
  sHandleRefreshButton(_autoRefresh->isChecked());
}

dspCustomersByCustomerType::~dspCustomersByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCustomersByCustomerType::languageChange()
{
  retranslateUi(this);
}

void dspCustomersByCustomerType::sPrint()
{
  ParameterList params;

  _customerType->appendValue(params);

  if(_showInactive->isChecked())
    params.append("showInactive");

  orReport report("CustomersByCustomerType", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCustomersByCustomerType::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Reassign Customer Type", this, SLOT(sReassignCustomerType()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspCustomersByCustomerType::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCustomerType::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCustomerType::sReassignCustomerType()
{
  ParameterList params;
  params.append("custtype_id", _cust->altId());

  customerTypeList *newdlg = new customerTypeList(this, "", TRUE);
  newdlg->set(params);
  int custtypeid = newdlg->exec();
  if ( (custtypeid != -1) && (custtypeid != XDialog::Rejected) )
  {
    q.prepare( "UPDATE custinfo "
               "SET cust_custtype_id=:custtype_id "
               "WHERE (cust_id=:cust_id);" );
    q.bindValue(":cust_id", _cust->id());
    q.bindValue(":custtype_id", custtypeid);
    q.exec();
    omfgThis->sCustomersUpdated(_cust->id(), TRUE);
  }
}

void dspCustomersByCustomerType::sFillList()
{
  QString sql( "SELECT cust_id, cust_custtype_id,"
               "       custtype_code, cust_active, cust_number, cust_name, cust_address1 "
               "FROM cust, custtype "
               "WHERE ( (cust_custtype_id=custtype_id)" );
    
  if (_customerType->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";

  if (!_showInactive->isChecked())
    sql += " AND (cust_active)";

  sql += ");";

  q.prepare(sql);
  _customerType->bindValue(q);
  q.exec();
  _cust->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCustomersByCustomerType::sHandleRefreshButton(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
