/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "usernamecluster.h"
#include <QtScript>

void setupUsernameLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("UsersAll",     QScriptValue(engine, UsernameLineEdit::UsersAll),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersActive",  QScriptValue(engine, UsernameLineEdit::UsersActive),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersInactive",QScriptValue(engine, UsernameLineEdit::UsersInactive),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("UsernameLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
