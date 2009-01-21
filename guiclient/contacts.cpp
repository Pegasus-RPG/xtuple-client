/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contacts.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include <openreports.h>
#include <metasql.h>

#include "contact.h"
#include "storedProcErrorLookup.h"

contacts::contacts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_contacts, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
    connect(_edit,		SIGNAL(clicked()),	this, SLOT(sEdit()));
    connect(_view,		SIGNAL(clicked()),	this, SLOT(sView()));
    connect(_delete,		SIGNAL(clicked()),	this, SLOT(sDelete()));
    connect(_print,		SIGNAL(clicked()),	this, SLOT(sPrint()));
    connect(_close,		SIGNAL(clicked()),	this, SLOT(close()));
    connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
    connect(_activeOnly,	SIGNAL(toggled(bool)),	this, SLOT(sFillList()));

    _activeOnly->setChecked(true);
    
    _contacts->addColumn(tr("First Name"),    50, Qt::AlignLeft, true, "cntct_first_name");
    _contacts->addColumn(tr("Last Name"),    100, Qt::AlignLeft, true, "cntct_last_name");
    _contacts->addColumn(tr("Account #"),     80, Qt::AlignLeft, true, "crmacct_number");
    _contacts->addColumn(tr("Account Name"), 160, Qt::AlignLeft, true, "crmacct_name");
    _contacts->addColumn(tr("Phone"),	     100, Qt::AlignLeft, true, "cntct_phone");
    _contacts->addColumn(tr("Alternate"),    100, Qt::AlignLeft, true, "cntct_phone2");
    _contacts->addColumn(tr("Fax"),	     100, Qt::AlignLeft, true, "cntct_fax");
    _contacts->addColumn(tr("E-Mail"),	     100, Qt::AlignLeft, true, "cntct_email");
    _contacts->addColumn(tr("Web Address"),  100, Qt::AlignLeft, true, "cntct_webaddr");

    if (_privileges->check("MaintainContacts"))
    {
      connect(_contacts, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_contacts, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_contacts, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_contacts, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    sFillList();
}

contacts::~contacts()
{
    // no need to delete child widgets, Qt does it all for us
}

void contacts::languageChange()
{
    retranslateUi(this);
}

void contacts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainContacts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainContacts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void contacts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contacts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _contacts->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contacts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _contacts->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void contacts::sDelete()
{
  q.prepare("SELECT deleteContact(:cntct_id) AS result;");
  q.bindValue(":cntct_id", _contacts->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteContact", result), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contacts::setParams(ParameterList &params)
{
  if (_activeOnly->isChecked())
    params.append("activeOnly");
}

void contacts::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ContactsMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void contacts::sFillList()
{
  QString sql("SELECT cntct_id, cntct_first_name, cntct_last_name, "
	      "       crmacct_number, crmacct_name, cntct_phone, "
	      "       cntct_phone2, cntct_fax, cntct_email, cntct_webaddr "
	      "FROM cntct LEFT OUTER JOIN crmacct ON (cntct_crmacct_id=crmacct_id) "
	      "<? if exists(\"activeOnly\") ?> WHERE cntct_active <? endif ?>"
	      "ORDER BY cntct_last_name, cntct_first_name, crmacct_number;");
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _contacts->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
