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
    return QString("QSqlDatabase(driver=%1, database=%2, "
                   "host=%3, port=%4, user=%5)")
                .arg(item->driverName()).arg(item->databaseName())
                .arg(item->hostName()).arg(item->port()).arg(item->userName());
  return QString("QSqlDatabase(unknown)");
}
