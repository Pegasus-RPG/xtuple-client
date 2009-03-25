/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "searchForContact.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "contact.h"

searchForContact::searchForContact(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_edit,	 SIGNAL(clicked()),	this, SLOT(sEdit()));
  connect(_search,	 SIGNAL(lostFocus()),	this, SLOT(sFillList()));
  connect(_searchStreet, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCity,   SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchState,  SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPostalCode,SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCountry,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchContact,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchName,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchNumber, SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPhone,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_showInactive, SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_cntct,	 SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu *)));
  connect(_view,	 SIGNAL(clicked()),	this, SLOT(sView()));

  _cntct->addColumn(tr("First"),       50, Qt::AlignLeft           , true, "cntct_first_name");
  _cntct->addColumn(tr("Last"),        -1, Qt::AlignLeft           , true, "cntct_last_name" );
  _cntct->addColumn(tr("Account Number"),      80, Qt::AlignCenter , true, "number");
  _cntct->addColumn(tr("Account Name"),        -1, Qt::AlignLeft   , true, "name");
  _cntct->addColumn(tr("Phone"),      100, Qt::AlignLeft           , true, "cntct_phone");
  _cntct->addColumn(tr("Email"),      100, Qt::AlignLeft           , true, "cntct_email");
  _cntct->addColumn(tr("Address"),     -1, Qt::AlignLeft           , true, "addr_line1");
  _cntct->addColumn(tr("City"),        75, Qt::AlignLeft           , true, "addr_city");
  _cntct->addColumn(tr("State"),       50, Qt::AlignLeft           , true, "addr_state");
  _cntct->addColumn(tr("Country"),    100, Qt::AlignLeft           , true, "addr_country");
  _cntct->addColumn(tr("Postal Code"), 75, Qt::AlignLeft           , true, "addr_postalcode");
  
  _editpriv = _privileges->check("MaintainContacts");
  _viewpriv = _privileges->check("ViewContacts");
  if (_editpriv)
  {
    connect(_cntct, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_cntct, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else if (_viewpriv)
    connect(_cntct, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    
}

searchForContact::~searchForContact()
{
  // no need to delete child widgets, Qt does it all for us
}

void searchForContact::languageChange()
{
  retranslateUi(this);
}

void searchForContact::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, _editpriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, _viewpriv);
}

void searchForContact::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _cntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForContact::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _cntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void searchForContact::sFillList()
{
  if (_search->text().trimmed().length() == 0)
  {
    _cntct->clear();
    return;
  }

  QString sql;

  sql = "SELECT cntct_id, addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
	"       cntct.* "
    "    FROM cntct LEFT OUTER JOIN"
	"         crmacct ON (cntct_crmacct_id=crmacct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> cntct_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(crmacct_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(crmacct_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? if exists(\"searchContactName\") ?>"
	"   OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"             ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchPhone\") ?>"
	"   OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"             cntct_fax) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchEmail\") ?>"
	"   OR (cntct_email ~* <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchStreetAddr\") ?>"
	"   OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"             addr_line3) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchCity\") ?>"
	"   OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchState\") ?>"
	"   OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchPostalCode\") ?>"
	"   OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchCountry\") ?>"
	"   OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"  )"
	"ORDER BY number;"  ;

  MetaSQLQuery mql(sql);
  ParameterList params;
  params.append("searchString", _search->text().trimmed().toUpper());

  if (! _showInactive->isChecked())
    params.append("activeOnly");

  if (_searchNumber->isChecked())
    params.append("searchNumber");

  if (_searchName->isChecked())
    params.append("searchName");
    
  if (_searchContact->isChecked())
    params.append("searchContactName");

  if (_searchPhone->isChecked())
    params.append("searchPhone");
    
  if (_searchEmail->isChecked())
    params.append("searchEmail");

  if (_searchStreet->isChecked())
    params.append("searchStreetAddr");

  if (_searchCity->isChecked())
    params.append("searchCity");

  if (_searchState->isChecked())
    params.append("searchState");

  if (_searchPostalCode->isChecked())
    params.append("searchPostalCode");

  if (_searchCountry->isChecked())
    params.append("searchCountry");

  XSqlQuery fillq = mql.toQuery(params);
  if (fillq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _cntct->populate(fillq);
}
