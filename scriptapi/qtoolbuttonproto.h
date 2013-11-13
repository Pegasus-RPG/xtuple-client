/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTOOLBUTTONPROTO_H__
#define __QTOOLBUTTONPROTO_H__


#include <QToolButton>
#include <QtScript>

Q_DECLARE_METATYPE(QToolButton*)


void setupQToolButtonProto(QScriptEngine *engine);
QScriptValue constructQToolButton(QScriptContext *context, QScriptEngine *engine);

class QToolButtonProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QToolButtonProto(QObject *parent);

    Q_INVOKABLE void setMenu(QMenu* menu);
    Q_INVOKABLE QMenu* menu() const;
    // TODO: fill in the rest of the function this class has
};

#endif
