/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "char.h"

QScriptValue charPointerToScriptValue(QScriptEngine *engine, char* &item)
{
  QVariant v;
  v.setValue(QString(QChar(item)));
  return engine->newVariant(v);
}
void charPointerFromScriptValue(const QScriptValue &obj, char* &item)
{
  QByteArray ba = obj.toString().toLocal8Bit();
  item = ba.data();
}

void setupchar(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, charPointerToScriptValue, charPointerFromScriptValue);
}
