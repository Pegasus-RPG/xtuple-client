/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "mqlutil.h"

#include <QFile>
#include <QTextStream>

static QString _lastError;

QString mqlLastError() { return _lastError; }
void    mqlClearLastError() { _lastError = QString::null; }

MetaSQLQuery mqlLoad(const QString & name, bool * valid)
{
  if(valid)
    *valid = true;
  QString fsrc = QString::null;
  QFile fin(name);
  if(fin.open(QIODevice::ReadOnly))
  {
    QTextStream sin(&fin);
    fsrc = sin.readAll();
  }
  else
  {
    _lastError = QString("Could not open file '%1': %2").arg(name).arg(fin.errorString());
    qWarning("%s", qPrintable(_lastError));
    if(valid)
      *valid = false;
  }

  return MetaSQLQuery(fsrc);
}

MetaSQLQuery mqlLoad(const QString & group, const QString & name, bool * valid)
{
  if(valid)
    *valid = true;
  QString fsrc = QString::null;
  XSqlQuery qry;
  QString sql;
  sql = QString("SELECT metasql_query "
              "FROM metasql "
              "WHERE ((metasql_group='%1') "
              "AND (metasql_name='%2'));").arg(group).arg(name);
  qry.exec(sql);
  if (qry.first())
    fsrc = qry.value("metasql_query").toString();
  else
  {
    _lastError = QString("Could not open query '%1': %2").arg(group).arg(name);
    qWarning("%s", qPrintable(_lastError));
    if(valid)
      *valid = false;
  }

  return MetaSQLQuery(fsrc);
}

