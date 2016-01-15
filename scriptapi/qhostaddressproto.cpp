/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qhostaddressproto.h"

void setupQHostAddressProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QHostAddressProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QHostAddress*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QHostAddress>(), proto);

  QScriptValue constructor = engine->newFunction(constructQHostAddress, proto);
  engine->globalObject().setProperty("QHostAddress",  constructor);
}

QScriptValue constructQHostAddress(QScriptContext * context, QScriptEngine  *engine)
{
  QHostAddress *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QHostAddress(context->argument(0).toString());
  else
    obj = new QHostAddress();
  return engine->toScriptValue(obj);
}

QHostAddressProto::QHostAddressProto(QObject *parent)
    : QObject(parent)
{
}
QHostAddressProto::~QHostAddressProto()
{
}

QString QHostAddressProto::toString() const
{
  QHostAddress *item = qscriptvalue_cast<QHostAddress*>(thisObject());
  if (item)
    return item->toString();
  return QString("[QHostAddress(unknown)]");
}
