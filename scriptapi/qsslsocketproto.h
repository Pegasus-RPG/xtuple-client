/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSSLSOCKETPROTO_H__
#define __QSSLSOCKETPROTO_H__

#include <QScriptEngine>

void setupQSslSocketProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QSslSocket>
#include "qtcpsocketproto.h"
class QSslSocket;

Q_DECLARE_METATYPE(QSslSocket*)
Q_DECLARE_METATYPE(enum QSslSocket::PeerVerifyMode)
Q_DECLARE_METATYPE(enum QSslSocket::SslMode)

QScriptValue constructQSslSocket(QScriptContext *context, QScriptEngine *engine);

class QSslSocketProto : public QTcpSocketProto
{
  Q_OBJECT

  public:
    QSslSocketProto(QObject *parent);
    virtual ~QSslSocketProto();

};

#endif
#endif
