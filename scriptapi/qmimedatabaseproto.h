/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QMIMEDATABASEPROTO_H__
#define __QMIMEDATABASEPROTO_H__

#include <QObject>
#include <QtScript>
#include <QScriptEngine>
#include <QMimeDatabase>

Q_DECLARE_METATYPE(QMimeDatabase*)
Q_DECLARE_METATYPE(enum QMimeDatabase::MatchMode)

void setupQMimeDatabaseProto(QScriptEngine*);
QScriptValue constructQMimeDatabase(QScriptContext*, QScriptEngine*);

class QMimeDatabaseProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QMimeDatabaseProto(QObject*);
    virtual ~QMimeDatabaseProto();

    Q_INVOKABLE QList<QMimeType> allMimeTypes() const;
    Q_INVOKABLE QMimeType        mimeTypeForData(const QByteArray&) const;
    Q_INVOKABLE QMimeType        mimeTypeForData(QIODevice*) const;
    Q_INVOKABLE QMimeType        mimeTypeForFile(const QFileInfo&, QMimeDatabase::MatchMode =
                                                 QMimeDatabase::MatchDefault) const;
    Q_INVOKABLE QMimeType        mimeTypeForFile(const QString&, QMimeDatabase::MatchMode = 
                                                 QMimeDatabase::MatchDefault) const;
    Q_INVOKABLE QMimeType        mimeTypeForFileNameAndData(const QString&, QIODevice*) const;
    Q_INVOKABLE QMimeType        mimeTypeForFileNameAndData(const QString&,
                                                            const QByteArray&) const;
    Q_INVOKABLE QMimeType        mimeTypeForName(const QString&) const;
    Q_INVOKABLE QMimeType        mimeTypeForUrl(const QUrl&) const;
    Q_INVOKABLE QList<QMimeType> mimeTypesForFileName(const QString&) const;
    Q_INVOKABLE QString          suffixForFileName(const QString&) const;
};

#endif
