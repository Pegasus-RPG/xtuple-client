/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "tocluster.h"
#include <QtScript>

void setupToLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("cToOpen",      QScriptValue(engine, cToOpen),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cToClosed",    QScriptValue(engine, cToClosed),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cToAtShipping",QScriptValue(engine, cToAtShipping),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ToLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
