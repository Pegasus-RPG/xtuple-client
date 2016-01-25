/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qbufferproto.h"

void setupQBufferProto(QScriptEngine *engine)
{
  QScriptValue constructor = engine->newFunction(constructQBuffer);
  QScriptValue metaObject = engine->newQMetaObject(&QBuffer::staticMetaObject, constructor);
  engine->globalObject().setProperty("QBuffer", metaObject);
}

QScriptValue constructQBuffer(QScriptContext *context, QScriptEngine *engine)
{
  QObject *parent = context->argument(0).toQObject();
  QBuffer *object = new QBuffer(parent);
  return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}
#endif
