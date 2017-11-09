/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qmimetypeproto.h"

void setupQMimeTypeProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QMimeTypeProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QMimeType>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QMimeType*>(), proto);

  QScriptValue ctor = engine->newFunction(constructQMimeType, proto);
  engine->globalObject().setProperty("QMimeType", ctor);
}

QScriptValue constructQMimeType(QScriptContext *context, QScriptEngine *engine)
{
  QMimeType *obj = 0;
  if (context->argumentCount() == 0)
  {
    obj = new QMimeType();
  }
  else if (context->argumentCount() == 1 && qscriptvalue_cast<QMimeType*>(context->argument(0)))
  {
    obj = new QMimeType(*(qscriptvalue_cast<QMimeType*>(context->argument(0))));
  }
  else
  {
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QMimeType constructor");
  }

  return engine->toScriptValue(obj);
}

QMimeTypeProto::QMimeTypeProto(QObject *parent)
  : QObject(parent)
{
}

QMimeTypeProto::~QMimeTypeProto()
{
}

QStringList QMimeTypeProto::aliases() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->aliases();
  return QStringList();
}

QStringList QMimeTypeProto::allAncestors() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->allAncestors();
  return QStringList();
}

QString QMimeTypeProto::comment() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->comment();
  return QString();
}

QString QMimeTypeProto::filterString() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->filterString();
  return QString();
}

QString QMimeTypeProto::genericIconName() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->genericIconName();
  return QString();
}

QStringList QMimeTypeProto::globPatterns() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->globPatterns();
  return QStringList();
}

QString QMimeTypeProto::iconName() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->iconName();
  return QString();
}

bool QMimeTypeProto::inherits(const QString &mimeTypeName) const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->inherits(mimeTypeName);
  return false;
}

bool QMimeTypeProto::isDefault() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->isDefault();
  return false;
}

bool QMimeTypeProto::isValid() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QString QMimeTypeProto::name() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

QStringList QMimeTypeProto::parentMimeTypes() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->parentMimeTypes();
  return QStringList();
}

QString QMimeTypeProto::preferredSuffix() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->preferredSuffix();
  return QString();
}

QStringList QMimeTypeProto::suffixes() const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->suffixes();
  return QStringList();
}

void QMimeTypeProto::swap(QMimeType& other)
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    item->swap(other);
}

bool QMimeTypeProto::operator!=(const QMimeType &other) const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->operator!=(other);
  return false;
}

QMimeType& QMimeTypeProto::operator=(const QMimeType &other)
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->operator=(other);
  return *(new QMimeType());
}

bool QMimeTypeProto::operator==(const QMimeType &other) const
{
  QMimeType *item = qscriptvalue_cast<QMimeType*>(thisObject());
  if (item)
    return item->operator==(other);
  return false;
}
