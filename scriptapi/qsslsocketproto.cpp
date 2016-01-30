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


#if QT_VERSION < 0x050000
void setupQSslSocketProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

QScriptValue PeerVerifyModeToScriptValue(QScriptEngine *engine, const QSslSocket::PeerVerifyMode &item)
{
  return engine->newVariant(item);
}
void PeerVerifyModeFromScriptValue(const QScriptValue &obj, QSslSocket::PeerVerifyMode &item)
{
  item = (QSslSocket::PeerVerifyMode)obj.toInt32();
}

QScriptValue SslModeToScriptValue(QScriptEngine *engine, const QSslSocket::SslMode &item)
{
  return engine->newVariant(item);
}
void SslModeFromScriptValue(const QScriptValue &obj, QSslSocket::SslMode &item)
{
  item = (QSslSocket::SslMode)obj.toInt32();
}

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
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QSslSocketProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslSocket*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslSocket,
                                                 proto);
  engine->globalObject().setProperty("QSslSocket",  constructor);

  qScriptRegisterMetaType(engine, PeerVerifyModeToScriptValue, PeerVerifyModeFromScriptValue);
  constructor.setProperty("VerifyNone", QScriptValue(engine, QSslSocket::VerifyNone), permanent);
  constructor.setProperty("QueryPeer", QScriptValue(engine, QSslSocket::QueryPeer), permanent);
  constructor.setProperty("VerifyPeer", QScriptValue(engine, QSslSocket::VerifyPeer), permanent);
  constructor.setProperty("AutoVerifyPeer", QScriptValue(engine, QSslSocket::AutoVerifyPeer), permanent);

  qScriptRegisterMetaType(engine, SslModeToScriptValue, SslModeFromScriptValue);
  constructor.setProperty("UnencryptedMode", QScriptValue(engine, QSslSocket::UnencryptedMode), permanent);
  constructor.setProperty("SslClientMode", QScriptValue(engine, QSslSocket::SslClientMode), permanent);
  constructor.setProperty("SslServerMode", QScriptValue(engine, QSslSocket::SslServerMode), permanent);

}

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
    : QTcpSocketProto(parent)
{
}

#endif
