/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxRegistration.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

taxRegistration::taxRegistration(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_number,  SIGNAL(textChanged(QString)), this, SLOT(sHandleButtons()));
  connect(_save,    SIGNAL(clicked()),		  this, SLOT(sSave()));
  connect(_taxauth, SIGNAL(newID(int)),		  this, SLOT(sHandleButtons()));

  _taxregid = -1;
  _reltype  = "";
  _relid    = -1;
  _mode     = cNew;
}

taxRegistration::~taxRegistration()
{
  // no need to delete child widget, Qt does it all for us
}

void taxRegistration::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxRegistration::set(const ParameterList pParams)
{
  QVariant param;
  bool	   valid;

  param = pParams.value("taxreg_id", &valid);
  if (valid)
  {
    _taxregid = param.toInt();
    sPopulate();
  }

  param = pParams.value("taxreg_rel_type", &valid);
  if (valid)
    _reltype = param.toString();

  param = pParams.value("taxreg_rel_id", &valid);
  if (valid)
    _relid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('taxreg_taxreg_id_seq') AS _taxreg_id;");
      if (q.first())
        _taxregid = q.value("_taxreg_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _cust->setEnabled(false);
      _vend->setEnabled(false);
      _taxauth->setEnabled(false);
      _number->setEnabled(false);

      _close->setText(tr("Close"));
      _save->hide();
    }
  }

  if (handleReltype() < 0)
    return UndefinedError;

  sHandleButtons();

  return NoError;
}

void taxRegistration::sSave()
{
  q.prepare("SELECT taxreg_id"
            "  FROM taxreg"
            " WHERE((taxreg_id!= :taxreg_id)"
            "   AND (taxreg_taxauth_id=:taxreg_taxauth_id)"
            "   AND (taxreg_number=:taxreg_number))");
  q.bindValue(":taxreg_id", _taxregid);
  q.bindValue(":taxreg_taxauth_id", _taxauth->id());
  q.bindValue(":taxreg_number", _number->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Tax Registration"),
      tr("A Tax Registration already exists for the parameters specified.") );
    _taxauth->setFocus();
    return;
  }

  if (cNew == _mode)
  {
    q.prepare("INSERT INTO taxreg (taxreg_id,"
	      "    taxreg_rel_id, taxreg_rel_type, "
	      "    taxreg_taxauth_id, taxreg_number "
	      " ) VALUES (:taxreg_id,"
	      "    :taxreg_rel_id, :taxreg_rel_type, "
	      "    :taxreg_taxauth_id, :taxreg_number "
	      " );");
  }
  else
  {
    q.prepare("UPDATE taxreg SET "
	      "    taxreg_rel_id=:taxreg_rel_id, "
	      "    taxreg_rel_type=:taxreg_rel_type, "
	      "    taxreg_taxauth_id=:taxreg_taxauth_id, "
	      "    taxreg_number=:taxreg_number "
	      "WHERE (taxreg_id=:taxreg_id);");
  }
  q.bindValue(":taxreg_id", _taxregid);
  q.bindValue(":taxreg_rel_id", _relid); //_reltype == "C" ? _cust->id() : _vend->id());
  if(!_reltype.isEmpty())
    q.bindValue(":taxreg_rel_type", _reltype);
  q.bindValue(":taxreg_taxauth_id", _taxauth->id());
  q.bindValue(":taxreg_number", _number->text());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  accept();
}

void taxRegistration::sPopulate()
{
  q.prepare("SELECT * FROM taxreg WHERE (taxreg_id=:taxreg_id);");
  q.bindValue(":taxreg_id", _taxregid);
  q.exec();
  if (q.first())
  {
    _taxregid	= q.value("taxreg_id").toInt();
    _reltype	= q.value("taxreg_rel_type").toString();
    _relid	= q.value("taxreg_rel_id").toInt();
    _number->setText(q.value("taxreg_number").toString());
    _taxauth->setId(q.value("taxreg_taxauth_id").toInt());
    if (handleReltype() < 0)
      return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxRegistration::sHandleButtons()
{
  _save->setEnabled( (! _number->text().isEmpty() && _taxauth->isValid()) );
}

int taxRegistration::handleReltype()
{
  if (_reltype == "C")
  {
    _cust->setId(_relid);
    _cust->setVisible(true);
    _vend->setVisible(false);
  }
  else if (_reltype == "V")
  {
    _vend->setId(_relid);
    _cust->setVisible(false);
    _vend->setVisible(true);
  }
  else
  {
    _cust->setVisible(false);
    _vend->setVisible(false);
  }

  return 0;
}
