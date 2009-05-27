/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptquery.h"

#include <QSqlError>
#include <QScriptEngine>
#include <QRegExp>
#include <QDateTime>

#include "scripttoolbox.h"

ScriptQuery::ScriptQuery(QScriptEngine * engine)
  : QObject(engine)
{
  _engine = engine;
}

ScriptQuery::~ScriptQuery()
{
}

XSqlQuery ScriptQuery::query() const
{
  return _query;
}

void ScriptQuery::setQuery(XSqlQuery query)
{
  _query = query;
}

QSqlRecord ScriptQuery::record() const
{
  return _query.record();
}

bool ScriptQuery::isActive()
{
  return _query.isActive();
}

bool ScriptQuery::isValid()
{
  return _query.isValid();
}

bool ScriptQuery::isForwardOnly()
{
  return _query.isForwardOnly();
}

bool ScriptQuery::isSelect()
{
  return _query.isSelect();
}

bool ScriptQuery::first()
{
  return _query.first();
}

bool ScriptQuery::last()
{
  return _query.last();
}

bool ScriptQuery::next()
{
  return _query.next();
}

bool ScriptQuery::previous()
{
  return _query.previous();
}

int ScriptQuery::size()
{
  return _query.size();
}

int ScriptQuery::numRowsAffected()
{
  return _query.numRowsAffected();
}

QScriptValue ScriptQuery::value(int index)
{
  return ScriptToolbox::variantToScriptValue(_engine, _query.value(index));
}

QScriptValue ScriptQuery::value(const QString & field)
{
  return ScriptToolbox::variantToScriptValue(_engine, _query.value(field));
}

QVariantMap ScriptQuery::lastError()
{
  QVariantMap m;
  QSqlError err = _query.lastError();
  m.insert("databaseText", err.databaseText());
  m.insert("driverText", err.driverText());
  m.insert("text", err.text());
  m.insert("number", err.number());
  m.insert("type", err.type());
  m.insert("isValid", QVariant(err.isValid(), 0));

  return m;
}

