/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qnetworkrequestproto.h"

#include <QByteArray>
#include <QList>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>

#define DEBUG false

QScriptValue NetworkRequestAttributeToScriptValue(QScriptEngine *engine, const QNetworkRequest::Attribute &item)
{
  return engine->newVariant(item);
}
void NetworkRequestAttributeFromScriptValue(const QScriptValue &obj, QNetworkRequest::Attribute &item)
{
  item = (QNetworkRequest::Attribute)obj.toInt32();
}

QScriptValue NetworkRequestCacheLoadControlToScriptValue(QScriptEngine *engine, const QNetworkRequest::CacheLoadControl &item)
{
  return engine->newVariant(item);
}
void NetworkRequestCacheLoadControlFromScriptValue(const QScriptValue &obj, QNetworkRequest::CacheLoadControl &item)
{
  item = (QNetworkRequest::CacheLoadControl)obj.toInt32();
}

QScriptValue NetworkRequestKnownHeadersToScriptValue(QScriptEngine *engine, const QNetworkRequest::KnownHeaders &item)
{
  return engine->newVariant(item);
}
void NetworkRequestKnownHeadersFromScriptValue(const QScriptValue &obj, QNetworkRequest::KnownHeaders &item)
{
  item = (QNetworkRequest::KnownHeaders)obj.toInt32();
}

#if QT_VERSION >= 0x050900
QScriptValue RedirectPolicyToScriptValue(QScriptEngine *engine, const QNetworkRequest::RedirectPolicy &item)
{
  return engine->newVariant(item);
}
void RedirectPolicyFromScriptValue(const QScriptValue &obj, QNetworkRequest::RedirectPolicy &item)
{
  item = (QNetworkRequest::RedirectPolicy)obj.toInt32();
}
#endif

QScriptValue NetworkRequestLoadControlToScriptValue(QScriptEngine *engine, const QNetworkRequest::LoadControl &item)
{
  return engine->newVariant(item);
}
void NetworkRequestLoadControlFromScriptValue(const QScriptValue &obj, QNetworkRequest::LoadControl &item)
{
  item = (QNetworkRequest::LoadControl)obj.toInt32();
}

QScriptValue NetworkRequestPriorityToScriptValue(QScriptEngine *engine, const QNetworkRequest::Priority &item)
{
  return engine->newVariant(item);
}
void NetworkRequestPriorityFromScriptValue(const QScriptValue &obj, QNetworkRequest::Priority &item)
{
  item = (QNetworkRequest::Priority)obj.toInt32();
}

void setupQNetworkRequestProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQNetworkRequestProto entered");

  QScriptValue::PropertyFlags ENUMPROPFLAGS = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue netreqproto = engine->newQObject(new QNetworkRequestProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), netreqproto);
  QScriptValue constructor = engine->newFunction(constructQNetworkRequest,  netreqproto);
  engine->globalObject().setProperty("QNetworkRequest", constructor);

  qScriptRegisterMetaType(engine, NetworkRequestAttributeToScriptValue, NetworkRequestAttributeFromScriptValue);
  constructor.setProperty("HttpStatusCodeAttribute", QScriptValue(engine, QNetworkRequest::HttpStatusCodeAttribute), ENUMPROPFLAGS);
  constructor.setProperty("HttpReasonPhraseAttribute", QScriptValue(engine, QNetworkRequest::HttpReasonPhraseAttribute), ENUMPROPFLAGS);
  constructor.setProperty("RedirectionTargetAttribute", QScriptValue(engine, QNetworkRequest::RedirectionTargetAttribute), ENUMPROPFLAGS);
  constructor.setProperty("ConnectionEncryptedAttribute", QScriptValue(engine, QNetworkRequest::ConnectionEncryptedAttribute), ENUMPROPFLAGS);
  constructor.setProperty("CacheLoadControlAttribute", QScriptValue(engine, QNetworkRequest::CacheLoadControlAttribute), ENUMPROPFLAGS);
  constructor.setProperty("CacheSaveControlAttribute", QScriptValue(engine, QNetworkRequest::CacheSaveControlAttribute), ENUMPROPFLAGS);
  constructor.setProperty("SourceIsFromCacheAttribute", QScriptValue(engine, QNetworkRequest::SourceIsFromCacheAttribute), ENUMPROPFLAGS);
  constructor.setProperty("DoNotBufferUploadDataAttribute", QScriptValue(engine, QNetworkRequest::DoNotBufferUploadDataAttribute), ENUMPROPFLAGS);
  constructor.setProperty("HttpPipeliningAllowedAttribute", QScriptValue(engine, QNetworkRequest::HttpPipeliningAllowedAttribute), ENUMPROPFLAGS);
  constructor.setProperty("HttpPipeliningWasUsedAttribute", QScriptValue(engine, QNetworkRequest::HttpPipeliningWasUsedAttribute), ENUMPROPFLAGS);
  constructor.setProperty("CustomVerbAttribute", QScriptValue(engine, QNetworkRequest::CustomVerbAttribute), ENUMPROPFLAGS);
  constructor.setProperty("CookieLoadControlAttribute", QScriptValue(engine, QNetworkRequest::CookieLoadControlAttribute), ENUMPROPFLAGS);
  constructor.setProperty("CookieSaveControlAttribute", QScriptValue(engine, QNetworkRequest::CookieSaveControlAttribute), ENUMPROPFLAGS);
  constructor.setProperty("AuthenticationReuseAttribute", QScriptValue(engine, QNetworkRequest::AuthenticationReuseAttribute), ENUMPROPFLAGS);
  constructor.setProperty("BackgroundRequestAttribute", QScriptValue(engine, QNetworkRequest::BackgroundRequestAttribute), ENUMPROPFLAGS);
  constructor.setProperty("SpdyAllowedAttribute", QScriptValue(engine, QNetworkRequest::SpdyAllowedAttribute), ENUMPROPFLAGS);
  constructor.setProperty("SpdyWasUsedAttribute", QScriptValue(engine, QNetworkRequest::SpdyWasUsedAttribute), ENUMPROPFLAGS);
