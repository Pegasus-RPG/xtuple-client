/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qmimedatabaseproto.h"

QScriptValue QMimeDatabaseMatchModeToScriptValue(QScriptEngine *engine,
                                                 const enum QMimeDatabase::MatchMode &in)
{
  return QScriptValue(engine, (int)in);
}

void QMimeDatabaseMatchModeFromScriptValue(const QScriptValue &object,
                                           enum QMimeDatabase::MatchMode &out)
{
  out = (enum QMimeDatabase::MatchMode)object.toInt32();
}

void setupQMimeDatabaseProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags ro = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QMimeDatabaseProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QMimeDatabase*>(), proto);

  QScriptValue ctor = engine->newFunction(constructQMimeDatabase, proto);

  qScriptRegisterMetaType(engine, QMimeDatabaseMatchModeToScriptValue, 
                                  QMimeDatabaseMatchModeFromScriptValue);
  ctor.setProperty("MatchDefault", QScriptValue(engine, QMimeDatabase::MatchDefault), ro);
  ctor.setProperty("MatchExtension", QScriptValue(engine, QMimeDatabase::MatchExtension), ro);
  ctor.setProperty("MatchContent", QScriptValue(engine, QMimeDatabase::MatchContent), ro);

  engine->globalObject().setProperty("QMimeDatabase", ctor);
}

QScriptValue constructQMimeDatabase(QScriptContext *context, QScriptEngine *engine)
{
  QMimeDatabase *obj = 0;
  if (context->argumentCount() == 0)
  {
    obj = new QMimeDatabase();
  }
  else
  {
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QMimeDatabase constructor");
  }

  return engine->toScriptValue(obj);
}

QMimeDatabaseProto::QMimeDatabaseProto(QObject *parent)
  : QObject(parent)
{
}

QMimeDatabaseProto::~QMimeDatabaseProto()
{
}

QList<QMimeType> QMimeDatabaseProto::allMimeTypes() const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->allMimeTypes();
  return QList<QMimeType>();
}

QMimeType QMimeDatabaseProto::mimeTypeForData(const QByteArray &data) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForData(data);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForData(QIODevice *device) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForData(device);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForFile(const QFileInfo &fileInfo,
                                              QMimeDatabase::MatchMode mode) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForFile(fileInfo, mode);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForFile(const QString &fileName, 
                                              QMimeDatabase::MatchMode mode) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForFile(fileName, mode);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForFileNameAndData(const QString &fileName,
                                                         QIODevice *device) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForFileNameAndData(fileName, device);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForFileNameAndData(const QString &fileName, 
                                                         const QByteArray &data) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForFileNameAndData(fileName, data);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForName(const QString &nameOrAlias) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForName(nameOrAlias);
  return QMimeType();
}

QMimeType QMimeDatabaseProto::mimeTypeForUrl(const QUrl &url) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypeForUrl(url);
  return QMimeType();
}

QList<QMimeType> QMimeDatabaseProto::mimeTypesForFileName(const QString &fileName) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->mimeTypesForFileName(fileName);
  return QList<QMimeType>();
}

QString QMimeDatabaseProto::suffixForFileName(const QString &fileName) const
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(thisObject());
  if (item)
    return item->suffixForFileName(fileName);
  return QString();
}
