/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qjsonobjectproto.h"

#include <QJsonObject>

QScriptValue QJsonObjecttoScriptValue(QScriptEngine *engine, QJsonObject::iterator const &iterator)
{
  //return engine->newQObject(item);
  return engine->newVariant(QVariant(&iterator));
}

void QJsonObjectfromScriptValue(const QScriptValue &obj, QJsonObject::iterator &iterator)
{
  //item = qobject_cast<QJsonObject*>(obj.toQObject());
  iterator = qscriptvalue_cast<QJsonObject::iterator>(obj);
}

void setupQJsonObjectProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QJsonObjecttoScriptValue, QJsonObjectfromScriptValue);

  QScriptValue proto = engine->newQObject(new QJsonObjectProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QJsonObject*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQJsonObject,
                                                 proto);
  engine->globalObject().setProperty("QJsonObject",  constructor);
}

QScriptValue constructQJsonObject(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QJsonObject *obj = 0;
  /* TODO QVariant::QJsonObject doesn't exist
   * https://github.com/qtproject/qtbase/blob/dev/src/corelib/kernel/qvariant.h#L125-L191
  if (context->argumentCount() == 1 && context->argument(0).isVariant() &&
        context->argument(0).toVariant().type() == QVariant::QJsonObject)
  */
  if (context->argumentCount() == 1 && context->argument(0).isVariant())
    obj = new QJsonObject(context->argument(0).toVariant().toJsonObject());
  else
    obj = new QJsonObject();
  return engine->toScriptValue(obj);
}

QJsonObjectProto::QJsonObjectProto(QObject *parent)
    : QObject(parent)
{
}

QJsonObject::iterator QJsonObjectProto::begin()
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->begin();
  // TODO: What to return here:
}

QJsonObject::const_iterator QJsonObjectProto::begin() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->begin();
  // TODO: What to return here:
}

QJsonObject::const_iterator QJsonObjectProto::constBegin() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->constBegin();
  // TODO: What to return here:
}

QJsonObject::const_iterator QJsonObjectProto::constEnd() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->constEnd();
  // TODO: What to return here:
}

QJsonObject::const_iterator QJsonObjectProto::constFind(const QString & key) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->constFind(key);
  // TODO: What to return here:
}

bool QJsonObjectProto::contains(const QString & key) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->contains(key);
  return false;
}

int QJsonObjectProto::count() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

bool QJsonObjectProto::empty() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->empty();
  return false;
}

QJsonObject::iterator QJsonObjectProto::end()
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->end();
  // TODO: What to return here:
}

QJsonObject::const_iterator QJsonObjectProto::end() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->end();
  // TODO: What to return here:
}

QJsonObject::iterator QJsonObjectProto::erase(QJsonObject::iterator it)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->erase(it);
  // TODO: What to return here:
}

QJsonObject::iterator QJsonObjectProto::find(const QString & key)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->find(key);
  // TODO: What to return here:
}

QJsonObject::const_iterator QJsonObjectProto::find(const QString & key) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->find(key);
  // TODO: What to return here:
}

QJsonObject::iterator QJsonObjectProto::insert(const QString & key, const QJsonValue & value)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->insert(key, value);
  // TODO: What to return here:
}

bool QJsonObjectProto::isEmpty() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

QStringList QJsonObjectProto::keys() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->keys();
  return QStringList();
}

int QJsonObjectProto::length() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

void QJsonObjectProto::remove(const QString & key)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->remove(key);
}

int QJsonObjectProto::size() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QJsonValue QJsonObjectProto::take(const QString & key)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->take(key);
  return QJsonValue();
}

/*
 * TODO: error: 'class QJsonObject' has no member named 'toVariantHash'
QVariantHash QJsonObjectProto::toVariantHash() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->toVariantHash();
  return QVariantHash();
}
*/

QVariantMap QJsonObjectProto::toVariantMap() const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->toVariantMap();
  return QVariantMap();
}

QJsonValue QJsonObjectProto::value(const QString & key) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->value(key);
  return QJsonValue();
}

bool QJsonObjectProto::operator!=(const QJsonObject & other) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->operator!=(other);
  return false;
}

QJsonObject & QJsonObjectProto::operator=(const QJsonObject & other)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->operator=(other);
  // TODO: What should be returned here?
  //return QJsonObject();
}

bool QJsonObjectProto::operator==(const QJsonObject & other) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->operator==(other);
  return false;
}

QJsonValue QJsonObjectProto::operator[](const QString & key) const
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->operator[](key);
  return QJsonValue();
}

QJsonValueRef QJsonObjectProto::operator[](const QString & key)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->operator[](key);
  // TODO: What should be returned here?
  //return QJsonValueRef();
}

/*
 * TODO: error: 'class QJsonObject' has no member named 'fromVariantHash'
QJsonObject QJsonObjectProto::fromVariantHash(const QVariantHash & hash)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->fromVariantHash(hash);
  // TODO: What should be returned here?
  //return QJsonObject();
}
*/

QJsonObject QJsonObjectProto::fromVariantMap(const QVariantMap & map)
{
  QJsonObject *item = qscriptvalue_cast<QJsonObject*>(thisObject());
  if (item)
    return item->fromVariantMap(map);
  return QJsonObject();
}
