/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <xabstractmessagehandler.h>

XAbstractMessageHandler::XAbstractMessageHandler(QObject *parent)
  : QAbstractMessageHandler(parent)
{
}

XAbstractMessageHandler::~XAbstractMessageHandler()
{
}

void XAbstractMessageHandler::message(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  QMutexLocker locker(&_mutex);
  _unhandledMessage.append(QPair<QtMsgType, QString>(type, description));
  handleMessage(type, description, identifier, sourceLocation);
}

void XAbstractMessageHandler::message(QtMsgType type, const QString title, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  QMutexLocker locker(&_mutex);
  _unhandledMessage.append(QPair<QtMsgType, QString>(type, description));
  handleMessage(type, title, description, identifier, sourceLocation);
}

// drains the queue but only returns those messages of the given type or greater
// on return, type holds the value of the greatest QtMsgType found
QStringList XAbstractMessageHandler::unhandledMessages(QtMsgType *type)
{
  QStringList unhandled;
  QtMsgType   dummy = QtDebugMsg;
  if (! type)
    type = &dummy;

  while (_unhandledMessage.size() > 0)
  {
    QPair<QtMsgType, QString> curr = _unhandledMessage.takeFirst();
    if (curr.first >= *type)
    {
      *type     = curr.first;
      unhandled.append(curr.second);
    }
  }

  return unhandled;
}
