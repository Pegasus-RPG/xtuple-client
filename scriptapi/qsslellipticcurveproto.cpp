/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslellipticcurveproto.h"

#if QT_VERSION < 0x050000
void setupQSslEllipticCurveProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
void setupQSslEllipticCurveProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslEllipticCurveProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslEllipticCurve*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslEllipticCurve, proto);
  engine->globalObject().setProperty("QSslEllipticCurve",  constructor);
}

QScriptValue constructQSslEllipticCurve(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QSslEllipticCurve *obj = 0;
  obj = new QSslEllipticCurve();
  return engine->toScriptValue(obj);
}

QSslEllipticCurveProto::QSslEllipticCurveProto(QObject *parent)
    : QObject(parent)
{
}

bool QSslEllipticCurveProto::isTlsNamedCurve() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->isTlsNamedCurve();
  return false;
}

bool QSslEllipticCurveProto::isValid() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QString QSslEllipticCurveProto::longName() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->longName();
  return QString();
}

QString QSslEllipticCurveProto::shortName() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->shortName();
  return QString();
}

QSslEllipticCurve QSslEllipticCurveProto::fromLongName(const QString & name)
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->fromLongName(name);
  return QSslEllipticCurve();
}

QSslEllipticCurve QSslEllipticCurveProto::fromShortName(const QString & name)
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->fromShortName(name);
  return QSslEllipticCurve();
}

#endif
