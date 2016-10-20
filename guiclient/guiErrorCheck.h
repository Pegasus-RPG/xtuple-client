/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GUIERRORCHECK_H
#define GUIERRORCHECK_H

#include <QString>
#include <QObject>
#include <QtScript>

class QWidget;

class GuiErrorCheck
{
  public:
    GuiErrorCheck(bool hasError, QWidget *widget, QString msg);
    GuiErrorCheck();
    ~GuiErrorCheck();

    static bool reportErrors(QWidget *parent, QString title, QList<GuiErrorCheck> list);

    // order of declarations should match order in constructor:
    // http://gcc.gnu.org/ml/gcc-help/2006-06/msg00068.html
    bool        hasError;
    QWidget    *widget;
    QString     msg;

};

// Script exposure

Q_DECLARE_METATYPE(GuiErrorCheck)

void setupGuiErrorCheckProto(QScriptEngine *engine);
QScriptValue constructGuiErrorCheck(QScriptContext *context, QScriptEngine *engine);

class GuiErrorCheckProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY(bool     hasError READ hasError)
  Q_PROPERTY(QWidget *widget   READ widget)
  Q_PROPERTY(QString  msg      READ msg)

  public:
    GuiErrorCheckProto(QObject *parent = 0);
    Q_INVOKABLE bool     hasError() const;
    Q_INVOKABLE QWidget *widget()   const;
    Q_INVOKABLE QString  msg()      const;

  public slots:
    QString toString() const;

};
#endif // GUIERRORCHECK_H
