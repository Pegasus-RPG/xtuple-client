/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtsettings.h"

#include <QSettings>

QVariant xtsettingsValue(const QString & key, const QVariant & defaultValue)
{
  QSettings settings(QSettings::UserScope, "xTuple.com", "xTuple");
  QString key2 = key;
  if(key.startsWith("/xTuple/"))
    key2 = key2.replace(0, 8, QString("/OpenMFG/"));
  if(settings.contains(key))
    return settings.value(key, defaultValue);
  else
  {
    QSettings oldsettings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
    if(oldsettings.contains(key2))
    {
      QVariant val = oldsettings.value(key2, defaultValue);
      xtsettingsSetValue(key, val);
      return val;
    }
  }
  return defaultValue;
}

void xtsettingsSetValue(const QString & key, const QVariant & value)
{
  QSettings settings(QSettings::UserScope, "xTuple.com", "xTuple");
  settings.setValue(key, value);
}

QScriptValue xtsettingsValueProto(QScriptContext *context, QScriptEngine *engine)
{
  QVariant result;

  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).toVariant().isValid())
    result = xtsettingsValue(context->argument(0).toString(), context->argument(1).toVariant());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find appropriate xtsettingsValue()");

  return engine->newVariant(result);
}

QScriptValue xtsettingsSetValueProto(QScriptContext *context, QScriptEngine *engine)
{
  Q_UNUSED(engine)

  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).toVariant().isValid())
    xtsettingsSetValue(context->argument(0).toString(), context->argument(1).toVariant());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find appropriate xtsettingsSetValue()");

  return QScriptValue();
}

void setupXtSettings(QScriptEngine *engine)
{
  engine->globalObject().setProperty("xtsettingsValue", engine->newFunction(xtsettingsValueProto));
  engine->globalObject().setProperty("xtsettingsSetValue", engine->newFunction(xtsettingsSetValueProto));
}
