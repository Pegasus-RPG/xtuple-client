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

#ifndef __QNETWORKREPLYPROTO_H__
#define __QNETWORKREPLYPROTO_H__

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSslConfiguration>
#include <QtScript>

class QByteArray;

Q_DECLARE_METATYPE(QNetworkReply*)

void setupQNetworkReplyProto(QScriptEngine *engine);

class QNetworkReplyProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QNetworkReplyProto(QObject *parent);

    Q_INVOKABLE void      abort() const;
    Q_INVOKABLE QVariant  attribute(const QNetworkRequest::Attribute &code) const;
    Q_INVOKABLE void      close();
    Q_INVOKABLE int       error() const;
    Q_INVOKABLE bool      hasRawHeader(const QByteArray &headerName)   const;
    Q_INVOKABLE QVariant  header(QNetworkRequest::KnownHeaders header) const;
    Q_INVOKABLE QNetworkAccessManager           *manager()   const;
    Q_INVOKABLE QNetworkAccessManager::Operation operation() const;
    Q_INVOKABLE QByteArray        rawHeader(const QByteArray &headerName) const;
    Q_INVOKABLE QList<QByteArray> rawHeaderList()    const;
    Q_INVOKABLE qint64            readBufferSize()   const;
    Q_INVOKABLE QNetworkRequest   request()          const;
    Q_INVOKABLE void              setReadBufferSize(qint64 size);
#ifndef QT_NO_OPENSSL
    Q_INVOKABLE void              setSslConfiguration(const QSslConfiguration &config);
    Q_INVOKABLE QSslConfiguration sslConfiguration() const;
#endif
    Q_INVOKABLE QString           toString()         const;
    Q_INVOKABLE QUrl              url()              const;

    // now for the QIODevice API
    Q_INVOKABLE virtual qint64      bytesAvailable()        const;
    Q_INVOKABLE virtual qint64      bytesToWrite()          const;
    Q_INVOKABLE virtual bool        canReadLine()           const;
    Q_INVOKABLE QString             errorString()           const;
    Q_INVOKABLE bool                getChar(char * c);
    Q_INVOKABLE bool                isOpen()                const;
    Q_INVOKABLE bool                isReadable()            const;
    Q_INVOKABLE virtual bool        isSequential()          const;
    Q_INVOKABLE bool                isTextModeEnabled()     const;
    Q_INVOKABLE bool                isWritable()            const;
    Q_INVOKABLE virtual bool        open(int mode);
    Q_INVOKABLE int                 openMode()              const;
    Q_INVOKABLE qint64              peek(char * data, qint64 maxSize);
    Q_INVOKABLE QByteArray          peek(qint64 maxSize);
    Q_INVOKABLE virtual qint64      pos()                   const;
    Q_INVOKABLE bool                putChar(char c);
    Q_INVOKABLE qint64              read(char * data, qint64 maxSize);
    Q_INVOKABLE QByteArray          read(qint64 maxSize);
    Q_INVOKABLE QByteArray          readAll();
    Q_INVOKABLE qint64              readLine(char * data, qint64 maxSize);
    Q_INVOKABLE QByteArray          readLine(qint64 maxSize = 0);
    Q_INVOKABLE virtual bool        reset();
    Q_INVOKABLE virtual bool        seek(qint64 pos);
    Q_INVOKABLE void                setTextModeEnabled(bool enabled);
    Q_INVOKABLE virtual qint64      size()                  const;
    Q_INVOKABLE void                ungetChar(char c);
    Q_INVOKABLE virtual bool        waitForBytesWritten(int msecs);
    Q_INVOKABLE virtual bool        waitForReadyRead(int msecs);
    Q_INVOKABLE qint64              write(const char * data, qint64 maxSize);
    Q_INVOKABLE qint64              write(const QByteArray &byteArray);
    Q_INVOKABLE qint64              write(const QString &string);
};

#endif
