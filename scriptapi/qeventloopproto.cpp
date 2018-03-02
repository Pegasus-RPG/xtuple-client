/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qeventloopproto.h"

#include <QEventLoop>
#include <QString>

QScriptValue ProcessEventsFlagToScriptValue(QScriptEngine *engine, const enum QEventLoop::ProcessEventsFlag &p)
{
  return QScriptValue(engine, (int)p);
}
void ProcessEventsFlagFromScriptValue(const QScriptValue &obj, enum QEventLoop::ProcessEventsFlag &p)
{
  p = (enum QEventLoop::ProcessEventsFlag)obj.toInt32();
}

QScriptValue ProcessEventsFlagsToScriptValue(QScriptEngine *engine, const QEventLoop::ProcessEventsFlags &p)
{
  return QScriptValue(engine, (int)p);
}
void ProcessEventsFlagsFromScriptValue(const QScriptValue &obj, QEventLoop::ProcessEventsFlags &p)
{
  p = (QEventLoop::ProcessEventsFlags)obj.toInt32();
}

static QScriptValue processEvents(QScriptContext *context, QScriptEngine *engine)
{
  QEventLoop *item = qscriptvalue_cast<QEventLoop *>(context->thisObject());
  bool result = false;
  if (item)
  {
    if (context->argumentCount() >= 2 &&
        qscriptvalue_cast<QEventLoop::ProcessEventsFlags>(context->argument(0)) &&
        context->argument(1).isNumber())
      (void)item->processEvents(qscriptvalue_cast<QEventLoop::ProcessEventsFlags>(context->argument(0)),
                                context->argument(1).toInt32());
    else if (context->argumentCount() == 1 &&
             qscriptvalue_cast<QEventLoop::ProcessEventsFlags>(context->argument(0)))
      result = item->processEvents(qscriptvalue_cast<QEventLoop::ProcessEventsFlags>(context->argument(0)));
    else
      result = item->processEvents();

  }
  return engine->toScriptValue(result);
}

QScriptValue constructQEventLoop(QScriptContext *context,
                                 QScriptEngine  *engine)
{
  QEventLoop *obj = 0;

  if (context->argumentCount() >= 1)
    obj = new QEventLoop(qscriptvalue_cast<QObject*>(context->argument(0)));
  else
    obj = new QEventLoop();

  return engine->toScriptValue(obj);
}

void setupQEventLoopProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QEventLoopProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QEventLoop*>(), proto);
  proto.setProperty("processEvents", engine->newFunction(processEvents));

  QScriptValue constructor = engine->newFunction(constructQEventLoop, proto);
  engine->globalObject().setProperty("QEventLoop", constructor);

  qScriptRegisterMetaType(engine, ProcessEventsFlagToScriptValue,  ProcessEventsFlagFromScriptValue);
  qScriptRegisterMetaType(engine, ProcessEventsFlagsToScriptValue, ProcessEventsFlagsFromScriptValue);
  constructor.setProperty("AllEvents",              QScriptValue(engine, QEventLoop::AllEvents),              ENUMPROPFLAGS);
  constructor.setProperty("ExcludeUserInputEvents", QScriptValue(engine, QEventLoop::ExcludeUserInputEvents), ENUMPROPFLAGS);
  constructor.setProperty("ExcludeSocketNotifiers", QScriptValue(engine, QEventLoop::ExcludeSocketNotifiers), ENUMPROPFLAGS);
  constructor.setProperty("WaitForMoreEvents",      QScriptValue(engine, QEventLoop::WaitForMoreEvents),      ENUMPROPFLAGS);
}

QEventLoopProto::QEventLoopProto(QObject *parent)
    : QObject(parent)
{
}

int QEventLoopProto::exec(QEventLoop::ProcessEventsFlags flags)
{
  QEventLoop *item = qscriptvalue_cast<QEventLoop*>(thisObject());
  if (item)
    return item->exec(flags);
  return 0;
}

void QEventLoopProto::exit(int returnCode)
{
  QEventLoop *item = qscriptvalue_cast<QEventLoop*>(thisObject());
  if (item)
    item->exit(returnCode);
}

bool QEventLoopProto::isRunning() const
{
  QEventLoop *item = qscriptvalue_cast<QEventLoop*>(thisObject());
  if (item)
    return item->isRunning();
  return false;
}

void QEventLoopProto::wakeUp()
{
  QEventLoop *item = qscriptvalue_cast<QEventLoop*>(thisObject());
  if (item)
    item->wakeUp();
}

