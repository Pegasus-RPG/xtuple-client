/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "addresses.h"

#include <QMenu>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <openreports.h>
#include <metasql.h>
#include "addresses.h"
#include "address.h"

#include "storedProcErrorLookup.h"

addresses::addresses(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_address, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*, int)));
    connect(_edit,		SIGNAL(clicked()),	this, SLOT(sEdit()));
    connect(_view,		SIGNAL(clicked()),	this, SLOT(sView()));
    connect(_delete,		SIGNAL(clicked()),	this, SLOT(sDelete()));
    connect(_print,		SIGNAL(clicked()),	this, SLOT(sPrint()));
    connect(_close,		SIGNAL(clicked()),	this, SLOT(close()));
    connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
    connect(_activeOnly,	SIGNAL(toggled(bool)),	this, SLOT(sFillList()));

    _activeOnly->setChecked(true);
    
    _address->addColumn(tr("Line 1"),	 -1, Qt::AlignLeft, true, "addr_line1");
    _address->addColumn(tr("Line 2"),	 75, Qt::AlignLeft, true, "addr_line2");
    _address->addColumn(tr("Line 3"),	 75, Qt::AlignLeft, true, "addr_line3");
    _address->addColumn(tr("City"),	 75, Qt::AlignLeft, true, "addr_city");
    _address->addColumn(tr("State"),	 50, Qt::AlignLeft, true, "addr_state");
    _address->addColumn(tr("Country"),	 50, Qt::AlignLeft, true, "addr_country");
    _address->addColumn(tr("Postal Code"),50,Qt::AlignLeft, true, "addr_postalcode");

    if (_privileges->check("MaintainAddresses"))
    {
      connect(_address, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_address, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_address, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_address, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    sFillList();
}

addresses::~addresses()
{
  // no need to delete child widgets, Qt does it all for us
}

void addresses::languageChange()
{
  retranslateUi(this);
}

void addresses::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainAddresses"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainAddresses"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void addresses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  address newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void addresses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("addr_id", _address->id());

  address newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void addresses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("addr_id", _address->id());

  address newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void addresses::sDelete()
{
  q.prepare("SELECT deleteAddress(:addr_id) AS result;");
  q.bindValue(":addr_id", _address->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::warning(this, tr("Cannot Delete Selected Address"),
			   storedProcErrorLookup("deleteAddress", result));
      return;
    }
    else
      sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void addresses::sPrint()
{
  ParameterList params;
  if (_activeOnly->isChecked())
    params.append("activeOnly");

  orReport report("AddressesMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void addresses::sFillList()
{
  QString sql("SELECT addr_id, addr_line1, addr_line2, addr_line3, "
	      "       addr_city, addr_state, addr_country, addr_postalcode "
              "FROM addr "
	      "<? if exists(\"activeOnly\") ?> WHERE addr_active <? endif ?>"
              "ORDER BY addr_country, addr_state, addr_city, addr_line1;");
  ParameterList params;
  if (_activeOnly->isChecked())
    params.append("activeOnly");
  MetaSQLQuery mql(sql);
  XSqlQuery r = mql.toQuery(params);
  _address->populate(r);
}
