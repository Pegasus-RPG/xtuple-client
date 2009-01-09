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

#ifndef __QURLPROTO_H__
#define __QURLPROTO_H__

#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtScript>

class QByteArray;
class QSslConfiguration;

Q_DECLARE_METATYPE(QUrl*)

void         setupQUrlProto(QScriptEngine *engine);
QScriptValue constructQUrl(QScriptContext *context, QScriptEngine *engine);

class QUrlProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QUrlProto(QObject *parent = 0);

    Q_INVOKABLE void addEncodedQueryItem(const QByteArray &key, const QByteArray &value);
    Q_INVOKABLE void addQueryItem(const QString &key, const QString & value);
    Q_INVOKABLE QList<QByteArray> allEncodedQueryItemValues(const QByteArray &key) const;
    Q_INVOKABLE QStringList       allQueryItemValues(const QString &key) const;
    Q_INVOKABLE QString    authority() const;
    Q_INVOKABLE void       clear();
    Q_INVOKABLE QByteArray encodedFragment() const;
    Q_INVOKABLE QByteArray encodedHost()     const;
    Q_INVOKABLE QByteArray encodedPassword() const;
    Q_INVOKABLE QByteArray encodedPath()     const;
    Q_INVOKABLE QByteArray encodedQuery()    const;
    Q_INVOKABLE QByteArray encodedQueryItemValue(const QByteArray &key) const;
    Q_INVOKABLE QList<QPair<QByteArray, QByteArray> > encodedQueryItems() const;
    Q_INVOKABLE QByteArray encodedUserName() const;
    Q_INVOKABLE QString    errorString()     const;
    Q_INVOKABLE QString    fragment()       const;
    Q_INVOKABLE bool       hasEncodedQueryItem(const QByteArray &key) const;
    Q_INVOKABLE bool       hasFragment()    const;
    Q_INVOKABLE bool       hasQuery()       const;
    Q_INVOKABLE bool       hasQueryItem(const QString &key) const;
    Q_INVOKABLE QString    host()           const;
    Q_INVOKABLE bool       isEmpty()        const;
    Q_INVOKABLE bool       isParentOf(const QUrl &childUrl) const;
    Q_INVOKABLE bool       isRelative()     const;
    Q_INVOKABLE bool       isValid()        const;
    Q_INVOKABLE QString    password()       const;
    Q_INVOKABLE QString    path()           const;
    Q_INVOKABLE int        port()           const;
    Q_INVOKABLE int        port(int defaultPort) const;
    Q_INVOKABLE QString    queryItemValue(const QString &key) const;
    Q_INVOKABLE QList<QPair<QString, QString> > queryItems()  const;
    Q_INVOKABLE char       queryPairDelimiter()               const;
    Q_INVOKABLE char       queryValueDelimiter()              const;
    Q_INVOKABLE void       removeAllEncodedQueryItems(const QByteArray &key);
    Q_INVOKABLE void       removeAllQueryItems(const QString &key);
    Q_INVOKABLE void       removeEncodedQueryItem(const QByteArray &key);
    Q_INVOKABLE void       removeQueryItem(const QString &key);
    Q_INVOKABLE QUrl       resolved(const QUrl &relative)     const;
    Q_INVOKABLE QString    scheme() const;
    Q_INVOKABLE void       setAuthority(const QString &authority);
    Q_INVOKABLE void       setEncodedFragment(const QByteArray &fragment);
    Q_INVOKABLE void       setEncodedHost(const QByteArray &host);
    Q_INVOKABLE void       setEncodedPassword(const QByteArray &password);
    Q_INVOKABLE void       setEncodedPath(const QByteArray &path);
    Q_INVOKABLE void       setEncodedQuery(const QByteArray &query);
    Q_INVOKABLE void       setEncodedQueryItems(const QList<QPair<QByteArray, QByteArray> > &query);
    Q_INVOKABLE void       setEncodedUrl(const QByteArray &encodedUrl);
    Q_INVOKABLE void       setEncodedUrl(const QByteArray &encodedUrl,
                                         QUrl::ParsingMode parsingMode);
    Q_INVOKABLE void       setEncodedUserName(const QByteArray &userName);
    Q_INVOKABLE void       setFragment(const QString &fragment);
    Q_INVOKABLE void       setHost(const QString &host);
    Q_INVOKABLE void       setPassword(const QString &password);
    Q_INVOKABLE void       setPath(const QString &path);
    Q_INVOKABLE void       setPort(int port);
    Q_INVOKABLE void       setQueryDelimiters(char valueDelimiter, char pairDelimiter);
    Q_INVOKABLE void       setQueryItems(const QList<QPair<QString, QString> > &query);
    Q_INVOKABLE void       setQueryItems(const QVariantMap &map);
    Q_INVOKABLE void       setScheme(const QString &scheme);
    Q_INVOKABLE void       setUrl(const QString &url);
    Q_INVOKABLE void       setUrl(const QString &url, QUrl::ParsingMode parsingMode);
    Q_INVOKABLE void       setUserInfo(const QString &userInfo);
    Q_INVOKABLE void       setUserName(const QString &userName);
    Q_INVOKABLE QByteArray toEncoded(QUrl::FormattingOptions options = QUrl::None) const;
    Q_INVOKABLE QString    toLocalFile() const;
    Q_INVOKABLE QString    userInfo()    const;
    Q_INVOKABLE QString    userName()    const;

  public slots:
    Q_INVOKABLE QString    toString(QUrl::FormattingOptions options = QUrl::None)  const;

};

#endif
