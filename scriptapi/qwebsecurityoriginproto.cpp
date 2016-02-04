/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsecurityoriginproto.h"

#if QT_VERSION < 0x050000
void setupQWebSecurityOriginProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
// TODO: enums

// TODO: statics

// TODO: constuct args.

void setupQWebSecurityOriginProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QWebSecurityOriginProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebSecurityOrigin*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QWebSecurityOrigin>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQWebSecurityOrigin, proto);
  engine->globalObject().setProperty("QWebSecurityOrigin", constructor);
}

QScriptValue constructQWebSecurityOrigin(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QWebSecurityOrigin *obj = 0;
  /* if (context->argumentCount() ...)
  else if (something bad)
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QWebSecurityOriginconstructor");
  else
  */
    obj = new QWebSecurityOrigin();
  return engine->toScriptValue(obj);
}

QWebSecurityOriginProto::QWebSecurityOriginProto(QObject *parent) : QObject(parent)
{
}
QWebSecurityOriginProto::~QWebSecurityOriginProto()
{
}

void QWebSecurityOriginProto::addAccessWhitelistEntry(const QString & scheme, const QString & host, QWebSecurityOrigin::SubdomainSetting subdomainSetting)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->addAccessWhitelistEntry(scheme, host, subdomainSetting);
}

qint64 QWebSecurityOriginProto::databaseQuota() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->databaseQuota();
  return 0;
}

qint64 QWebSecurityOriginProto::databaseUsage() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->databaseUsage();
  return 0;
}

QList<QWebDatabase> QWebSecurityOriginProto::databases() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->databases();
  return QList<QWebDatabase>();
}

QString QWebSecurityOriginProto::host() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->host();
  return QString();
}

int QWebSecurityOriginProto::port() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->port();
  return 0;
}

void QWebSecurityOriginProto::removeAccessWhitelistEntry(const QString & scheme, const QString & host, QWebSecurityOrigin::SubdomainSetting subdomainSetting)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->removeAccessWhitelistEntry(scheme, host, subdomainSetting);
}

QString QWebSecurityOriginProto::scheme() const
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    return item->scheme();
  return QString();
}

void QWebSecurityOriginProto::setApplicationCacheQuota(qint64 quota)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->setApplicationCacheQuota(quota);
}

void QWebSecurityOriginProto::setDatabaseQuota(qint64 quota)
{
  QWebSecurityOrigin *item = qscriptvalue_cast<QWebSecurityOrigin*>(thisObject());
  if (item)
    item->setDatabaseQuota(quota);
}

#endif
