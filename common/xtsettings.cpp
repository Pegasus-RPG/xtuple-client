/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtsettings.h"

#include <QSettings>

static QSettings settings(QSettings::UserScope, "xTuple.com", "xTuple");
static QSettings oldsettings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");

QVariant xtsettingsValue(const QString & key, const QVariant & defaultValue)
{
  QString key1 = key;
  QString key2 = key;
  if(key1.startsWith("/xTuple/"))
    key2 = key2.replace(0, 8, QString("/OpenMFG/"));
  if(settings.contains(key1))
    return settings.value(key1, defaultValue);
  else if(oldsettings.contains(key2))
  {
    QVariant val = oldsettings.value(key2, defaultValue);
    xtsettingsSetValue(key, val);
    return val;
  }
  return defaultValue;
}

void xtsettingsSetValue(const QString & key, const QVariant & value)
{
  QString key1 = "/xTuple/" + key;
  settings.setValue(key1, value);
}

