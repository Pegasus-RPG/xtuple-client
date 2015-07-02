/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QOBJECTPROTO_H__
#define __QOBJECTPROTO_H__

#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QObject*)

void setupQObjectProto(QScriptEngine *engine);
QScriptValue constructQObject(QScriptContext *context, QScriptEngine *engine);

class QObjectProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QObjectProto(QObject *parent = 0);
};

#endif
