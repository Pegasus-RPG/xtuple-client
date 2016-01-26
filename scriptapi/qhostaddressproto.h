/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QHOSTADDRESSPROTO_H__
#define __QHOSTADDRESSPROTO_H__

#include <QScriptEngine>
#include <QHostAddress>
#include <QScriptable>

Q_DECLARE_METATYPE(QHostAddress*)
Q_DECLARE_METATYPE(QHostAddress)

void setupQHostAddressProto(QScriptEngine *engine);
QScriptValue constructQHostAddress(QScriptContext *context, QScriptEngine *engine);

class QHostAddressProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QHostAddressProto(QObject *parent);
    ~QHostAddressProto();

    Q_INVOKABLE QString                   toString() const;

};

#endif
