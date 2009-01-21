/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxSelections.h"

#include <QSqlError>

#include "metasql.h"
#include "parameter.h"

#include "guiclient.h"
#include "taxSelection.h"

taxSelections::taxSelections(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,	SIGNAL(clicked()),	 this, SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	 this, SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	 this, SLOT(sNew()));
  connect(_showAll,	SIGNAL(toggled(bool)),	 this, SLOT(sFillList()));
  connect(_taxsel,	SIGNAL(valid(bool)),	_view, SLOT(setEnabled(bool)));
  connect(_taxauth,	SIGNAL(newID(int)),	 this, SLOT(sFillList()));
  connect(_view,	SIGNAL(clicked()),	 this, SLOT(sView()));

  if (_privileges->check("MaintainTaxSel"))
  {
    connect(_taxsel,	SIGNAL(valid(bool)),  _delete, SLOT(setEnabled(bool)));
    connect(_taxsel,	SIGNAL(valid(bool)),	_edit, SLOT(setEnabled(bool)));
    connect(_taxsel, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_taxsel, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _taxsel->addColumn(tr("Tax Authority"),	 -1,  Qt::AlignLeft,   true,  "taxauth"  );
  _taxsel->addColumn(tr("Tax Type"),      100,  Qt::AlignLeft,   true,  "taxtype"  );
  _taxsel->addColumn(tr("Tax Code"),      100,  Qt::AlignLeft,   true,  "tax"  );

  sFillList();
}

taxSelections::~taxSelections()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxSelections::languageChange()
{
  retranslateUi(this);
}

void taxSelections::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxSelection newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxSelections::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxsel_id", _taxsel->id());

  taxSelection newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxSelections::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxsel_id", _taxsel->id());

  taxSelection newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxSelections::sDelete()
{
  q.prepare("DELETE FROM taxsel WHERE (taxsel_id=:taxsel_id);");
  q.bindValue(":taxsel_id", _taxsel->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void taxSelections::sFillList()
{
  ParameterList params;
  if (! _showAll->isChecked())
    params.append("taxauth_id", _taxauth->id());
  params.append("any", tr("~Any~"));

  QString sql("SELECT taxsel_id, "
	      "       COALESCE(taxauth_code, <? value(\"any\") ?>) AS taxauth, "
	      "       COALESCE(taxtype_name, <? value(\"any\") ?>) AS taxtype, "
	      "       COALESCE(tax_code, '') AS tax "
	      "FROM taxsel LEFT OUTER JOIN "
	      "     taxauth ON (taxsel_taxauth_id=taxauth_id) LEFT OUTER JOIN "
	      "     taxtype ON (taxsel_taxtype_id=taxtype_id) LEFT OUTER JOIN "
	      "     tax     ON (taxsel_tax_id=tax_id)"
	      "<? if exists(\"taxauth_id\") ?>"
	      "WHERE ((taxsel_taxauth_id = <? value(\"taxauth_id\") ?>) "
	      "  OR   (taxsel_taxauth_id IS NULL))"
	      "<? endif ?>"
	      "ORDER BY taxauth_id, taxtype_id, tax"
	      ";");
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _taxsel->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
