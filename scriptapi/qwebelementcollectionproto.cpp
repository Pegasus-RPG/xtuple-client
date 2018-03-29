/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qwebelementcollectionproto.h"

#if QT_VERSION < 0x050000
void setupQWebElementCollectionProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQWebElementCollectionProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QWebElementCollectionProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebElementCollection*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QWebElementCollection>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQWebElementCollection, proto);
  engine->globalObject().setProperty("QWebElementCollection", constructor);
}

QScriptValue constructQWebElementCollection(QScriptContext *context, QScriptEngine  *engine)
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *obj = 0;
  if (context->argumentCount() == 2) {
    QWebElement contextElement = qscriptvalue_cast<QWebElement>(context->argument(0));
    obj = new QWebElementCollection(contextElement, context->argument(1).toString());
  } else if (context->argumentCount() == 1) {
    QWebElementCollection other = qscriptvalue_cast<QWebElementCollection>(context->argument(0));
    obj = new QWebElementCollection(other);
  } else {
    obj = new QWebElementCollection();
  }

  return engine->toScriptValue(obj);
}

QWebElementCollectionProto::QWebElementCollectionProto(QObject *parent) : QObject(parent)
{
}
QWebElementCollectionProto::~QWebElementCollectionProto()
{
}

void QWebElementCollectionProto::append(const QWebElementCollection & other)
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    item->append(other);
}

QWebElement QWebElementCollectionProto::at(int i) const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->at(i);
  return QWebElement();
}

#ifdef Use_QWebElementCollectionIterators
QWebElementCollection::const_iterator QWebElementCollectionProto::begin() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->begin();
  return QWebElementCollection::const_iterator();
}

QWebElementCollection::iterator QWebElementCollectionProto::begin()
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->begin();
  return QWebElementCollection::iterator();
}

QWebElementCollection::const_iterator QWebElementCollectionProto::constBegin() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->constBegin();
  return QWebElementCollection::const_iterator();
}

QWebElementCollection::const_iterator QWebElementCollectionProto::constEnd() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->constEnd();
  return QWebElementCollection::const_iterator();
}
#endif

int QWebElementCollectionProto::count() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

#ifdef Use_QWebElementCollectionIterators
QWebElementCollection::const_iterator QWebElementCollectionProto::end() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->end();
  return QWebElementCollection::const_iterator();
}

QWebElementCollection::iterator QWebElementCollectionProto::end()
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->end();
  return QWebElementCollection::iterator();
}
#endif

QWebElement QWebElementCollectionProto::first() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->first();
  return QWebElement();
}

QWebElement QWebElementCollectionProto::last() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->last();
  return QWebElement();
}

QList<QWebElement> QWebElementCollectionProto::toList() const
{
  scriptDeprecated("QWebElementCollection will not be available in future versions");
  QWebElementCollection *item = qscriptvalue_cast<QWebElementCollection*>(thisObject());
  if (item)
    return item->toList();
  return QList<QWebElement>();
}

#endif
