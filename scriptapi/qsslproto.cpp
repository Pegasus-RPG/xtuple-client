/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslproto.h"

#if QT_VERSION >= 0x050000
QScriptValue AlternativeNameEntryTypeToScriptValue(QScriptEngine *engine, const QSsl::AlternativeNameEntryType &item)
{
  return engine->newVariant(item);
}
void AlternativeNameEntryTypeFromScriptValue(const QScriptValue &obj, QSsl::AlternativeNameEntryType &item)
{
  item = (QSsl::AlternativeNameEntryType)obj.toInt32();
}

QScriptValue EncodingFormatToScriptValue(QScriptEngine *engine, const QSsl::EncodingFormat &item)
{
  return engine->newVariant(item);
}
void EncodingFormatFromScriptValue(const QScriptValue &obj, QSsl::EncodingFormat &item)
{
  item = (QSsl::EncodingFormat)obj.toInt32();
}

QScriptValue KeyAlgorithmToScriptValue(QScriptEngine *engine, const QSsl::KeyAlgorithm &item)
{
  return engine->newVariant(item);
}
void KeyAlgorithmFromScriptValue(const QScriptValue &obj, QSsl::KeyAlgorithm &item)
{
  item = (QSsl::KeyAlgorithm)obj.toInt32();
}

QScriptValue KeyTypeToScriptValue(QScriptEngine *engine, const QSsl::KeyType &item)
{
  return engine->newVariant(item);
}
void KeyTypeFromScriptValue(const QScriptValue &obj, QSsl::KeyType &item)
{
  item = (QSsl::KeyType)obj.toInt32();
}

QScriptValue SslOptionToScriptValue(QScriptEngine *engine, const QSsl::SslOption &item)
{
  return engine->newVariant(item);
}
void SslOptionFromScriptValue(const QScriptValue &obj, QSsl::SslOption &item)
{
  item = (QSsl::SslOption)obj.toInt32();
}

QScriptValue SslOptionsToScriptValue(QScriptEngine *engine, const QSsl::SslOptions &item)
{
  return engine->newVariant(item);
}
void SslOptionsFromScriptValue(const QScriptValue &obj, QSsl::SslOptions &item)
{
  item = (QSsl::SslOptions)obj.toInt32();
}

QScriptValue SslProtocolToScriptValue(QScriptEngine *engine, const QSsl::SslProtocol &item)
{
  return engine->newVariant(item);
}
void SslProtocolFromScriptValue(const QScriptValue &obj, QSsl::SslProtocol &item)
{
  item = (QSsl::SslProtocol)obj.toInt32();
}
#endif

void setupQSslProto(QScriptEngine *engine)
{
  QScriptValue obj = engine->newObject();
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

#if QT_VERSION >= 0x050000
  qScriptRegisterMetaType(engine, AlternativeNameEntryTypeToScriptValue, AlternativeNameEntryTypeFromScriptValue);
  obj.setProperty("EmailEntry", QScriptValue(engine, QSsl::EmailEntry), permanent);
  obj.setProperty("DnsEntry",   QScriptValue(engine, QSsl::DnsEntry), permanent);

  qScriptRegisterMetaType(engine, EncodingFormatToScriptValue, EncodingFormatFromScriptValue);
  obj.setProperty("Pem",  QScriptValue(engine, QSsl::Pem), permanent);
  obj.setProperty("Der",  QScriptValue(engine, QSsl::Der), permanent);

  qScriptRegisterMetaType(engine, KeyAlgorithmToScriptValue, KeyAlgorithmFromScriptValue);
  obj.setProperty("Rsa",    QScriptValue(engine, QSsl::Rsa), permanent);
  obj.setProperty("Dsa",    QScriptValue(engine, QSsl::Dsa), permanent);
  obj.setProperty("Ec",     QScriptValue(engine, QSsl::Ec), permanent);
  obj.setProperty("Opaque", QScriptValue(engine, QSsl::Opaque), permanent);

  qScriptRegisterMetaType(engine, KeyTypeToScriptValue, KeyTypeFromScriptValue);
  obj.setProperty("PrivateKey", QScriptValue(engine, QSsl::PrivateKey), permanent);
  obj.setProperty("PublicKey",  QScriptValue(engine, QSsl::PublicKey), permanent);

  qScriptRegisterMetaType(engine, SslOptionToScriptValue, SslOptionFromScriptValue);
  qScriptRegisterMetaType(engine, SslOptionsToScriptValue, SslOptionsFromScriptValue);
  obj.setProperty("SslOptionDisableEmptyFragments",       QScriptValue(engine, QSsl::SslOptionDisableEmptyFragments), permanent);
  obj.setProperty("SslOptionDisableSessionTickets",       QScriptValue(engine, QSsl::SslOptionDisableSessionTickets), permanent);
  obj.setProperty("SslOptionDisableCompression",          QScriptValue(engine, QSsl::SslOptionDisableCompression), permanent);
  obj.setProperty("SslOptionDisableServerNameIndication", QScriptValue(engine, QSsl::SslOptionDisableServerNameIndication), permanent);
  obj.setProperty("SslOptionDisableLegacyRenegotiation",  QScriptValue(engine, QSsl::SslOptionDisableLegacyRenegotiation), permanent);
  obj.setProperty("SslOptionDisableSessionSharing",       QScriptValue(engine, QSsl::SslOptionDisableSessionSharing), permanent);
  obj.setProperty("SslOptionDisableSessionPersistence",   QScriptValue(engine, QSsl::SslOptionDisableSessionPersistence), permanent);

  qScriptRegisterMetaType(engine, SslProtocolToScriptValue, SslProtocolFromScriptValue);
  obj.setProperty("SslV3",            QScriptValue(engine, QSsl::SslV3), permanent);
  obj.setProperty("SslV2",            QScriptValue(engine, QSsl::SslV2), permanent);
  obj.setProperty("TlsV1_0",          QScriptValue(engine, QSsl::TlsV1_0), permanent);
  obj.setProperty("TlsV1_0OrLater",   QScriptValue(engine, QSsl::TlsV1_0OrLater), permanent);
  obj.setProperty("TlsV1",            QScriptValue(engine, QSsl::TlsV1), permanent);
  obj.setProperty("TlsV1_1",          QScriptValue(engine, QSsl::TlsV1_1), permanent);
  obj.setProperty("TlsV1_1OrLater",   QScriptValue(engine, QSsl::TlsV1_1OrLater), permanent);
  obj.setProperty("TlsV1_2",          QScriptValue(engine, QSsl::TlsV1_2), permanent);
  obj.setProperty("TlsV1_2OrLater",   QScriptValue(engine, QSsl::TlsV1_2OrLater), permanent);
  obj.setProperty("UnknownProtocol",  QScriptValue(engine, QSsl::UnknownProtocol), permanent);
  obj.setProperty("AnyProtocol",      QScriptValue(engine, QSsl::AnyProtocol), permanent);
  obj.setProperty("TlsV1SslV3",       QScriptValue(engine, QSsl::TlsV1SslV3), permanent);
  obj.setProperty("SecureProtocols",  QScriptValue(engine, QSsl::SecureProtocols), permanent);
#endif

  engine->globalObject().setProperty("QSsl", obj, permanent);
}
