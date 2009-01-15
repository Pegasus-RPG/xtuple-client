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

#include "qnetworkrequestproto.h"

#include <QByteArray>
#include <QList>
#include <QNetworkRequest>
#include <QUrl>
#include <QVariant>

#define DEBUG false

void setupQNetworkRequestProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQNetworkRequestProto entered");

  QScriptValue netreqproto = engine->newQObject(new QNetworkRequestProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), netreqproto);
  QScriptValue netreqConstructor = engine->newFunction(constructQNetworkRequest,
                                                       netreqproto);
  engine->globalObject().setProperty("QNetworkRequest", netreqConstructor);
}

QScriptValue constructQNetworkRequest(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  if (DEBUG) qDebug("constructQNetworkRequest called");
  QNetworkRequest *req = 0;

  if (context->argumentCount() > 0)
    context->throwError(QScriptContext::UnknownError,
                        "QNetworkRequest() constructors with "
                        "arguments are not supported");
  else
    req = new QNetworkRequest();

  return engine->toScriptValue(req);
}

QNetworkRequestProto::QNetworkRequestProto(QObject *parent)
  : QObject(parent)
{
}

QVariant QNetworkRequestProto::attribute(QNetworkRequest::Attribute code, const QVariant &defaultValue) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->attribute(code, defaultValue);
  return QVariant();
}

bool QNetworkRequestProto::hasRawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->hasRawHeader(headerName);
  return false;
}    

QVariant QNetworkRequestProto::header(QNetworkRequest::KnownHeaders header) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->header(header);
  return QVariant();
}

QByteArray QNetworkRequestProto::rawHeader(const QByteArray &headerName) const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->rawHeader(headerName);
  return QByteArray();
}

QList<QByteArray> QNetworkRequestProto::rawHeaderList() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->rawHeaderList();
  return QList<QByteArray>();
}

void QNetworkRequestProto::setAttribute(QNetworkRequest::Attribute code, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setAttribute(code, value);
}

void QNetworkRequestProto::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setHeader(header, value);
}

void QNetworkRequestProto::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setRawHeader(headerName, headerValue);
}

#ifndef QT_NO_OPENSSL
void QNetworkRequestProto::setSslConfiguration(const QSslConfiguration &config)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setSslConfiguration(config);
}

QSslConfiguration QNetworkRequestProto::sslConfiguration() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}
#endif

void QNetworkRequestProto::setUrl(const QUrl &url)
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    item->setUrl(url);
}

QUrl QNetworkRequestProto::url() const
{
  QNetworkRequest *item = qscriptvalue_cast<QNetworkRequest*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

QString QNetworkRequestProto::toString() const
{
  return QString("QNetworkRequest(url=%1)").arg(url());
}
