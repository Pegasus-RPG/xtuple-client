

/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBELEMENTCOLLECTIONPROTO_H__
#define __QWEBELEMENTCOLLECTIONPROTO_H__

#include <QScriptEngine>

void setupQWebElementCollectionProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QWebElementCollection>

Q_DECLARE_METATYPE(QWebElementCollection*)
Q_DECLARE_METATYPE(QWebElementCollection)

QScriptValue construct(QScriptContext *context, QScriptEngine *engine);

class QWebElementCollectionProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebElementCollectionProto(QObject *parent);
    virtual ~QWebElementCollectionProto();

    Q_INVOKABLE void                append(const QWebElementCollection & other);
    Q_INVOKABLE QWebElement         at(int i) const;
    Q_INVOKABLE const_iterator      begin() const;
    Q_INVOKABLE iterator            begin();
    Q_INVOKABLE const_iterator      constBegin() const;
    Q_INVOKABLE const_iterator      constEnd() const;
    Q_INVOKABLE int                 count() const;
    Q_INVOKABLE const_iterator      end() const;
    Q_INVOKABLE iterator            end();
    Q_INVOKABLE QWebElement         first() const;
    Q_INVOKABLE QWebElement         last() const;
    Q_INVOKABLE QList<QWebElement>  toList() const;
};

#endif
#endif
