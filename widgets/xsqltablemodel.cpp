/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlIndex>
#include <QSqlField>

#include "xsqltablemodel.h"

XSqlTableModel::XSqlTableModel(QObject *parent) : 
  QSqlRelationalTableModel(parent)
{
}

XSqlTableModel::~XSqlTableModel()
{
}

void XSqlTableModel::setTable(const QString &tableName, int keyColumns)
{
  QSqlRelationalTableModel::setTable(tableName);
  setKeys(keyColumns);
}

void XSqlTableModel::setKeys(int keyColumns)
{
  if (keyColumns && tableName().length())
  {
    QSqlRecord rec = database().record(tableName());
    QSqlIndex idx(tableName());
    for (int i = 0; i < keyColumns; i++)
      idx.append(rec.field(i));
    idx.setName(tableName() + "_pkey");
    QSqlRelationalTableModel::setPrimaryKey(idx);
  }
}



