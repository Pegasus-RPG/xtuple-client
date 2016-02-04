/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "quuidproto.h"

#if QT_VERSION < 0x050000
void setupQUuidProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQUuidProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QUuidtoScriptValue, QUuidfromScriptValue);

  QScriptValue proto = engine->newQObject(new QUuidProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUuid*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QUuid>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQUuid, proto);
  engine->globalObject().setProperty("QUuid", constructor);
}

QScriptValue constructQUuid(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QUuid *obj = 0;
  /* if (context->argumentCount() ...)
  else if (something bad)
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QUuidconstructor");
  else
  */
    obj = new QUuid();
  return engine->toScriptValue(obj);
}

QUuidProto::QUuidProto(QObject *parent) : QObject(parent)
{
}
QUuidProto::~QUuidProto()
{
}

bool QUuidProto::isNull() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QByteArray QUuidProto::toByteArray() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->toByteArray();
  return QByteArray();
}

QByteArray QUuidProto::toRfc4122() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->toRfc4122();
  return QByteArray();
}

QString QUuidProto::toString() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}

QUuid::Variant QUuidProto::variant() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->variant();
  return QUuid::Variant();
}

QUuid::Version QUuidProto::version() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->version();
  return QUuid::Version();
}

#endif
