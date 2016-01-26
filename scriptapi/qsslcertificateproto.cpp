/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslcertificateproto.h"

QScriptValue QSslCertificatetoScriptValue(QScriptEngine *engine, QSslCertificate* const &item)
{
  return engine->newQObject(item);
}

void QSslCertificatefromScriptValue(const QScriptValue &obj, QSslCertificate* &item)
{
  item = qobject_cast<QSslCertificate*>(obj.toQObject());
}

void setupQSslCertificateProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QSslCertificatetoScriptValue, QSslCertificatefromScriptValue);

  QScriptValue proto = engine->newQObject(new QSslCertificateProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslCertificate*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslCertificate>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSslCertificate,
                                                 proto);
  engine->globalObject().setProperty("QSslCertificate",  constructor);
}

#include <QSslCertificate>
QScriptValue constructQSslCertificate(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslCertificate *obj = 0;
  /* if (context->argumentCount() ...)
  else if (something bad)
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QSslCertificateconstructor");
  else
  */
    obj = new QSslCertificate();
  return engine->toScriptValue(obj);
}

QSslCertificateProto::QSslCertificateProto(QObject *parent)
    : QObject(parent)
{
}

void QSslCertificateProto::clear()
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    item->clear();
}

QByteArray QSslCertificateProto::digest(QCryptographicHash::Algorithm algorithm) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->digest(algorithm);
  return QByteArray();
}

QString QSslCertificateProto::effectiveDate() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->effectiveDate();
  return QString();
}

QString QSslCertificateProto::expiryDate() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->expiryDate();
  return QString();
}

QList<QSslCertificateExtension> QSslCertificateProto::extensions() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->extensions();
  return QList<QSslCertificateExtension>();
}

Qt::HANDLE QSslCertificateProto::handle() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->handle();
  return Qt::HANDLE();
}

bool QSslCertificateProto::isBlacklisted() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->isBlacklisted();
  return false;
}

bool QSslCertificateProto::isNull() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QSslCertificateProto::isSelfSigned() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->isSelfSigned();
  return false;
}

QStringList QSslCertificateProto::issuerInfo(QSslProto::SubjectInfo subject) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->issuerInfo(subject);
  return QStringList();
}

QStringList QSslCertificateProto::issuerInfo(const QByteArray & attribute) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->issuerInfo(attribute);
  return QStringList();
}

QList<QByteArray> QSslCertificateProto::issuerInfoAttributes() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->issuerInfoAttributes();
  return QList<QByteArray>();
}

QSslKey QSslCertificateProto::publicKey() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->publicKey();
  return QSslKey();
}

QByteArray QSslCertificateProto::serialNumber() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->serialNumber();
  return QByteArray();
}

QMultiMap<QSsl::AlternativeNameEntryType, QString> QSslCertificateProto::subjectAlternativeNames() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->();
  return QMultiMap<QSsl::AlternativeNameEntryType, QString>();
}

QStringList QSslCertificateProto::subjectInfo(QSslProto::SubjectInfo subject) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectInfo(subject);
  return QStringList();
}

QStringList QSslCertificateProto::subjectInfo(const QByteArray & attribute) const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectInfo(attribute);
  return QStringList();
}

QList<QByteArray> QSslCertificateProto::subjectInfoAttributes() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->subjectInfoAttributes();
  return QList<QByteArray>();
}

void QSslCertificateProto::swap(QSslCertificate & other)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    item->swap(QSslCertificate & other);
}

QByteArray QSslCertificateProto::toDer() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->toDer();
  return QByteArray();
}

QByteArray QSslCertificateProto::toPem() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->toPem();
  return QByteArray();
}

QString QSslCertificateProto::toText() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->toText();
  return QString();
}

QByteArray QSslCertificateProto::version() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->version();
  return QByteArray();
}

QList<QSslCertificate> QSslCertificateProto::fromData(const QByteArray & data, QSsl::EncodingFormat format)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->fromData(data, format);
  return QList<QSslCertificate>();
}

QList<QSslCertificate> QSslCertificateProto::fromDevice(QIODevice * device, QSsl::EncodingFormat format)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->fromDevice(device, format);
  return QList<QSslCertificate>();
}

QList<QSslCertificate> QSslCertificateProto::fromPath(const QString & path, QSsl::EncodingFormat format, QRegExp::PatternSyntax syntax)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->fromPath(path, format, syntax);
  return QList<QSslCertificate>();
}

bool QSslCertificateProto::importPkcs12(QIODevice * device, QSslKey * key, QSslCertificate * certificate, QList<QSslCertificate> * caCertificates, const QByteArray & passPhrase)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->importPkcs12(device, key, certificate, caCertificates, passPhrase);
  return false;
}

QList<QSslError> QSslCertificateProto::verify(const QList<QSslCertificate> & certificateChain, const QString & hostName)
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return item->verify(certificateChain, hostName);
  return QList<QSslError>();
}

QString QSslCertificateProto::toString() const
{
  QSslCertificate *item = qscriptvalue_cast<QSslCertificate*>(thisObject());
  if (item)
    return QString("QSslCertificate()");
  return QString("QSslCertificate(unknown)");
}
