/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslsocketproto.h"

QScriptValue QSslSockettoScriptValue(QScriptEngine *engine, QSslSocket* const &item)
{
  return engine->newQObject(item);
}

void QSslSocketfromScriptValue(const QScriptValue &obj, QSslSocket* &item)
{
  item = qobject_cast<QSslSocket*>(obj.toQObject());
}

void setupQSslSocketProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QSslSockettoScriptValue, QSslSocketfromScriptValue);

  QScriptValue proto = engine->newQObject(new QSslSocketProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslSocket*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslSocket>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSslSocket,
                                                 proto);
  engine->globalObject().setProperty("QSslSocket",  constructor);
}

#include <QSslSocket>
QScriptValue constructQSslSocket(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslSocket *obj = 0;
  /* if (context->argumentCount() ...)
  else if (something bad)
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSslSocketconstructor");
  else
  */
    obj = new QSslSocket();
  return engine->toScriptValue(obj);
}

QSslSocketProto::QSslSocketProto(QObject *parent)
    : QObject(parent)
{
}

QString QSslSocketProto::toString() const
{
  QSslSocket *item = qscriptvalue_cast<QSslSocket*>(thisObject());
  if (item)
    return QString("QSslSocket()");
  return QString("QSslSocket(unknown)");
}
