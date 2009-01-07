/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "qnetworkaccessmanagerproto.h"

#include <QByteArray>
#include <QIODevice>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>

#define DEBUG false

// support functions
void setupQNetworkAccessManagerProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQNetworkAccessManagerProto entered");

  QScriptValue netmgrproto = engine->newQObject(new QNetworkAccessManagerProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkAccessManager*>(),
                              netmgrproto);

  QScriptValue netmgrConstructor = engine->newFunction(constructQNetworkAccessManager,
                                                       netmgrproto);
  engine->globalObject().setProperty("QNetworkAccessManager",
                                     netmgrConstructor);
}

QScriptValue constructQNetworkAccessManager(QScriptContext *context,
                                            QScriptEngine  *engine)
{
  if (DEBUG) qDebug("constructQNetworkAccessManager called");
  QNetworkAccessManager *netmgr = 0;

  if (context->argumentCount() == 1)
  {
    if (DEBUG) qDebug("netmgr(1 arg)");
    netmgr = new QNetworkAccessManager(context->argument(0).toQObject());
  }
  else
    netmgr = new QNetworkAccessManager();

  /* TODO: why do we need to relay the signals? the rest of this function
     should simply be:
   */
       return engine->toScriptValue(netmgr);
  /*
     instead of the following crud:
   */
  /*
  QScriptValue scriptvalue = engine->toScriptValue(netmgr);
  QObject *protoobj = scriptvalue.toQObject();
  QObject::connect(netmgr,
                 SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                 protoobj,
                 SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)));
  QObject::connect(netmgr,
                 SIGNAL(finished(QNetworkReply*)),
                 protoobj,
                 SIGNAL(finished(QNetworkReply*)));
  QObject::connect(netmgr,
                 SIGNAL(proxyAuthenticationRequired(QNetworkProxy&,QAuthenticator*)),
                 protoobj,
                 SIGNAL(proxyAuthenticationRequired(QNetworkReply&,QAuthenticator*)));
  QObject::connect(netmgr,
                 SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>&)),
                 protoobj,
                 SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>&)));
  return scriptvalue;
  // end of crud
   */
}

// QNetworkAccessManagerProto itself
QNetworkAccessManagerProto::QNetworkAccessManagerProto(QObject *parent)
  : QObject(parent)
{
}

QNetworkCookieJar *QNetworkAccessManagerProto::cookieJar() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->cookieJar();
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::get(const QNetworkRequest &request)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->get(request);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::head(const QNetworkRequest &request)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->head(request);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::post(const QNetworkRequest &request,
                                                QIODevice *data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->post(request, data);
  return 0;
}

QNetworkReply *QNetworkAccessManagerProto::post(const QNetworkRequest &request,
                                                  const QByteArray &data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->post(request, data);
  return 0;
}    

QNetworkReply *QNetworkAccessManagerProto::post(const QNetworkRequest &request,
                                                const QString &data)
{
  if (DEBUG)
    qDebug("QNetworkAccessManagerProto::post(QNetworkRequest, QString) entered");
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->post(request, data.toAscii());
  return 0;
}    

QNetworkProxy QNetworkAccessManagerProto::proxy() const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->proxy();
  return QNetworkProxy();
}

QNetworkReply *QNetworkAccessManagerProto::put(const QNetworkRequest &request,
                                               QIODevice *data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->put(request, data);
  return 0;
}

QNetworkReply * QNetworkAccessManagerProto::put(const QNetworkRequest &request,
                                                const QByteArray &data)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    return item->put(request, data);
  return 0;
}

void QNetworkAccessManagerProto::setCookieJar(QNetworkCookieJar *cookieJar)
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setCookieJar(cookieJar);
}

void QNetworkAccessManagerProto::setProxy(const QNetworkProxy &proxy) const
{
  QNetworkAccessManager *item = qscriptvalue_cast<QNetworkAccessManager*>(thisObject());
  if (item)
    item->setProxy(proxy);
}

QString QNetworkAccessManagerProto::toString() const
{
  return QString("QNetworkAccessManager(%1)")
      .arg(parent() ? parent()->objectName() : QString(""));
}
