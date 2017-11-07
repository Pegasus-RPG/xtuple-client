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
#include "xsqlquery.h"
#include "errorReporter.h"

QScriptValue QDateToScriptValue(QScriptEngine *engine, const QDate &in)
{
  XSqlQuery getTZ;
  getTZ.prepare("SELECT EXTRACT(TIMEZONE FROM :date::TIMESTAMP WITH TIME ZONE) AS offset;");
  getTZ.bindValue(":date", in);
  getTZ.exec();
  if (getTZ.first())
    return engine->newDate(QDateTime(in, QTime(), QTimeZone(getTZ.value("offset").toInt())));
  else
  {
    ErrorReporter::error(QtCriticalMsg, 0, "Failed to fetch server time zone",
                                getTZ, __FILE__, __LINE__);
    return engine->newDate(QDateTime(in));
  }
}

void QDateFromScriptValue(const QScriptValue &object, QDate &out)
{
  QDateTime obj = object.toDateTime();
  obj.setTimeSpec(Qt::LocalTime);
  out = obj.date();
}

void setupQDateProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QDateToScriptValue, QDateFromScriptValue);
}
