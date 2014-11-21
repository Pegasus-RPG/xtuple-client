/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtoolbuttonproto.h"

#include <QToolButton>

QScriptValue QToolButtontoScriptValue(QScriptEngine *engine, QToolButton* const &item)
{
  return engine->newQObject(item);
}

void QToolButtonfromScriptValue(const QScriptValue &obj, QToolButton* &item)
{
  item = qobject_cast<QToolButton*>(obj.toQObject());
}

void setupQToolButtonProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QToolButtontoScriptValue, QToolButtonfromScriptValue);

  QScriptValue proto = engine->newQObject(new QToolButtonProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QToolButton*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQToolButton,
                                                 proto);
  engine->globalObject().setProperty("QToolButton",  constructor);
}

QScriptValue constructQToolButton(QScriptContext * context,
                                  QScriptEngine  *engine)
{
  QToolButton *obj = 0;
  if(context->argumentCount() == 1 && context->argument(0).isQObject())
    obj = new QToolButton(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QToolButton();
  return engine->toScriptValue(obj);
}

QToolButtonProto::QToolButtonProto(QObject *parent)
    : QObject(parent)
{
}

void QToolButtonProto::setMenu(QMenu *menu)
{
  QToolButton *item = qscriptvalue_cast<QToolButton*>(thisObject());
  if (item)
    item->setMenu(menu);
}

QMenu* QToolButtonProto::menu() const
{
  QToolButton *item = qscriptvalue_cast<QToolButton*>(thisObject());
  if (item)
    return item->menu();
  return 0;
}
