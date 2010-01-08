/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsqldatabaseproto.h"

void setupQSqlDatabaseProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSqlDatabaseProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSqlDatabase*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSqlDatabase>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQSqlDatabase,
                                                 proto);
  engine->globalObject().setProperty("QSqlDatabase",  constructor);
}

QScriptValue constructQSqlDatabase(QScriptContext *context,
                                    QScriptEngine *engine)
{
  QSqlDatabase *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QSqlDatabase(qscriptvalue_cast<QSqlDatabase>(context->argument(0)));
  else
    obj = new QSqlDatabase();
  return engine->toScriptValue(obj);
}

QSqlDatabaseProto::QSqlDatabaseProto(QObject *parent)
    : QObject(parent)
{
}

void QSqlDatabaseProto::close()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->close();
}

bool QSqlDatabaseProto::commit()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->commit();
  return false;
}

QString QSqlDatabaseProto::connectOptions()   const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->connectOptions();
  return QString();
}

QString QSqlDatabaseProto::connectionName()   const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->connectionName();
  return QString();
}

QString QSqlDatabaseProto::databaseName()     const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->databaseName();
  return QString();
}

QSqlDriver *QSqlDatabaseProto::driver()           const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->driver();
  return 0;
}

QString QSqlDatabaseProto::driverName()       const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->driverName();
  return QString();
}

QSqlQuery QSqlDatabaseProto::exec(const QString &query) const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->exec(query);
  return QSqlQuery();
}

QString QSqlDatabaseProto::hostName()         const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->hostName();
  return QString();
}

bool QSqlDatabaseProto::isOpen()           const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->isOpen();
  return false;
}

bool QSqlDatabaseProto::isOpenError()      const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->isOpenError();
  return false;
}

bool QSqlDatabaseProto::isValid()          const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QSqlError QSqlDatabaseProto::lastError()        const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->lastError();
  return QSqlError();
}

bool QSqlDatabaseProto::open()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->open();
  return false;
}

bool QSqlDatabaseProto::open(const QString &user, const QString &password)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->open(user, password);
  return false;
}

QString QSqlDatabaseProto::password()         const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->password();
  return QString();
}

int QSqlDatabaseProto::port()             const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->port();
  return 0;
}

QSqlIndex QSqlDatabaseProto::primaryIndex(const QString &tablename) const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->primaryIndex(tablename);
  return QSqlIndex();
}

QSqlRecord QSqlDatabaseProto::record(const QString &tablename)       const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->record(tablename);
  return QSqlRecord();
}

bool QSqlDatabaseProto::rollback()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->rollback();
  return false;
}

void QSqlDatabaseProto::setConnectOptions(const QString &options)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setConnectOptions(options);
}

void QSqlDatabaseProto::setDatabaseName(const QString &name)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setDatabaseName(name);
}

void QSqlDatabaseProto::setHostName(const QString &host)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setHostName(host);
}

void QSqlDatabaseProto::setPassword(const QString &password)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setPassword(password);
}

void QSqlDatabaseProto::setPort(int port)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setPort(port);
}

void QSqlDatabaseProto::setUserName(const QString &name)
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    item->setUserName(name);
}

QStringList QSqlDatabaseProto::tables(int type) const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->tables((QSql::TableType)type);
  return QStringList();
}

bool QSqlDatabaseProto::transaction()
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->transaction();
  return false;
}

QString QSqlDatabaseProto::userName() const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return item->userName();
  return QString();
}

QString QSqlDatabaseProto::toString() const
{
  QSqlDatabase *item = qscriptvalue_cast<QSqlDatabase*>(thisObject());
  if (item)
    return QString("[QSqlDatabase(driver=%1, database=%2, "
                   "host=%3, port=%4, user=%5)]")
                .arg(item->driverName()).arg(item->databaseName())
                .arg(item->hostName()).arg(item->port()).arg(item->userName());
  return QString("[QSqlDatabase(unknown)]");
}
