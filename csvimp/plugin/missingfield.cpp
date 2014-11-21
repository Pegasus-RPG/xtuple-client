/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "missingfield.h"

#include <QSqlField>
#include <QSqlRecord>

MissingField::MissingField(QWidget *parent, const QString &field, const QSqlRecord &record) :
  QDialog(parent)
{
  setupUi(this);

  _lblField->setText(field);
  for (int i = 0; ! record.field(i).name().isEmpty(); i++)
    _fields->insertItem(i, record.field(i).name());
}

MissingField::~MissingField()
{
}

void MissingField::languageChange()
{
  retranslateUi(this);
}
