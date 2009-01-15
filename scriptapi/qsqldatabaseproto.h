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

#ifndef __QSQLDATABASEPROTO_H__
#define __QSQLDATABASEPROTO_H__

#include <QObject>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QtScript>

class QSqlDriver;

Q_DECLARE_METATYPE(QSqlDatabase*)
Q_DECLARE_METATYPE(QSqlDatabase)

void setupQSqlDatabaseProto(QScriptEngine *engine);
QScriptValue constructQSqlDatabase(QScriptContext *context, QScriptEngine *engine);

class QSqlDatabaseProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSqlDatabaseProto(QObject *parent);

    Q_INVOKABLE void         close();
    Q_INVOKABLE bool         commit();
    Q_INVOKABLE QString      connectOptions()   const;
    Q_INVOKABLE QString      connectionName()   const;
    Q_INVOKABLE QString      databaseName()     const;
    Q_INVOKABLE QSqlDriver  *driver()           const;
    Q_INVOKABLE QString      driverName()       const;
    Q_INVOKABLE QSqlQuery    exec(const QString &query = QString()) const;
    Q_INVOKABLE QString      hostName()         const;
    Q_INVOKABLE bool         isOpen()           const;
    Q_INVOKABLE bool         isOpenError()      const;
    Q_INVOKABLE bool         isValid()          const;
    Q_INVOKABLE QSqlError    lastError()        const;
    Q_INVOKABLE bool         open();
    Q_INVOKABLE bool         open(const QString &user, const QString &password);
    Q_INVOKABLE QString      password()         const;
    Q_INVOKABLE int          port()             const;
    Q_INVOKABLE QSqlIndex    primaryIndex(const QString &tablename) const;
    Q_INVOKABLE QSqlRecord   record(const QString &tablename)       const;
    Q_INVOKABLE bool         rollback();
    Q_INVOKABLE void         setConnectOptions(const QString &options = QString());
    Q_INVOKABLE void         setDatabaseName(const QString &name);
    Q_INVOKABLE void         setHostName(const QString &host);
    Q_INVOKABLE void         setPassword(const QString &password);
    Q_INVOKABLE void         setPort(int port);
    Q_INVOKABLE void         setUserName(const QString &name);
    Q_INVOKABLE QStringList  tables(int type = QSql::Tables) const;
    Q_INVOKABLE QString      toString()                      const;
    Q_INVOKABLE bool         transaction();
    Q_INVOKABLE QString      userName()                      const;
};

#endif
