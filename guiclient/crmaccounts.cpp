/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccounts.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "crmaccount.h"
#include "storedProcErrorLookup.h"

crmaccounts::crmaccounts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_print,	 SIGNAL(clicked()),	this,	SLOT(sPrint()));
    connect(_new,	 SIGNAL(clicked()),	this,	SLOT(sNew()));
    connect(_edit,	 SIGNAL(clicked()),	this,	SLOT(sEdit()));
    connect(_view,	 SIGNAL(clicked()),	this,	SLOT(sView()));
    connect(_delete,	 SIGNAL(clicked()),	this,	SLOT(sDelete()));
    connect(_activeOnly, SIGNAL(toggled(bool)),	this,	SLOT(sFillList()));
    connect(_crmaccount, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
				      this, SLOT(sPopulateMenu(QMenu*)));
    connect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(prospectsUpdated()), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(vendorsUpdated()), this, SLOT(sFillList()));

    _crmaccount->addColumn(tr("Number"),    80, Qt::AlignLeft,  true, "crmacct_number");
    _crmaccount->addColumn(tr("Name"),	    -1, Qt::AlignLeft,  true, "crmacct_name");
    _crmaccount->addColumn(tr("Customer"),  70, Qt::AlignCenter,true, "cust");
    _crmaccount->addColumn(tr("Prospect"),  70, Qt::AlignCenter,true, "prospect");
    _crmaccount->addColumn(tr("Vendor"),    70, Qt::AlignCenter,true, "vend");
    _crmaccount->addColumn(tr("Competitor"),70, Qt::AlignCenter,true, "competitor");
    _crmaccount->addColumn(tr("Partner"),   70, Qt::AlignCenter,true, "partner");
    _crmaccount->addColumn(tr("Tax Auth."), 70, Qt::AlignCenter,true, "taxauth");

    if (_privileges->check("MaintainCRMAccounts"))
    {
      connect(_crmaccount, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_crmaccount, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_crmaccount, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_crmaccount, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    _activeOnly->setChecked(true);

    sFillList();
}

crmaccounts::~crmaccounts()
{
    // no need to delete child widgets, Qt does it all for us
}

void crmaccounts::languageChange()
{
    retranslateUi(this);
}

void crmaccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id", _crmaccount->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccounts::sDelete()
{
  q.prepare("SELECT deleteCRMAccount(:crmacct_id) AS returnVal;");
  q.bindValue(":crmacct_id", _crmaccount->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("returnVal").toInt();
    if (returnVal < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCRMAccount", returnVal),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void crmaccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id", _crmaccount->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccounts::sFillList()
{
  QString sql("SELECT crmacct_id, crmacct_number, crmacct_name,"
	      "     CASE WHEN crmacct_cust_id IS NULL THEN '' ELSE 'Y' END AS cust,"
	      "     CASE WHEN crmacct_prospect_id IS NULL THEN '' ELSE 'Y' END AS prospect,"
	      "     CASE WHEN crmacct_vend_id IS NULL THEN '' ELSE 'Y' END AS vend,"
	      "     CASE WHEN crmacct_competitor_id IS NULL THEN '' ELSE 'Y' END AS competitor,"
	      "     CASE WHEN crmacct_partner_id IS NULL THEN '' ELSE 'Y' END AS partner,"
	      "     CASE WHEN crmacct_taxauth_id IS NULL THEN '' ELSE 'Y' END AS taxauth "
              "FROM crmacct "
	      "<? if exists(\"activeOnly\") ?> WHERE crmacct_active <? endif ?>"
              "ORDER BY crmacct_number;");
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _crmaccount->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void crmaccounts::sPopulateMenu( QMenu * pMenu )
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCRMAccounts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainCRMAccounts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

bool crmaccounts::setParams(ParameterList &params)
{
  if (_activeOnly->isChecked())
    params.append("activeOnly");

  return true;
}

void crmaccounts::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CRMAccountMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
