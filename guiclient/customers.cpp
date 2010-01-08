/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customers.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "customer.h"
#include "customerTypeList.h"
#include "storedProcErrorLookup.h"

customers::customers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_cust, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainCustomerMasters"))
  {
    connect(_cust, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_cust, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_cust, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_cust, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _cust->addColumn(tr("Type"),    _itemColumn,  Qt::AlignCenter, true, "custtype");
  _cust->addColumn(tr("Number"),  _orderColumn, Qt::AlignCenter, true, "cust_number");
  _cust->addColumn(tr("Active"),  _ynColumn,    Qt::AlignCenter, true, "cust_active");
  _cust->addColumn(tr("Name"),    -1,           Qt::AlignLeft,   true, "cust_name");
  _cust->addColumn(tr("Address"), 175,          Qt::AlignLeft,   true, "cust_address1");
  _cust->addColumn(tr("Phone #"), 100,          Qt::AlignLeft,   true, "cust_phone");

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), SLOT(sFillList(int, bool)));

  sFillList(-1, FALSE);
}

/*
 *  Destroys the object and frees any allocated resources
 */
customers::~customers()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void customers::languageChange()
{
  retranslateUi(this);
}

void customers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sEdit()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "edit");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sView()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "view");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sReassignCustomerType()
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

void customers::sDelete()
{
  q.prepare("SELECT deleteCustomer(:cust_id) AS result;");
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
      QMessageBox::critical(this, tr("Cannot Delete Customer"),
			    storedProcErrorLookup("deleteCustomer", returnVal));
      return;
    }
    omfgThis->sCustomersUpdated(-1, TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customers::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem("View...", this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Reassign Customer Type", this, SLOT(sReassignCustomerType()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Delete", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void customers::sFillList(int pCustid, bool pLocal)
{
  q.prepare( "SELECT cust_id, cust_custtype_id,"
             "       COALESCE(custtype_code, :error) AS custtype, cust_number,"
             "       cust_active, cust_name, cust_address1, cust_phone "
             "FROM cust LEFT OUTER JOIN custtype ON (cust_custtype_id=custtype_id) "
             "ORDER BY cust_number;" );
  q.bindValue(":error", tr("Error"));
  q.exec();
 
  if (pLocal)
    _cust->populate(q, pCustid, TRUE);
  else
    _cust->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
