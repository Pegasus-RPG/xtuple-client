/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glTransactionDetail.h"

#include <QMessageBox>
#include <QVariant>

glTransactionDetail::glTransactionDetail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _amount->setPrecision(omfgThis->moneyVal());

  _gltransid = -1;
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

  return NoError;
}

void glTransactionDetail::populate()
{
  q.prepare("SELECT *,"
            "       (gltrans_doctype || ' ' || gltrans_docnumber) AS f_document,"
            "       formatGLAccountLong(gltrans_accnt_id) AS f_accnt "
            "  FROM gltrans"
            " WHERE (gltrans_id=:gltrans_id);");
  q.bindValue(":gltrans_id", _gltransid);
  q.exec();
  if(q.first())
  {
    _date->setDate(q.value("gltrans_date").toDate());
    _source->setText(q.value("gltrans_source").toString());
    _document->setText(q.value("f_document").toString());
    _journalnumber->setText(q.value("gltrans_journalnumber").toString());
    _accnt->setText(q.value("f_accnt").toString());
    _amount->setDouble(q.value("gltrans_amount").toDouble());
    _username->setText(q.value("gltrans_username").toString());
    _created->setDate(q.value("gltrans_created").toDate());
    _posted->setText(q.value("gltrans_posted").toBool() ? tr("Yes") : tr("No"));
    _notes->setText(q.value("gltrans_notes").toString());
  }
}
