/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslcipherproto.h"

#if QT_VERSION < 0x050000
void setupQSslCipherProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQSslCipherProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslCipherProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslCipher*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslCipher,
                                                 proto);
  engine->globalObject().setProperty("QSslCipher",  constructor);
}

QScriptValue constructQSslCipher(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  QSslCipher *obj = 0;
  if (context->argumentCount() == 1) {
    obj = new QSslCipher(context->argument(0).toString());
  }
  else if (context->argumentCount() == 2) {
    obj = new QSslCipher(context->argument(0).toString(), static_cast<QSsl::SslProtocol>(context->argument(1).toInt32()));
  }
  else {
    obj = new QSslCipher();
  }
  return engine->toScriptValue(obj);
}

QSslCipherProto::QSslCipherProto(QObject *parent)
    : QObject(parent)
{
}
QSslCipherProto::~QSslCipherProto()
{
}

QString QSslCipherProto::authenticationMethod() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->authenticationMethod();
  return QString();
}

QString QSslCipherProto::encryptionMethod() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->encryptionMethod();
  return QString();
}

bool QSslCipherProto::isNull() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QString QSslCipherProto::keyExchangeMethod() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->keyExchangeMethod();
  return QString();
}

QString QSslCipherProto::name() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QSsl::SslProtocol QSslCipherProto::protocol() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->protocol();
  return QSsl::SslProtocol();
}

QString QSslCipherProto::protocolString() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->protocolString();
  return QString();
}

int QSslCipherProto::supportedBits() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->supportedBits();
  return 0;
}

void QSslCipherProto::swap(QSslCipher & other)
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    item->swap(other);
}

int QSslCipherProto::usedBits() const
{
  QSslCipher *item = qscriptvalue_cast<QSslCipher*>(thisObject());
  if (item)
    return item->usedBits();
  return 0;
}

#endif
