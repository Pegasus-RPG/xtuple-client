/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlDriver>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlRelation>

#include "xsqltablemodel.h"

XSqlTableModel::XSqlTableModel(QObject *parent) : 
  QSqlRelationalTableModel(parent)
{
}

XSqlTableModel::~XSqlTableModel()
{
}

bool XSqlTableModel::select()
{
  bool result;
  result = QSqlRelationalTableModel::select();
  if (result && rowCount());
    emit dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
  return result;
}

void XSqlTableModel::setTable(const QString &tableName, int keyColumns)
{
  QSqlRelationalTableModel::setTable(tableName);
  setKeys(keyColumns);
}

void XSqlTableModel::setKeys(int keyColumns)
{
  if (keyColumns && tableName().length()) {
    QSqlRecord rec = database().record(tableName());
    QSqlIndex idx(tableName());
    for (int i = 0; i < keyColumns; i++)
      idx.append(rec.field(i));
    idx.setName(tableName() + "_pkey");
    QSqlRelationalTableModel::setPrimaryKey(idx);
  }
}

/* qAppendWhereClause() and XSqlTableModel::selectStatement() have been
   copied from qt-mac-commercial-src-4.4.3/src/sql/models/qsqlrelationaltablemodel.cpp
   and hacked to work around Qt bug 171541:
     It would be useful if the QRelationalSqlTableModel would display rows with
     unresolvavble foreign keys
   http://www.qtsoftware.com/developer/task-tracker/index_html?method=entry&id=171541

   Unfortunately the original Qt code is full of references to private data structures,
   so this really is a hack.
   */

static void qAppendWhereClause(QString &query, const QString &clause1, const QString &clause2)
{
    if (clause1.isEmpty() && clause2.isEmpty())
        return;
    if (clause1.isEmpty() || clause2.isEmpty())
        query.append(QLatin1String(" WHERE (")).append(clause1).append(clause2);
    else
        query.append(QLatin1String(" WHERE (")).append(clause1).append(
                        QLatin1String(") AND (")).append(clause2);
    query.append(QLatin1String(") "));
}

QString XSqlTableModel::selectStatement() const
{
    QString query;

    if (tableName().isEmpty())
        return query;

    QString tList;
    QString fList;
    QString where;

    QSqlRecord rec = record();
    QStringList tables;

    // Count how many times each field name occurs in the record
    int fkeycount = 0;
    QHash<QString, int> fieldNames;
    for (int i = 0; i < rec.count(); ++i) {
        QSqlRelation rel = relation(i);
        QString name;
        if (rel.isValid()) {
            // Count the display column name, not the original foreign key
            name = rel.displayColumn();
            fkeycount++;
        }
        else
            name = rec.fieldName(i);
        fieldNames.insert(name, fieldNames.value(name, 0) + 1);
    }
    if (fkeycount == 0)
      return QSqlRelationalTableModel::selectStatement();

    for (int i = 0; i < rec.count(); ++i) {
        QSqlRelation rel = relation(i);
        if (rel.isValid()) {
            QString relTableAlias = QString::fromLatin1("%1_%2")
                                              .arg(rel.tableName()).arg(i);
            fList.append(relTableAlias + "." + rel.displayColumn());
            
            // If there are duplicate field names they must be aliased
            if (fieldNames.value(rel.displayColumn()) > 1)
                fList.append(QString::fromLatin1(" AS %1_%2")
                               .arg(relTableAlias)
                               .arg(rel.displayColumn()));

            fList.append(QLatin1Char(','));
            if (!tables.contains(rel.tableName()))
                tables.append(QString(" LEFT OUTER JOIN %1 %2 ON (%3.%4 = %5.%6) ")
                                 .arg(rel.tableName()) .arg(relTableAlias)
                                 .arg(tableName())     .arg(rec.fieldName(i))
                                 .arg(relTableAlias)   .arg(rel.indexColumn()));
        } else {
            fList.append(tableName() + "." + rec.fieldName(i));
            fList.append(QLatin1Char(','));
        }
    }
    if (!tables.isEmpty())
        tList.append(tables.join(QLatin1String(" "))).append(QLatin1String(" "));
    if (fList.isEmpty())
        return query;
    tList.prepend(QLatin1Char(' ')).prepend(database().driver()->escapeIdentifier(tableName(),
                QSqlDriver::TableName));
    // truncate tailing comma
    tList.chop(1);
    fList.chop(1);
    query.append(QLatin1String("SELECT "));
    query.append(fList).append(QLatin1String(" FROM ")).append(tList);
    if (!where.isEmpty())
        where.chop(5);
    qAppendWhereClause(query, where, filter());

    QString orderBy = orderByClause();
    if (!orderBy.isEmpty())
        query.append(QLatin1Char(' ')).append(orderBy);

    return query;
}

QVariant XSqlTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
      return QVariant();
        
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole: {
            return QSqlRelationalTableModel::data(index, role);
    } break;
    case Qt::TextAlignmentRole:
    case Qt::ForegroundRole:
      QPair<QModelIndex, int> key;
      key.first = index;
      key.second = role;
      if (roles.contains(key))
        return roles.value(key);
    }
    
    return QVariant(); 
}

bool XSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
      return false;
      
  switch (role) {
  case Qt::EditRole:
  case Qt::DisplayRole: {
    return QSqlRelationalTableModel::setData(index, value, role);
  } break;
  case Qt::TextAlignmentRole:
  case Qt::ForegroundRole:
    QPair<QModelIndex, int> key;
    key.first = index;
    key.second = role;
    if (roles.contains(key)) {
      if (roles.value(key) == value)
        return true;
    }
    roles.insert(key, value);
    emit dataChanged ( index, index );
  }
  return true;
}
