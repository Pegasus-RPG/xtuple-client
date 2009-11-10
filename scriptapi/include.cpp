/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "include.h"
#include <xsqlquery.h>

void setupInclude(QScriptEngine *engine)
{
  engine->globalObject().setProperty("include",
                                     engine->newFunction(includeScript));
}

QScriptValue includeScript(QScriptContext *context, QScriptEngine *engine)
{
  int count = 0;
  XSqlQuery scriptq;
  scriptq.prepare("SELECT script_id, script_source AS src"
                  "  FROM script"
                  " WHERE ((script_name=:script_name)"
                  "   AND  (script_enabled))"
                  " ORDER BY script_order;");

  context->setActivationObject(context->parentContext()->activationObject());
  context->setThisObject(context->parentContext()->thisObject());

  for (; count < context->argumentCount(); count++)
  {
    QString scriptname = context->argument(count).toString();
    scriptq.bindValue(":script_name", scriptname);
    scriptq.exec();
    while (scriptq.next())
    {
      QScriptValue result = engine->evaluate(scriptq.value("src").toString(),
                                             scriptname,
                                             1);
      if (engine->hasUncaughtException())
      {
        qWarning() << "uncaught exception in" << scriptname
                   << "(id" << scriptq.value("script_id").toInt()
                   << ") at line"
                   << engine->uncaughtExceptionLineNumber() << ":"
                   << result.toString();
        break;
      }
    }
  }

  return engine->toScriptValue(count);
}
