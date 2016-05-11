/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdnslookupproto.h"

#if QT_VERSION < 0x050000

QScriptValue ErrorToScriptValue(QScriptEngine *engine, const QDnsLookup::Error &item)
{
  return engine->newVariant(item);
}
void ErrorFromScriptValue(const QScriptValue &obj, QDnsLookup::Error &item)
{
  item = (QDnsLookup::Error)obj.toInt32();
}

QScriptValue TypeToScriptValue(QScriptEngine *engine, const QDnsLookup::Type &item)
{
  return engine->newVariant(item);
}
void TypeFromScriptValue(const QScriptValue &obj, QDnsLookup::Type &item)
{
  item = (QDnsLookup::Type)obj.toInt32();
}

void setupQDnsLookupProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QDnsLookupProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDnsLookup*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDnsLookup>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDnsLookup, proto);
  engine->globalObject().setProperty("QDnsLookup",  constructor);

  qScriptRegisterMetaType(engine, ErrorToScriptValue, ErrorFromScriptValue);
  constructor.setProperty("NoError", QScriptValue(engine, QDnsLookup::NoError), permanent);
  constructor.setProperty("ResolverError", QScriptValue(engine, QDnsLookup::ResolverError), permanent);
  constructor.setProperty("OperationCancelledError", QScriptValue(engine, QDnsLookup::OperationCancelledError), permanent);
  constructor.setProperty("InvalidRequestError", QScriptValue(engine, QDnsLookup::InvalidRequestError), permanent);
  constructor.setProperty("InvalidReplyError", QScriptValue(engine, QDnsLookup::InvalidReplyError), permanent);
  constructor.setProperty("ServerFailureError", QScriptValue(engine, QDnsLookup::ServerFailureError), permanent);
  constructor.setProperty("ServerRefusedError", QScriptValue(engine, QDnsLookup::ServerRefusedError), permanent);
  constructor.setProperty("NotFoundError", QScriptValue(engine, QDnsLookup::NotFoundError), permanent);

  qScriptRegisterMetaType(engine, TypeToScriptValue, TypeFromScriptValue);
  constructor.setProperty("A", QScriptValue(engine, QDnsLookup::A), permanent);
  constructor.setProperty("AAAA", QScriptValue(engine, QDnsLookup::AAAA), permanent);
  constructor.setProperty("ANY", QScriptValue(engine, QDnsLookup::ANY), permanent);
  constructor.setProperty("CNAME", QScriptValue(engine, QDnsLookup::CNAME), permanent);
  constructor.setProperty("MX", QScriptValue(engine, QDnsLookup::MX), permanent);
  constructor.setProperty("NS", QScriptValue(engine, QDnsLookup::NS), permanent);
  constructor.setProperty("PTR", QScriptValue(engine, QDnsLookup::PTR), permanent);
  constructor.setProperty("SRV", QScriptValue(engine, QDnsLookup::SRV), permanent);
  constructor.setProperty("TXT", QScriptValue(engine, QDnsLookup::TXT), permanent);
}

QScriptValue constructQDnsLookup(QScriptContext *context, QScriptEngine *engine)
{
  QDnsLookup *obj = 0;
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString()) {

    obj = new QDnsLookup(context->argument(0).toString(),
                         context->argument(1)).toString());
  } else if (context->argumentCount() == 3 &&
             context->argument(0).isString() &&
             context->argument(1).isString() &&
             context->argument(2).isObject()) {

    obj = new QDnsLookup(context->argument(0).toString(),
                         context->argument(1).toString(),
                         qscriptvalue_cast<QHostAddress>(context->argument(2)));
  } else {
    obj = new QDnsLookup();
  }

  return engine->toScriptValue(obj);
}

QDnsLookupProto::QDnsLookupProto(QObject *parent) : QObject(parent)
{
}
QDnsLookupProto::~QDnsLookupProto()
{
}

QList<QDnsDomainNameRecord> QDnsLookupProto::canonicalNameRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->canonicalNameRecords();
  return QList<QDnsDomainNameRecord>();
}

QDnsLookup::Error QDnsLookupProto::error() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->error();
  return QDnsLookup::Error();
}

QString QDnsLookupProto::errorString() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

QList<QDnsHostAddressRecord> QDnsLookupProto::hostAddressRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->hostAddressRecords();
  return QList<QDnsHostAddressRecord>();
}

bool QDnsLookupProto::isFinished() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->isFinished();
  return false;
}

QList<QDnsMailExchangeRecord> QDnsLookupProto::mailExchangeRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->mailExchangeRecords();
  return QList<QDnsMailExchangeRecord>();
}

QString QDnsLookupProto::name() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QList<QDnsDomainNameRecord> QDnsLookupProto::nameServerRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->nameServerRecords();
  return QList<QDnsDomainNameRecord>();
}

QHostAddress QDnsLookupProto::nameserver() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->nameserver();
  return QHostAddress();
}

QList<QDnsDomainNameRecord> QDnsLookupProto::pointerRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->pointerRecords();
  return QList<QDnsDomainNameRecord>();
}

QList<QDnsServiceRecord> QDnsLookupProto::serviceRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->serviceRecords();
  return QList<QDnsServiceRecord>();
}

void QDnsLookupProto::setName(const QString &name)
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    item->setName(name);
}

void QDnsLookupProto::setNameserver(const QHostAddress &nameserver)
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    item->setNameserver(nameserver);
}

void QDnsLookupProto::setType(QDnsLookup::Type type)
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    item->setType(type);
}

QList<QDnsTextRecord> QDnsLookupProto::textRecords() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->textRecords();
  return QList<QDnsTextRecord>();
}

QDnsLookup::Type QDnsLookupProto::type() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->type();
  return QDnsLookup::Type();
}

QString QDnsLookupProto::toString() const
{
  *item = qscriptvalue_cast<*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}

#endif
