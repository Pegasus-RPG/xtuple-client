/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "custcluster.h"
#include <QtScript>

void setupCLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("AllCustomers",	           QScriptValue(engine, CLineEdit::AllCustomers),	        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActiveCustomers",	           QScriptValue(engine, CLineEdit::ActiveCustomers), 	        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllProspects",	           QScriptValue(engine, CLineEdit::AllProspects),	        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActiveProspects",	           QScriptValue(engine, CLineEdit::ActiveProspects),	        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllCustomersAndProspects",   QScriptValue(engine, CLineEdit::AllCustomersAndProspects),	QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActiveCustomersAndProspects",QScriptValue(engine, CLineEdit::ActiveCustomersAndProspects),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("CLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
