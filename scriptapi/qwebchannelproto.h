/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBCHANNELPROTO_H__
#define __QWEBCHANNELPROTO_H__

#include <QtScript>

void setupQWebChannelProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000

#include <QHash>
#include <QWebChannel>

Q_DECLARE_METATYPE(QWebChannel*)

QScriptValue constructQWebChannel(QScriptContext *context, QScriptEngine *engine);
class QWebChannelProto :public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebChannelProto(QObject *parent = 0);
    Q_INVOKABLE ~QWebChannelProto();

    Q_INVOKABLE bool                          blockUpdates() const;
    Q_INVOKABLE void                          deregisterObject(QObject * object);
    Q_INVOKABLE void                          registerObject(const QString & id, QObject * object);
    Q_INVOKABLE void                          registerObjects(const QHash<QString, QObject *> & objects);
    Q_INVOKABLE QHash<QString, QObject *>     registeredObjects() const;
    Q_INVOKABLE void                          setBlockUpdates(bool block);

  public slots:

    void connectTo(QWebChannelAbstractTransport * transport);
    void disconnectFrom(QWebChannelAbstractTransport * transport);

};

#endif

#endif