#if QT_VERSION >= 0x050900
  constructor.setProperty("HTTP2AllowedAttribute", QScriptValue(engine, QNetworkRequest::HTTP2AllowedAttribute), ENUMPROPFLAGS);
  constructor.setProperty("HTTP2WasUsedAttribute", QScriptValue(engine, QNetworkRequest::HTTP2WasUsedAttribute), ENUMPROPFLAGS);
#endif

  constructor.setProperty("EmitAllUploadProgressSignalsAttribute", QScriptValue(engine, QNetworkRequest::EmitAllUploadProgressSignalsAttribute), ENUMPROPFLAGS);

#if QT_VERSION >= 0x050600
  constructor.setProperty("FollowRedirectsAttribute", QScriptValue(engine, QNetworkRequest::FollowRedirectsAttribute), ENUMPROPFLAGS);
#endif

#if QT_VERSION >= 0x050900
  constructor.setProperty("OriginalContentLengthAttribute", QScriptValue(engine, QNetworkRequest::OriginalContentLengthAttribute), ENUMPROPFLAGS);
  constructor.setProperty("RedirectPolicyAttribute", QScriptValue(engine, QNetworkRequest::RedirectPolicyAttribute), ENUMPROPFLAGS);
#endif

  constructor.setProperty("User", QScriptValue(engine, QNetworkRequest::User), ENUMPROPFLAGS);
  constructor.setProperty("UserMax", QScriptValue(engine, QNetworkRequest::UserMax), ENUMPROPFLAGS);

  qScriptRegisterMetaType(engine, NetworkRequestCacheLoadControlToScriptValue, NetworkRequestCacheLoadControlFromScriptValue);
  constructor.setProperty("AlwaysNetwork", QScriptValue(engine, QNetworkRequest::AlwaysNetwork), ENUMPROPFLAGS);
  constructor.setProperty("PreferNetwork", QScriptValue(engine, QNetworkRequest::PreferNetwork), ENUMPROPFLAGS);
  constructor.setProperty("PreferCache", QScriptValue(engine, QNetworkRequest::PreferCache), ENUMPROPFLAGS);
  constructor.setProperty("AlwaysCache", QScriptValue(engine, QNetworkRequest::AlwaysCache), ENUMPROPFLAGS);

  qScriptRegisterMetaType(engine, NetworkRequestKnownHeadersToScriptValue, NetworkRequestKnownHeadersFromScriptValue);
  constructor.setProperty("ContentDispositionHeader", QScriptValue(engine, QNetworkRequest::ContentDispositionHeader), ENUMPROPFLAGS);
  constructor.setProperty("ContentTypeHeader", QScriptValue(engine, QNetworkRequest::ContentTypeHeader), ENUMPROPFLAGS);
  constructor.setProperty("ContentLengthHeader", QScriptValue(engine, QNetworkRequest::ContentLengthHeader), ENUMPROPFLAGS);
  constructor.setProperty("LocationHeader", QScriptValue(engine, QNetworkRequest::LocationHeader), ENUMPROPFLAGS);
  constructor.setProperty("LastModifiedHeader", QScriptValue(engine, QNetworkRequest::LastModifiedHeader), ENUMPROPFLAGS);
  constructor.setProperty("CookieHeader", QScriptValue(engine, QNetworkRequest::CookieHeader), ENUMPROPFLAGS);
  constructor.setProperty("SetCookieHeader", QScriptValue(engine, QNetworkRequest::SetCookieHeader), ENUMPROPFLAGS);
  constructor.setProperty("UserAgentHeader", QScriptValue(engine, QNetworkRequest::UserAgentHeader), ENUMPROPFLAGS);
  constructor.setProperty("ServerHeader", QScriptValue(engine, QNetworkRequest::ServerHeader), ENUMPROPFLAGS);

  qScriptRegisterMetaType(engine, NetworkRequestLoadControlToScriptValue, NetworkRequestLoadControlFromScriptValue);
  constructor.setProperty("Automatic", QScriptValue(engine, QNetworkRequest::Automatic), ENUMPROPFLAGS);
  constructor.setProperty("Manual", QScriptValue(engine, QNetworkRequest::Manual), ENUMPROPFLAGS);

  qScriptRegisterMetaType(engine, NetworkRequestPriorityToScriptValue, NetworkRequestPriorityFromScriptValue);
  constructor.setProperty("HighPriority", QScriptValue(engine, QNetworkRequest::HighPriority), ENUMPROPFLAGS);
  constructor.setProperty("NormalPriority", QScriptValue(engine, QNetworkRequest::NormalPriority), ENUMPROPFLAGS);
  constructor.setProperty("LowPriority", QScriptValue(engine, QNetworkRequest::LowPriority), ENUMPROPFLAGS);

