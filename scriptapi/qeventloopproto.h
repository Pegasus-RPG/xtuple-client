/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QEVENTLOOPPROTO_H__
#define __QEVENTLOOPPROTO_H__

#include <QEventLoop>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QEventLoop*)
Q_DECLARE_METATYPE(enum QEventLoop::ProcessEventsFlag)
Q_DECLARE_METATYPE(QEventLoop::ProcessEventsFlags)

void setupQEventLoopProto(QScriptEngine *engine);

class QEventLoopProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QEventLoopProto(QObject *parent);

    Q_INVOKABLE int  exec(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    Q_INVOKABLE void exit(int returnCode = 0);
    Q_INVOKABLE bool isRunning() const;
    Q_INVOKABLE void wakeUp();
};
#endif
