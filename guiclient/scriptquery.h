/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __SCRIPTQUERY_H__
#define __SCRIPTQUERY_H__

#include <QObject>
#include <QVariant>
#include <QScriptValue>

#include <xsqlquery.h>

class QScriptEngine;

class ScriptQuery : public QObject
{
  Q_OBJECT

  public:
    ScriptQuery(QScriptEngine * engine);
    virtual ~ScriptQuery();

    Q_INVOKABLE XSqlQuery query() const;
    Q_INVOKABLE void setQuery(XSqlQuery query);
    Q_INVOKABLE QSqlRecord record() const;
    Q_INVOKABLE QString    toString() const;

  public slots:
    bool isActive();
    bool isValid();
    bool isForwardOnly();
    bool isSelect();

    bool first();
    bool last();
    bool next();
    bool previous();

    int size();
    int numRowsAffected();

    QScriptValue value(int index);
    QScriptValue value(const QString & field);

    QVariantMap lastError();

  private:
    QScriptEngine * _engine;
    XSqlQuery _query;
};

#endif // __SCRIPTQUERY_H__
