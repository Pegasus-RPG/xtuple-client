/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XWEBSYNCPROTO_H__
#define __XWEBSYNCPROTO_H__

#include <QJsonObject>
#include <QtScript>

#include <xwebsync.h>

Q_DECLARE_METATYPE(XWebSync*)

void setupXWebSyncProto(QScriptEngine *engine);
QScriptValue constructXWebSync(QScriptContext *context, QScriptEngine *engine);

class XWebSyncProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    XWebSyncProto(QObject *parent);

    Q_INVOKABLE QString data() const;
    Q_INVOKABLE QString query() const;
    Q_INVOKABLE QString title() const;
    //Q_INVOKABLE QJsonObject test() const;
};

#endif
