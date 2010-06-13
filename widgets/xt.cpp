/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xt.h"

void setupXt(QScriptEngine *engine)
{
  QScriptValue glob = engine->newObject();

  glob.setProperty("RawRole",         QScriptValue(engine, Xt::RawRole),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("ScaleRole",       QScriptValue(engine, Xt::ScaleRole),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("IdRole",          QScriptValue(engine, Xt::IdRole),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("RunningSetRole",  QScriptValue(engine, Xt::RunningSetRole),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("RunningInitRole", QScriptValue(engine, Xt::RunningInitRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("TotalSetRole",    QScriptValue(engine, Xt::TotalSetRole),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("TotalInitRole",   QScriptValue(engine, Xt::TotalInitRole),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("IndentRole",      QScriptValue(engine, Xt::IndentRole),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  glob.setProperty("DeletedRole",     QScriptValue(engine, Xt::DeletedRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

