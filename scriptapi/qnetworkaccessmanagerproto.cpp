/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qnetworkaccessmanagerproto.h"

#include <QNetworkAccessManager>

// TODO: either complete this or remove it. we don't need this if all we're looking for is signals
void setupQNetworkAccessManagerCoreProto(QScriptEngine *engine)
{
  QScriptValue replyproto = engine->newQObject(new QNetworkAccessManagerProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkAccessManager*>(), replyproto);
}

QNetworkAccessManagerProto::QNetworkAccessManagerProto(QObject *parent)
  : QObject(parent)
{
}
