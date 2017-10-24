/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "description.h"

#include "errorReporter.h"
#include "xsqlquery.h"

description::description(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) :
  XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
}

enum SetResponse description::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdthist_id", &valid);
  if (valid)
  {
    XSqlQuery text;
    text.prepare("SELECT incdthist_descrip "
                 "  FROM incdthist "
                 " WHERE incdthist_id=:incdthist_id;");
    text.bindValue(":incdthist_id", param.toInt());
    text.exec();
    if (text.first())
      _text->setText(text.value("incdthist_descrip").toString());
    else if(ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Description"),
                                 text, __FILE__, __LINE__))
      return UndefinedError;
  }

  return NoError;
}
