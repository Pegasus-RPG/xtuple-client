/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glTransactionDetail.h"

#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

glTransactionDetail::glTransactionDetail(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _amount->setPrecision(omfgThis->moneyVal());

  _gltransid = -1;
  _table = "gltrans";
}

glTransactionDetail::~glTransactionDetail()
{
  // no need to delete child widgets, Qt does it all for us
}

void glTransactionDetail::languageChange()
{
  retranslateUi(this);
}

enum SetResponse glTransactionDetail::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("gltrans_id", &valid);
  if (valid)
  {
    _gltransid = param.toInt();
    populate();
  }

  param = pParams.value("sltrans_id", &valid);
  if (valid)
  {
    setWindowTitle(tr("Sub Ledger"));
    _table = "sltrans";
    _gltransid = param.toInt();
    populate();
  }

  return NoError;
}

void glTransactionDetail::populate()
{
  XSqlQuery glpopulate;
  QString sql("SELECT <? literal(\"table\") ?>_date AS transdate, "
            "<? literal(\"table\") ?>_source AS source, "
            "<? literal(\"table\") ?>_sequence AS sequence, "
            "<? literal(\"table\") ?>_journalnumber AS journalnumber, "
            "<? literal(\"table\") ?>_amount AS amount, "
            "<? literal(\"table\") ?>_username AS username, "
            "<? literal(\"table\") ?>_created AS created, "
            "<? literal(\"table\") ?>_posted AS posted, "
            "<? literal(\"table\") ?>_notes AS notes, "
            "       (<? literal(\"table\") ?>_doctype || ' ' || <? literal(\"table\") ?>_docnumber) AS f_document,"
            "       formatGLAccountLong(<? literal(\"table\") ?>_accnt_id) AS f_accnt "
            "  FROM <? literal(\"table\") ?> "
            " WHERE (<? literal(\"table\") ?>_id=<? value(\"gltrans_id\") ?>);");

  ParameterList params;
  params.append("gltrans_id", _gltransid);
  params.append("table", _table);
  MetaSQLQuery mql(sql);
  glpopulate = mql.toQuery(params);
  if(glpopulate.first())
  {
    _date->setDate(glpopulate.value("transdate").toDate());
    _source->setText(glpopulate.value("source").toString());
    _document->setText(glpopulate.value("f_document").toString());
    _journalnumber->setText(glpopulate.value("journalnumber").toString());
    _accnt->setText(glpopulate.value("f_accnt").toString());
    _amount->setDouble(glpopulate.value("amount").toDouble());
    _username->setText(glpopulate.value("username").toString());
    _created->setDate(glpopulate.value("created").toDate());
    _posted->setText(glpopulate.value("posted").toBool() ? tr("Yes") : tr("No"));
    _notes->setText(glpopulate.value("notes").toString());
    _documents->setType("JE");
    _documents->setId(glpopulate.value("sequence").toInt());
  }
}
