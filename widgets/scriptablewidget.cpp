/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptablewidget.h"

#include <QDebug>
#include <QWidget>
#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValue>

#include "include.h"
#include "qtsetup.h"
#include "widgets.h"
#include "xsqlquery.h"

// _object lets us work with the widget we want to script without deriving
// ScriptableWidget from QObject, which would cause multiple inheritance headaches

ScriptableWidget::ScriptableWidget(QObject* object)
  : _debugger(0),
    _engine(0),
    _object(object),
    _scriptLoaded(false)
{
}

ScriptableWidget::~ScriptableWidget()
{
}

QScriptEngine *ScriptableWidget::engine()
{
  if (!_engine)
  {
    _engine = new QScriptEngine(_object);
    if (_x_preferences && _x_preferences->boolean("EnableScriptDebug"))
    {
      _debugger = new QScriptEngineDebugger(_object);
      _debugger->attachTo(_engine);
    }

    setupQt(_engine);
    setupInclude(_engine);
    QScriptValue mywidget = _engine->newQObject(_object);
    _engine->globalObject().setProperty("mywidget",  mywidget);
  }

  return _engine;
}

void ScriptableWidget::loadScript(const QStringList &list)
{
  if (! engine())
  {
    qDebug() << "could not initialize engine" << list;
    return;
  }
  qDebug() << "Looking for scripts" << list;

  // make one query to get all relevant scripts, using a JSON object
  // to enforce load order: key = order, value = name
  QStringList pair;
  for (int i = 0; i < list.length(); i++)
  {
    pair.append(QString("\"%1\": \"%2\"").arg(i).arg(list.at(i)));
  }

  XSqlQuery scriptq;
  scriptq.prepare("WITH jsonlist AS (SELECT *"
                  "                    FROM json_each_text(:jsonlist))"
                  "SELECT script_source"
                  "  FROM script"
                  "  JOIN jsonlist ON script_name = value"
                  " WHERE script_enabled"
                  " ORDER BY key, script_order;");
  scriptq.bindValue(":jsonlist", "{" + pair.join(", ") + "}");
  scriptq.exec();
  while (scriptq.next())
  {
    QScriptValue result = engine()->evaluate(scriptq.value("script_source").toString(),
                                             _object->objectName());
    if (engine()->hasUncaughtException())
    {
      qDebug() << "uncaught exception at line"
               << engine()->uncaughtExceptionLineNumber()
               << ":" << result.toString();
    }
  }
}

void ScriptableWidget::loadScript(const QString& oName)
{
  loadScript(QStringList(oName));
}

void ScriptableWidget::loadScriptEngine()
{
  if (_scriptLoaded || !_object)
    return;
  _scriptLoaded = true;

  QStringList scriptList;

  // load scripts by class heirarchy name
  const QMetaObject *m = _object->metaObject();
  while(m)
  {
    scriptList.prepend(m->className());
    m = m->superClass();
  }

  // load scripts by object name
  QStringList parts = _object->objectName().split(" ");
  QStringList search_parts;
  QString oName;
  while(!parts.isEmpty())
  {
    search_parts.append(parts.takeFirst());
    oName = search_parts.join(" ");
    scriptList.append(oName);
  }

  scriptList.removeDuplicates();
  loadScript(scriptList);
}
