/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "xwebsyncproto.h"

#include <QObject>

QScriptValue XWebSynctoScriptValue(QScriptEngine *engine, QJsonObject const &item)
{
  //return engine->newQObject(item);
  return engine->newVariant(QVariant(&item));
}

void XWebSyncfromScriptValue(const QScriptValue &obj, QJsonObject &item)
{
  //item = qobject_cast<QJsonValue*>(obj.toQObject());
  item = QVariant(obj.toVariant()).toJsonObject();
}

void setupXWebSyncProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, XWebSynctoScriptValue, XWebSyncfromScriptValue);

  QScriptValue proto = engine->newQObject(new XWebSyncProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XWebSync*>(), proto);

  QScriptValue constructor = engine->newFunction(constructXWebSync,
                                                 proto);
  engine->globalObject().setProperty("XWebSync",  constructor);
}

QScriptValue constructXWebSync(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  XWebSync *obj = 0;
  if (context->argumentCount() == 1)
  {
    obj = new XWebSync(qobject_cast<QObject*>(context->argument(0).toQObject()));
  }
  else
    obj = new XWebSync();
  return engine->toScriptValue(obj);
}

XWebSyncProto::XWebSyncProto(QObject *parent)
    : QObject(parent)
{
}

QString XWebSyncProto::data() const
{
  XWebSync *item = qscriptvalue_cast<XWebSync*>(thisObject());
  if (item)
    return item->data();
  return QString();
}

QString XWebSyncProto::query() const
{
  XWebSync *item = qscriptvalue_cast<XWebSync*>(thisObject());
  if (item)
    return item->query();
  return QString();
}

QString XWebSyncProto::title() const
{
  XWebSync *item = qscriptvalue_cast<XWebSync*>(thisObject());
  if (item)
    return item->title();
  return QString();
}

/*
QJsonObject XWebSyncProto::test() const
{
  XWebSync *item = qscriptvalue_cast<XWebSync*>(thisObject());
  if (item)
    return item->test();
  return QJsonObject();
}
*/