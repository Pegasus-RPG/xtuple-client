/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslkeyproto.h"

QScriptValue QSslKeytoScriptValue(QScriptEngine *engine, QSslKey* const &item)
{
  return engine->newQObject(item);
}

void QSslKeyfromScriptValue(const QScriptValue &obj, QSslKey* &item)
{
  item = qobject_cast<QSslKey*>(obj.toQObject());
}

void setupQSslKeyProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QSslKeytoScriptValue, QSslKeyfromScriptValue);

  QScriptValue proto = engine->newQObject(new QSslKeyProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslKey*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslKey>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSslKey,
                                                 proto);
  engine->globalObject().setProperty("QSslKey",  constructor);
}

#include <QSslKey>
QScriptValue constructQSslKey(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslKey *obj = 0;
  /* if (context->argumentCount() ...)
  else if (something bad)
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSslKeyconstructor");
  else
  */
    obj = new QSslKey();
  return engine->toScriptValue(obj);
}

QSslKeyProto::QSslKeyProto(QObject *parent)
    : QObject(parent)
{
}

QSsl::KeyAlgorithm QSslKeyProto::algorithm() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->algorithm();
  return QSsl::KeyAlgorithm();
}

void QSslKeyProto::clear()
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    item->clear();
}

Qt::HANDLE QSslKeyProto::handle() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->handle();
  return Qt::HANDLE();
}

bool QSslKeyProto::isNull() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

int QSslKeyProto::length() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

void QSslKeyProto::swap(QSslKey & other)
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    item->swap(other);
}

QByteArray QSslKeyProto::toDer(const QByteArray & passPhrase) const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->toDer(passPhrase);
  return QByteArray();
}

QByteArray QSslKeyProto::toPem(const QByteArray & passPhrase) const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->toPem(passPhrase);
  return QByteArray();
}

QSsl::KeyType QSslKeyProto::type() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return item->type();
  return QSsl::KeyType();
}

QString QSslKeyProto::toString() const
{
  QSslKey *item = qscriptvalue_cast<QSslKey*>(thisObject());
  if (item)
    return QString("QSslKey()");
  return QString("QSslKey(unknown)");
}
