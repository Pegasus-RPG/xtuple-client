/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomersByCharacteristic.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "characteristicAssignment.h"
#include "customer.h"
#include "guiclient.h"

dspCustomersByCharacteristic::dspCustomersByCharacteristic(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cust, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_customers) "
                   "ORDER BY char_name;" );

  _cust->addColumn(tr("Active"),         _ynColumn,   Qt::AlignLeft,  true, "cust_active");
  _cust->addColumn(tr("Number"),         _itemColumn, Qt::AlignLeft,  true, "cust_number");
  _cust->addColumn(tr("Name"),           -1,          Qt::AlignLeft,  true, "cust_name");
  _cust->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignCenter,true, "char_name");
  _cust->addColumn(tr("Value"),          _itemColumn, Qt::AlignLeft,  true, "charass_value");

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

dspCustomersByCharacteristic::~dspCustomersByCharacteristic()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCustomersByCharacteristic::languageChange()
{
  retranslateUi(this);
}

void dspCustomersByCharacteristic::setParams(ParameterList &params)
{
  params.append("char_id", _char->id());
  params.append("value", _value->text());

  if (_showInactive->isChecked())
    params.append("showInactive");
  if (_emptyValue->isChecked())
    params.append("emptyValue");
  if(_charIsSet->isChecked())
    params.append("hasCharacteristic");
}

void dspCustomersByCharacteristic::sPrint()
{
  ParameterList params;

  params.append("char_id", _char->id());
  params.append("value", _value->text());

  if(_showInactive->isChecked())
    params.append("showInactive");

  if(_emptyValue->isChecked())
    params.append("emptyValue");

  if(_charIsSet->isChecked())
    params.append("hasCharacteristic");

  orReport report("CustomersByCharacteristic", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCustomersByCharacteristic::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Customer..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Customer..."), this, SLOT(sView()), 0);

  if (((XTreeWidgetItem *)pSelected)->altId() == -1)
  {
    menuItem = pMenu->insertItem(tr("New Characteristic..."), this, SLOT(sNewCharacteristic()), 0);
    if (!_privileges->check("MaintainCustomerMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Edit Characteristic..."), this, SLOT(sEditCharacteristic()), 0);
    if (!_privileges->check("MaintainCustomerMasters"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspCustomersByCharacteristic::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCharacteristic::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCharacteristic::sNewCharacteristic()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());
  params.append("char_id", _char->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCustomersByCharacteristic::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _cust->altId());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCustomersByCharacteristic::sFillList()
{
  sFillList(-1, TRUE);
}

void dspCustomersByCharacteristic::sFillList(int pCustid, bool pLocal)
{
  QString sql =
		"<? if exists(\"hasCharacteristic\") ?>"
    "SELECT cust_id, charass_id, cust_active, cust_number, cust_name, char_name, charass_value "
		"FROM cust, charass, char "
		"WHERE ( (charass_target_type='C')"
		" AND (charass_target_id=cust_id)"
		" AND (charass_char_id=char_id)"
		" AND (char_id=<? value(\"char_id\") ?>)"
		" <? if exists(\"emptyValue\") ?>"
		" AND (charass_value IS NULL OR LENGTH(TRIM(charass_value)) = 0)"
		" <? else ?>"
		" AND (charass_value ~* <? value(\"value\") ?>) "
		" <? endif ?>"
		") "
		"<? else ?>"	// if does not have characteristic
    "SELECT cust_id, -1, cust_active, cust_number, cust_name, char_name, '' AS charass_value "
		"FROM cust, char "
		"WHERE ((cust_id NOT IN (SELECT charass_target_id"
		"                        FROM charass"
		"                        WHERE ((charass_target_type='C')"
		"                          AND  (charass_char_id=char_id))"
		"       ))"
		" AND (char_id=<? value(\"char_id\") ?>)"
		") "
		"<? endif ?>"
		"<? if not exists(\"showInactive\") ?>"
		" AND (cust_active)"
		"<? endif ?>"
	        "ORDER BY cust_number;";

  MetaSQLQuery mql(sql);
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);

  if ((pCustid != -1) && (pLocal))
    _cust->populate(q, pCustid, true);
  else
    _cust->populate(q, true);

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _cust->setDragString("custid=");
}