#if QT_VERSION >= 0x050900
  qScriptRegisterMetaType(engine, RedirectPolicyToScriptValue, RedirectPolicyFromScriptValue);
  constructor.setProperty("ManualRedirectPolicy", QScriptValue(engine, QNetworkRequest::ManualRedirectPolicy), ENUMPROPFLAGS);
  constructor.setProperty("NoLessSafeRedirectPolicy", QScriptValue(engine, QNetworkRequest::NoLessSafeRedirectPolicy), ENUMPROPFLAGS);
  constructor.setProperty("SameOriginRedirectPolicy", QScriptValue(engine, QNetworkRequest::SameOriginRedirectPolicy), ENUMPROPFLAGS);
  constructor.setProperty("UserVerifiedRedirectPolicy", QScriptValue(engine, QNetworkRequest::UserVerifiedRedirectPolicy), ENUMPROPFLAGS);
#endif

}

QScriptValue constructQNetworkRequest(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  if (DEBUG) qDebug("constructQNetworkRequest called");
  QNetworkRequest *req = 0;

  if (context->argumentCount() > 0)
    context->throwError(QScriptContext::UnknownError,
                        "QNetworkRequest() constructors with "
                        "arguments are not supported");
  else
    req = new QNetworkRequest();

  return engine->toScriptValue(req);
}

QNetworkRequestProto::QNetworkRequestProto(QObject *parent)
  : QObject(parent)
{
}

QVariant QNetworkRequestProto::attribute(QNetworkRequest::Attribute code, const QVariant &defaultValue) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->attribute(code, defaultValue);
  return QVariant();
}

bool QNetworkRequestProto::hasRawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->hasRawHeader(headerName);
  return false;
}

QVariant QNetworkRequestProto::header(QNetworkRequest::KnownHeaders header) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->header(header);
  return QVariant();
}

QByteArray QNetworkRequestProto::rawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->rawHeader(headerName);
  return QByteArray();
}

QList<QByteArray> QNetworkRequestProto::rawHeaderList() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->rawHeaderList();
  return QList<QByteArray>();
}

void QNetworkRequestProto::setAttribute(QNetworkRequest::Attribute code, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setAttribute(code, value);
}

void QNetworkRequestProto::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setHeader(header, value);
}

void QNetworkRequestProto::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setRawHeader(headerName, headerValue);
}

#ifndef QT_NO_OPENSSL
void QNetworkRequestProto::setSslConfiguration(const QSslConfiguration &config)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setSslConfiguration(config);
}

QSslConfiguration QNetworkRequestProto::sslConfiguration() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}
#endif

void QNetworkRequestProto::setUrl(const QUrl &url)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setUrl(url);
}

QUrl QNetworkRequestProto::url() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

QString QNetworkRequestProto::toString() const
{
  return QString("[QNetworkRequest(url=%1)]")
            .arg(url().toString(QUrl::RemovePassword));
}
