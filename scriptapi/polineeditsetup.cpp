/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "pocluster.h"
#include <QtScript>

void setupPoLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("cPOUnposted", QScriptValue(engine, cPOUnposted),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPOOpen",     QScriptValue(engine, cPOOpen),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPOClosed",   QScriptValue(engine, cPOClosed),  QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("cPOItem",     QScriptValue(engine, cPOItem),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPOItemsrc",  QScriptValue(engine, cPOItemsrc), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("PoLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
