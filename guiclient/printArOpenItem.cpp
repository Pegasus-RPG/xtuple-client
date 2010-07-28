/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printArOpenItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

printArOpenItem::printArOpenItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = FALSE;
}

printArOpenItem::~printArOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void printArOpenItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printArOpenItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropen->setId(param.toInt());
    _print->setFocus();
  }

  return NoError;
}

void printArOpenItem::sPrint()
{
  QString doctype;

  XSqlQuery qry;
  qry.prepare("SELECT aropen_doctype "
              "FROM aropen "
              "WHERE (aropen_id=:aropen_id);");
  qry.bindValue(":aropen_id", _aropen->id());
  qry.exec();
  if (qry.first())
    doctype = qry.value("aropen_doctype").toString();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("aropen_id", _aropen->id());
  if ("I" == doctype || "D" == doctype)
    params.append("docTypeID", 1);
  else if ("R" == doctype || "C" == doctype)
    params.append("docTypeRC", 1);

  orReport report("AROpenItem", params);

  if (report.isValid())
  {
    if (report.print())
      emit finishedPrinting(_aropen->id());
    accept();
  }
  else
  {
    report.reportError(this);
    reject();
  }
}
