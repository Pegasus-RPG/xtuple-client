/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qhostinfoproto.h"

#define DEBUG false

static QScriptValue qhostinfo_abortHostLookup(QScriptContext *context, QScriptEngine * /*engine*/)
{
  if (context->argumentCount() == 1 && context->argument(0).isNumber()) {
      QHostInfo::abortHostLookup(context->argument(1).toInt32());
  }
  return QScriptValue();
}


static QScriptValue qhostinfo_localDomainName(QScriptContext * /*context*/, QScriptEngine * /*engine*/)
{
  return QHostInfo::localDomainName();
}


static QScriptValue qhostinfo_localHostName(QScriptContext * /*context*/, QScriptEngine * /*engine*/)
{
  return QHostInfo::localHostName();
}


QScriptValue HostInfoErrortoScriptValue(QScriptEngine *engine, const enum QHostInfo::HostInfoError &p)
{
  return QScriptValue(engine, (int)p);
}

void HostInfoErrorfromScriptValue(const QScriptValue &obj, enum QHostInfo::HostInfoError &p)
{
  p = (enum QHostInfo::HostInfoError)obj.toInt32();
}

void setupQHostInfoProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QHostInfoProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QHostInfo*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQHostInfo, proto);

  qScriptRegisterMetaType(engine, HostInfoErrortoScriptValue, HostInfoErrorfromScriptValue);
  constructor.setProperty("NoError",      QScriptValue(engine, QHostInfo::NoError),      ENUMPROPFLAGS);
  constructor.setProperty("HostNotFound", QScriptValue(engine, QHostInfo::HostNotFound), ENUMPROPFLAGS);
  constructor.setProperty("UnknownError", QScriptValue(engine, QHostInfo::UnknownError), ENUMPROPFLAGS);

  constructor.setProperty("abortHostLookup", engine->newFunction(qhostinfo_abortHostLookup), STATICPROPFLAGS);
  constructor.setProperty("localDomainName", engine->newFunction(qhostinfo_localDomainName), STATICPROPFLAGS);
  constructor.setProperty("localHostName",   engine->newFunction(qhostinfo_localHostName),   STATICPROPFLAGS);

  engine->globalObject().setProperty("QHostInfo",  constructor);
}

QScriptValue constructQHostInfo(QScriptContext *context, QScriptEngine  *engine)
{
  QHostInfo *obj = 0;

  if (context->argumentCount() == 0)
  {
      if (DEBUG) qDebug("qhostinfo(0 args)");
      obj = new QHostInfo();
  }
  else if (context->argumentCount() == 1 && context->argument(0).isNumber())
  {
      if (DEBUG) qDebug("qhostinfo(1 arg, int)");
      obj = new QHostInfo(context->argument(0).toInt32());
  }
  else
  {
    if (DEBUG) qDebug("qhostinfo unknown");
    context->throwError(QScriptContext::UnknownError, "Unknown Constructor for QHostInfo");
  }

  return engine->toScriptValue(obj);
}

QHostInfoProto::QHostInfoProto(QObject *parent)
    : QObject(parent)
{
}

QList<QHostAddress> QHostInfoProto::addresses() const
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    return item->addresses();
  return QList<QHostAddress>();
}

QHostInfo::HostInfoError QHostInfoProto::error() const
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    return item->error();
  return QHostInfo::NoError;
}

QString QHostInfoProto::errorString() const
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

QString QHostInfoProto::hostName() const
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    return item->hostName();
  return QString();
}

int QHostInfoProto::lookupId() const
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    return item->lookupId();
  return 0;
}

void QHostInfoProto::setAddresses(const QList<QHostAddress> & addresses)
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    item->setAddresses(addresses);
}

void QHostInfoProto::setError(QHostInfo::HostInfoError error)
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    item->setError(error);
}

void QHostInfoProto::setErrorString(const QString & str)
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    item->setErrorString(str);
}

void QHostInfoProto::setHostName(const QString & hostName)
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    item->setHostName(hostName);
}

void QHostInfoProto::setLookupId(int id)
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    item->setLookupId(id);
}

QString QHostInfoProto::toString() const
{
  QHostInfo *item = qscriptvalue_cast<QHostInfo*>(thisObject());
  if (item)
    return QString("QHostInfo()");
  return QString("QHostInfo(unknown)");
}
