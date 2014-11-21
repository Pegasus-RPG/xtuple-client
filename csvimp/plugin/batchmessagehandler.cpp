/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "batchmessagehandler.h"

BatchMessageHandler::BatchMessageHandler(QObject *parent)
  : XAbstractMessageHandler(parent)
{
}

BatchMessageHandler::~BatchMessageHandler()
{
}

void BatchMessageHandler::handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  QString title;
  switch (type)
  {
    case QtDebugMsg:
      title = tr("Information");
      break;
    case QtWarningMsg:
      title = tr("Warning");
      break;
    case QtCriticalMsg:
    case QtFatalMsg:
    default:
      title = tr("Error");
      break;
  }

  handleMessage(type, title, description, identifier, sourceLocation);
}

void BatchMessageHandler::handleMessage(QtMsgType type, const QString title, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
  Q_UNUSED(type);
  Q_UNUSED(title);
  Q_UNUSED(description);
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);

  return;
}
