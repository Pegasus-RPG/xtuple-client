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

#include "qbytearrayproto.h"
#include "qfileinfoproto.h"
#include "qmimetypeproto.h"

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

QScriptValue mimeTypeForData(QScriptContext *context, QScriptEngine *engine)
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(context->thisObject());
  QMimeType obj;

  if (item)
  {
    if (context->argumentCount() == 1 && qscriptvalue_cast<QByteArray*>(context->argument(0)))
      obj = item->mimeTypeForData(*(qscriptvalue_cast<QByteArray*>(context->argument(0))));
    else if (context->argumentCount() == 1 && qscriptvalue_cast<QIODevice*>(context->argument(0)))
      obj = item->mimeTypeForData(qscriptvalue_cast<QIODevice*>(context->argument(0)));
    else
      context->throwError(QScriptContext::UnknownError, "Could not determine correct overload.");
  }

  return engine->toScriptValue(obj);
}

QScriptValue mimeTypeForFile(QScriptContext *context, QScriptEngine *engine)
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(context->thisObject());
  QMimeType obj;

  if (item)
  {
    if (context->argumentCount() == 2 && qscriptvalue_cast<QFileInfo*>(context->argument(0)))
      obj = item->mimeTypeForFile(*(qscriptvalue_cast<QFileInfo*>(context->argument(0))),
                                  (QMimeDatabase::MatchMode)context->argument(1).toInt32());
    else if (context->argumentCount() == 2 && context->argument(0).isString())
      obj = item->mimeTypeForFile(context->argument(0).toString(),
                                  (QMimeDatabase::MatchMode)context->argument(1).toInt32());
    else if (context->argumentCount() == 1 && qscriptvalue_cast<QFileInfo*>(context->argument(0)))
      obj = item->mimeTypeForFile(*(qscriptvalue_cast<QFileInfo*>(context->argument(0))));
    else if (context->argumentCount() == 1 && context->argument(0).isString())
      obj = item->mimeTypeForFile(context->argument(0).toString());
    else
      context->throwError(QScriptContext::UnknownError, "Could not determine correct overload.");
  }

  return engine->toScriptValue(obj);
}

QScriptValue mimeTypeForFileNameAndData(QScriptContext *context, QScriptEngine *engine)
{
  QMimeDatabase *item = qscriptvalue_cast<QMimeDatabase*>(context->thisObject());
  QMimeType obj;

  if (item)
  {
    if (context->argumentCount() == 2 && qscriptvalue_cast<QIODevice*>(context->argument(1)))
      obj = item->mimeTypeForFileNameAndData(context->argument(0).toString(),
                                             qscriptvalue_cast<QIODevice*>(context->argument(1)));
    else if (context->argumentCount() == 2 && qscriptvalue_cast<QByteArray*>(context->argument(1)))
      obj = item->mimeTypeForFileNameAndData(context->argument(0).toString(),
                                             *(qscriptvalue_cast<QByteArray*>(context->argument(1))));
    else
      context->throwError(QScriptContext::UnknownError, "Could not determine correct overload.");
  }

  return engine->toScriptValue(obj);
}

void setupQMimeDatabaseProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags ro = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QMimeDatabaseProto(engine));
  proto.setProperty("mimeTypeForData", engine->newFunction(mimeTypeForData));
  proto.setProperty("mimeTypeForFile", engine->newFunction(mimeTypeForFile));
  proto.setProperty("mimeTypeForFileNameAndData", engine->newFunction(mimeTypeForFileNameAndData));
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
