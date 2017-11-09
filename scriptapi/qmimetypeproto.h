/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QMIMETYPEPROTO_H__
#define __QMIMETYPEPROTO_H__

#include <QObject>
#include <QtScript>
#include <QScriptEngine>
#include <QMimeType>

Q_DECLARE_METATYPE(QMimeType)
Q_DECLARE_METATYPE(QMimeType*)

void setupQMimeTypeProto(QScriptEngine*);
QScriptValue constructQMimeType(QScriptContext*, QScriptEngine*);

class QMimeTypeProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QMimeTypeProto(QObject*);
    virtual ~QMimeTypeProto();

    Q_INVOKABLE QStringList aliases() const;
    Q_INVOKABLE QStringList allAncestors() const;
    Q_INVOKABLE QString     comment() const;
    Q_INVOKABLE QString     filterString() const;
    Q_INVOKABLE QString     genericIconName() const;
    Q_INVOKABLE QStringList globPatterns() const;
    Q_INVOKABLE QString     iconName() const;
    Q_INVOKABLE bool        inherits(const QString&) const;
    Q_INVOKABLE bool        isDefault() const;
    Q_INVOKABLE bool        isValid() const;
    Q_INVOKABLE QString     name() const;
    Q_INVOKABLE QStringList parentMimeTypes() const;
    Q_INVOKABLE QString     preferredSuffix() const;
    Q_INVOKABLE QStringList suffixes() const;
    Q_INVOKABLE void        swap(QMimeType&);
    Q_INVOKABLE bool        operator!=(const QMimeType&) const;
    Q_INVOKABLE QMimeType&  operator=(const QMimeType&);
    Q_INVOKABLE bool        operator==(const QMimeType&) const;
};

#endif
