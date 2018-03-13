/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "mqlhash.h"

#include <QVariant>

#include "xsqlquery.h"

MqlHash::MqlHash(QObject *pParent, QSqlDatabase pDb)
  : XCachedHash<QString, QString>(pParent, QString(), pDb)
{
  setNotification(QStringList() << "metasql" << "pkgmetasql");
}

bool MqlHash::refresh(const QString &key)
{
  QStringList parts = key.split("%");   // must match value(pGroup, pName) below
  XSqlQuery q;
  q.prepare("SELECT metasql_query"
            "  FROM metasql"
            " WHERE metasql_group = :group AND metasql_name = :name"
            " ORDER BY metasql_grade DESC"
            " LIMIT 1;");
  q.bindValue(":group", QVariant(parts.at(0)));
  q.bindValue(":name",  QVariant(parts.at(1)));
  q.exec();
  if (q.first())
  {
    insert(key, q.value("metasql_query").toString());
    return true;
  }
  return false;
}

const QString MqlHash::value(const QString &pGroup, const QString &pName)
{
  return value(pGroup + "%" + pName);   // must match key.split() above
}

