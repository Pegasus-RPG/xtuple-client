/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "applock.h"

#include <QtScript>
#include <QSqlError>

#include "errorReporter.h"
#include "xsqlquery.h"

class AppLockPrivate
{
  public:
    AppLockPrivate(AppLock *parent)
      : _id(-1),
        _myLock(false),
        _otherLock(false),
        _parent(parent)
    {
    }

    QString  _error;
    int      _id;
    bool     _myLock;
    bool     _otherLock;
    AppLock *_parent;
    QString  _table;
};

AppLock::AppLock(QObject *parent)
  : QObject(parent)
{
  _p = new AppLockPrivate(this);
}

AppLock::AppLock(QString table, int id, QObject *parent)
  : QObject(parent)
{
  _p = new AppLockPrivate(this);
  (void)acquire(table, id);
}

AppLock::~AppLock()
{
  (void)release();
  delete _p;
  _p = 0;
}

/** Try to acquire an application-level lock on the record described in the
    constructor or last call to this AppLock's acquire(table, id).
  
   @return true if this AppLock instance appears to hold the lock
   @return false if the lock could not be acquired

   @see holdsLock()
   @see isLockedOut()
   @see lastError()
 */
bool AppLock::acquire()
{
  if (_p->_id < 0 || _p->_table.isEmpty()) {
    _p->_error = tr("Cannot acquire a lock without a table and record id.");
    return false;
  }

  bool result = false;
  XSqlQuery q;
  q.prepare("SELECT tryLock(CAST(oid AS INTEGER), :id) AS locked"
            "  FROM pg_class"
            " WHERE relname=:table;");
  q.bindValue(":id",    _p->_id);
  q.bindValue(":table", _p->_table);
  q.exec();
  if (q.first())
  {
    result = q.value("locked").toBool();
    // TODO: can we detect if _p->_myLock == true but the advisory lock was
    //       released somehow and someone else acquired it in the meantime?
    if (result || _p->_myLock)
    {
      _p->_myLock    = true;
      _p->_otherLock = false;
      _p->_error.clear();
      result = true;
    }
    else if (! _p->_myLock)
    {
      _p->_otherLock = true;
      _p->_error = tr("The record you are trying to edit is currently being "
                       "edited by another user.");
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    _p->_error = q.lastError().databaseText();
  }

  return result;
}

bool AppLock::acquire(QString table, int id)
{
  if (_p->_myLock && id != _p->_id && table != _p->_table)
  {
    _p->_error = tr("Cannot change the description of a locked object.");
    return false;
  }
  _p->_table = table;
  _p->_id    = id;

  return acquire();
}

/** @return true if _this_ instance of AppLock holds the lock */
bool AppLock::holdsLock() const
{
  return _p->_myLock;
}

/** @return true if the object appears locked by some other entity */
bool AppLock::isLockedOut() const
{
  return _p->_otherLock;
}

bool AppLock::release()
{
  if (_p->_id < 0 || _p->_table.isEmpty() || ! _p->_myLock)
  {
    // there's nothing for us to release
    return true;
  }

  if (_p->_otherLock)
  {
    _p->_error = tr("Should not release someone else' lock.");
    return false;
  }

  bool result = false;
  XSqlQuery q;
  q.prepare("SELECT pg_advisory_unlock(CAST(oid AS INTEGER), :id) AS unlocked"
            "  FROM pg_class"
            " WHERE relname = :table;");
  q.bindValue(":id",    _p->_id);
  q.bindValue(":table", _p->_table);
  q.exec();
  if (q.first())
  {
    result = q.value("unlocked").toBool();
    if (result)
    {
      _p->_error.clear();
      _p->_myLock    = false;
      _p->_otherLock = false;
    }
    else
    {
      _p->_error = tr("Could not release the lock.");
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    _p->_error = q.lastError().text();
  }

  return result;
}

QString AppLock::lastError() const
{
  return _p->_error;
}

QString AppLock::toString()  const
{
  QString result("AppLock[%1, %2 (%3)]");
  return result.arg(_p->_table).arg(_p->_id).arg(_p->_error);
}

// script exposure /////////////////////////////////////////////////////////////

QScriptValue AppLockToScriptValue(QScriptEngine *engine, AppLock *const &lock)
{
  return engine->newQObject(lock);
}

void AppLockFromScriptValue(const QScriptValue &obj, AppLock * &lock)
{
  lock = qobject_cast<AppLock *>(obj.toQObject());
}

void setupAppLockProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, AppLockToScriptValue, AppLockFromScriptValue);

  QScriptValue proto = engine->newQObject(new AppLockProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<AppLock*>(), proto);

  QScriptValue constructor = engine->newFunction(constructAppLock, proto);
  engine->globalObject().setProperty("AppLock", constructor);
}

QScriptValue constructAppLock(QScriptContext *context, QScriptEngine *engine)
{
  AppLock *lock = 0;

  if (context->argumentCount() == 0)
    lock = new AppLock();
  else if (context->argumentCount() == 1)
    lock = new AppLock(qscriptvalue_cast<QObject*>(context->argument(0)));
  else
  {
    QVariant table  = context->argument(0).toVariant();
    QVariant id     = context->argument(1).toVariant();
    QObject *parent = context->argument(2).toQObject();

    if (table.isValid() && id.isValid())
      lock = new AppLock(table.toString(), id.toInt(), parent);
    else
      context->throwError(QScriptContext::UnknownError,
                          QString("Could not find an appropriate AppLock constructor"));
  }

  return engine->toScriptValue(lock);
}

AppLockProto::AppLockProto(QObject *parent)
  : QObject(parent)
{
}

bool AppLockProto::acquire()
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->acquire();
  return false;
}

bool AppLockProto::acquire(QString table, int id)
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->acquire(table, id);
  return false;
}

bool AppLockProto::holdsLock() const
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->holdsLock();
  return false;
}

bool AppLockProto::isLockedOut() const
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->isLockedOut();
  return false;
}

QString AppLockProto::lastError() const
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->lastError();
  return QString(tr("This object does not appear to be an application lock."));
}

bool AppLockProto::release()
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->release();
  return false;
}

QString AppLockProto::toString() const
{
  AppLock *lock = qscriptvalue_cast<AppLock*>(thisObject());
  if (lock)
    return lock->toString();
  return QString();
}

