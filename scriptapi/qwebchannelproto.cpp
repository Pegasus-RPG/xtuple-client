/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebchannelproto.h"

QScriptValue QWebChanneltoScriptValue(QScriptEngine *engine, QWebChannel* const &item)
{ return engine->newQObject(item); }

void QWebChannelfromScriptValue(const QScriptValue &obj, QWebChannel* &item)
{
  item = qobject_cast<QWebChannel*>(obj.toQObject());
}

void setupQWebChannelProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QWebChanneltoScriptValue, QWebChannelfromScriptValue);

  QScriptValue proto = engine->newQObject(new QWebChannelProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebChannel*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QWebChannel>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQWebChannel,
                                                 proto);
  engine->globalObject().setProperty("QWebChannel",  constructor);
}

QScriptValue constructQWebChannel(QScriptContext *context, QScriptEngine *engine)
{
  QWebChannel *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QWebChannel(context->argument(0).toQObject());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QWebChannel constructor");
  return engine->toScriptValue(obj);
}

QWebChannelProto::QWebChannelProto(QObject *parent)
    : QObject(parent)
{
}

QWebChannelProto::~QWebChannelProto()
{
}

bool QWebChannelProto::blockUpdates() const
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    return item->blockUpdates();
  return false;
}

void QWebChannelProto::deregisterObject(QObject * object)
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    item->deregisterObject(object);
}

void QWebChannelProto::registerObject(const QString & id, QObject * object)
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    item->registerObject(id, object);
}

void QWebChannelProto::registerObjects(const QHash<QString, QObject *> & objects)
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    item->registerObjects(objects);
}

QHash<QString, QObject *> QWebChannelProto::registeredObjects() const
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    return item->registeredObjects();
  return QHash<QString, QObject *>();
}

void QWebChannelProto::setBlockUpdates(bool block)
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    item->setBlockUpdates(block);
}

void QWebChannelProto::connectTo(QWebChannelAbstractTransport * transport)
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    return item->connectTo(transport);
}

void QWebChannelProto::disconnectFrom(QWebChannelAbstractTransport * transport)
{
  QWebChannel *item = qscriptvalue_cast<QWebChannel*>(thisObject());
  if (item)
    return item->disconnectFrom(transport);
}
