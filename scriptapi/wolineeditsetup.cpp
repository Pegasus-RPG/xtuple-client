/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "wocluster.h"
#include <QtScript>

void setupWoLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("cWoOpen",      QScriptValue(engine, cWoOpen),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cWoExploded",  QScriptValue(engine, cWoExploded), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cWoIssued",    QScriptValue(engine, cWoIssued),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cWoReleased",  QScriptValue(engine, cWoReleased), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cWoClosed",    QScriptValue(engine, cWoClosed),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("WoLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
