/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BATCHMESSAGEHANDLER_H
#define BATCHMESSAGEHANDLER_H

#include "xabstractmessagehandler.h"

class BatchMessageHandler : public XAbstractMessageHandler
{
  Q_OBJECT
    
  public:
    BatchMessageHandler(QObject *parent = 0);
    virtual ~BatchMessageHandler();

  protected:
    virtual void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation);
    virtual void handleMessage(QtMsgType type, const QString title, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation);
};

#endif

