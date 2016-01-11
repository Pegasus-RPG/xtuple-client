/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QNETWORKACCESSMANAGERPROTO_H__
#define __QNETWORKACCESSMANAGERPROTO_H__

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QtScript>

class QByteArray;

Q_DECLARE_METATYPE(QNetworkAccessManager*)

void setupQNetworkAccessManagerCoreProto(QScriptEngine *engine);

class QNetworkAccessManagerProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QNetworkAccessManagerProto(QObject *parent);

};

#endif
