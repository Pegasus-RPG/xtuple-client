/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdateproto.h"

QScriptValue QDateToScriptValue(QScriptEngine *engine, const QDate &in)
{
  return engine->newVariant(in);
}

void QDateFromScriptValue(const QScriptValue &object, QDate &out)
{
  out = object.toVariant().toDate();
}

void setupQDateProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QDateToScriptValue, QDateFromScriptValue);
}
