/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qobjectproto.h"

#include <QObject>

void setupQObjectProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QObjectProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QObject*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQObject,
                                                 proto);
  engine->globalObject().setProperty("QObject",  constructor);
}

QScriptValue constructQObject(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QObject *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QObject(qobject_cast<QObject*>(context->argument(0).toQObject()));
  else
    obj = new QObject();
  return engine->toScriptValue(obj);
}

QObjectProto::QObjectProto(QObject *parent)
    : QObject(parent)
{
}
