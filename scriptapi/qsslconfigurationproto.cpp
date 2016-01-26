/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslconfigurationproto.h"

QScriptValue QSslConfigurationtoScriptValue(QScriptEngine *engine, QSslConfiguration* const &item)
{
  return engine->newQObject(item);
}

void QSslConfigurationfromScriptValue(const QScriptValue &obj, QSslConfiguration* &item)
{
  item = qobject_cast<QSslConfiguration*>(obj.toQObject());
}

void setupQSslConfigurationProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QSslConfigurationtoScriptValue, QSslConfigurationfromScriptValue);

  QScriptValue proto = engine->newQObject(new QSslConfigurationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslConfiguration*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslConfiguration>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSslConfiguration,
                                                 proto);
  engine->globalObject().setProperty("QSslConfiguration",  constructor);
}

#include <QSslConfiguration>
QScriptValue constructQSslConfiguration(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslConfiguration *obj = 0;
  /* if (context->argumentCount() ...)
  else if (something bad)
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSslConfigurationconstructor");
  else
  */
    obj = new QSslConfiguration();
  return engine->toScriptValue(obj);
}

QSslConfigurationProto::QSslConfigurationProto(QObject *parent)
    : QObject(parent)
{
}

QList<QByteArray> QSslConfigurationProto::allowedNextProtocols() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->allowedNextProtocols();
  return QList<QByteArray>();
}

QList<QSslCertificate> QSslConfigurationProto::caCertificates() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->caCertificates();
  return QList<QSslCertificate>();
}

QList<QSslCipher> QSslConfigurationProto::ciphers() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->ciphers();
  return QList<QSslCipher>();
}

QVector<QSslEllipticCurve> QSslConfigurationProto::ellipticCurves() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->ellipticCurves();
  return QVector<QSslEllipticCurve>();
}

bool QSslConfigurationProto::isNull() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QSslCertificate QSslConfigurationProto::localCertificate() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->localCertificate();
  return QSslCertificate();
}

QList<QSslCertificate> QSslConfigurationProto::localCertificateChain() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->localCertificateChain();
  return QList<QSslCertificate>();
}

QByteArray QSslConfigurationProto::nextNegotiatedProtocol() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->nextNegotiatedProtocol();
  return QByteArray();
}

QSslConfiguration::NextProtocolNegotiationStatus QSslConfigurationProto::nextProtocolNegotiationStatus() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->nextProtocolNegotiationStatus();
  return QSslConfiguration::NextProtocolNegotiationStatus();
}

QSslCertificate QSslConfigurationProto::peerCertificate() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerCertificate();
  return QSslCertificate();
}

QList<QSslCertificate> QSslConfigurationProto::peerCertificateChain() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerCertificateChain();
  return QList<QSslCertificate>();
}

int QSslConfigurationProto::peerVerifyDepth() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerVerifyDepth();
  return 0;
}

QSslSocket::PeerVerifyMode QSslConfigurationProto::peerVerifyMode() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->peerVerifyMode();
  return QSslSocket::PeerVerifyMode();
}

QSslKey QSslConfigurationProto::privateKey() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->privateKey();
  return QSslKey();
}

QSsl::SslProtocol QSslConfigurationProto::protocol() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->protocol();
  return QSsl::SslProtocol();
}

QSslCipher QSslConfigurationProto::sessionCipher() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionCipher();
  return QSslCipher();
}

QSsl::SslProtocol QSslConfigurationProto::sessionProtocol() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionProtocol();
  return QSsl::SslProtocol();
}

QByteArray QSslConfigurationProto::sessionTicket() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionTicket();
  return QByteArray();
}

int QSslConfigurationProto::sessionTicketLifeTimeHint() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->sessionTicketLifeTimeHint();
  return 0;
}

void QSslConfigurationProto::setAllowedNextProtocols(const QList<QByteArray> & protocols)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setAllowedNextProtocols(protocols);
}

void QSslConfigurationProto::setCaCertificates(const QList<QSslCertificate> & certificates)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setCaCertificates(certificates);
}

void QSslConfigurationProto::setCiphers(const QList<QSslCipher> & ciphers)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setCiphers(ciphers);
}

void QSslConfigurationProto::setEllipticCurves(const QVector<QSslEllipticCurve> & curves)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setEllipticCurves(curves);
}

void QSslConfigurationProto::setLocalCertificate(const QSslCertificate & certificate)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setLocalCertificate(certificate);
}

void QSslConfigurationProto::setLocalCertificateChain(const QList<QSslCertificate> & localChain)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setLocalCertificateChain(localChain);
}

void QSslConfigurationProto::setPeerVerifyDepth(int depth)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setPeerVerifyDepth(depth);
}

void QSslConfigurationProto::setPeerVerifyMode(QSslSocket::PeerVerifyMode mode)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setPeerVerifyMode(mode);
}

void QSslConfigurationProto::setPrivateKey(const QSslKey & key)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setPrivateKey(key);
}

void QSslConfigurationProto::setProtocol(QSsl::SslProtocol protocol)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setProtocol(protocol);
}

void QSslConfigurationProto::setSessionTicket(const QByteArray & sessionTicket)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setSessionTicket(sessionTicket);
}

void QSslConfigurationProto::setSslOption(QSsl::SslOption option, bool on)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setSslOption(option, on);
}

void QSslConfigurationProto::swap(QSslConfiguration & other)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->swap(other);
}

bool QSslConfigurationProto::testSslOption(QSsl::SslOption option) const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->testSslOption(option);
  return false;
}

QSslConfiguration QSslConfigurationProto::defaultConfiguration()
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->defaultConfiguration();
  return QSslConfiguration();
}

void QSslConfigurationProto::setDefaultConfiguration(const QSslConfiguration & configuration)
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    item->setDefaultConfiguration(const QSslConfiguration & configuration);
}

QList<QSslCipher> QSslConfigurationProto::supportedCiphers()
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->supportedCiphers();
  return QList<QSslCipher>();
}

QVector<QSslEllipticCurve> QSslConfigurationProto::supportedEllipticCurves()
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->supportedEllipticCurves();
  return QVector<QSslEllipticCurve>();
}

QList<QSslCertificate> QSslConfigurationProto::systemCaCertificates()
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return item->systemCaCertificates();
  return QList<QSslCertificate>();
}

QString QSslConfigurationProto::toString() const
{
  QSslConfiguration *item = qscriptvalue_cast<QSslConfiguration*>(thisObject());
  if (item)
    return QString("QSslConfiguration()");
  return QString("QSslConfiguration(unknown)");
}
