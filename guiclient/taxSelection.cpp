/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxSelection.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>


taxSelection::taxSelection(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));

  _mode		= cNew;
  _taxselId	= -1;
}

taxSelection::~taxSelection()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxSelection::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxSelection::set(const ParameterList& pParams)
{
  QVariant	param;
  bool		valid;

  param = pParams.value("taxsel_id", &valid);
  if (valid)
  {
    _taxselId = param.toInt();
    sPopulate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _taxauth->setFocus();

      q.exec("SELECT NEXTVAL('taxsel_taxsel_id_seq') AS _taxsel_id;");
      if (q.first())
        _taxselId = q.value("_taxsel_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _tax->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _taxauth->setEnabled(false);
      _taxtype->setEnabled(false);
      _tax->setEnabled(false);
      _save->hide();
      _close->setText(tr("Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void taxSelection::sSave()
{
  q.prepare("SELECT taxsel_id"
            "  FROM taxsel"
            " WHERE((taxsel_id != :taxsel_id)"
            "   AND (COALESCE(taxsel_taxauth_id, 0)=:taxsel_taxauth_id)"
            "   AND (COALESCE(taxsel_taxtype_id, 0)=:taxsel_taxtype_id)"
            "   AND (taxsel_tax_id=:taxsel_tax_id))");
  q.bindValue(":taxsel_id", _taxselId);
  if (_taxauth->isValid())
    q.bindValue(":taxsel_taxauth_id", _taxauth->id());
  else
    q.bindValue(":taxsel_taxauth_id", 0);
  if (_taxtype->isValid())
    q.bindValue(":taxsel_taxtype_id", _taxtype->id());
  else
    q.bindValue(":taxsel_taxtype_id", 0);
  if (_tax->isValid())
    q.bindValue(":taxsel_tax_id", _tax->id());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Tax Selection"),
      tr("A Tax Selection already exists for the parameters specified.") );
    _taxauth->setFocus();
    return;
  }

  if (cNew == _mode)
  {
    q.prepare("INSERT INTO taxsel (taxsel_id,"
	      "    taxsel_taxauth_id,  taxsel_taxtype_id,  taxsel_tax_id "
	      "  ) VALUES (:taxsel_id, "
	      "    :taxsel_taxauth_id, :taxsel_taxtype_id, :taxsel_tax_id "
	      ");");
  }
  else
  {
    q.prepare("UPDATE taxsel SET "
	      "    taxsel_taxauth_id=:taxsel_taxauth_id,"
	      "    taxsel_taxtype_id=:taxsel_taxtype_id,"
	      "    taxsel_tax_id=:taxsel_tax_id "
	      "WHERE (taxsel_id=:taxsel_id);");
  }
  q.bindValue(":taxsel_id", _taxselId);
  if (_taxauth->isValid())
    q.bindValue(":taxsel_taxauth_id", _taxauth->id());
  if (_taxtype->isValid())
    q.bindValue(":taxsel_taxtype_id", _taxtype->id());
  if (_tax->isValid())
    q.bindValue(":taxsel_tax_id", _tax->id());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void taxSelection::sPopulate()
{
  q.prepare("SELECT taxsel_taxauth_id, taxsel_taxtype_id, taxsel_tax_id "
	    "FROM taxsel "
	    "WHERE (taxsel_id=:taxsel_id);");
  q.bindValue(":taxsel_id", _taxselId);
  q.exec();
  if (q.first())
  {
    if (! q.value("taxsel_taxauth_id").isNull())
      _taxauth->setId(q.value("taxsel_taxauth_id").toInt());
    if (! q.value("taxsel_taxtype_id").isNull())
      _taxtype->setId(q.value("taxsel_taxtype_id").toInt());
    if (! q.value("taxsel_tax_id").isNull())
      _tax->setId(q.value("taxsel_tax_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
