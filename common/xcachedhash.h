/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/* This header violates the coding standard of not having code in headers.
   That's because this is our first class template. We'll have to revise the
   standard if we do this again.
*/

#ifndef xtcachedhash_h
#define xtcachedhash_h

#include <QHash>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QString>
#include <QStringList>

template <class K, class v> class XCachedHash;

class XCachedHashQObject : public QObject
{
  Q_OBJECT

  public:
    explicit XCachedHashQObject(QObject *pParent = 0, QSqlDatabase pDb = QSqlDatabase::database());

  public slots:
    virtual void sConnectionLost();
    virtual void sNotified(const QString &pNotification);
    virtual void clear() = 0;

  protected:
    QStringList _notice;
};


/**
  @class XCachedHash

  @brief The XCachedHash combines a templated QHash with a QObject that clears
        itself upon receipt of a specific database notification or set of
        notifications.

  This allows for straightforward access to a named object that's stored in the
  database, such as a script or metasql statement with minimal database access.
  The database is queried whenever a named object is not in the hash. The value
  retrieved from the database is stored in the hash and returned to the caller.
  The next time a caller requests that object, it is returned from the hash directly.
  The hash clears itself when the named database notification is received. Thus
  the next time a caller requests that named object, the hash requeries.

  This is currently designed to be subclassed. @see MqlHash for an example.
 */
template <class K, class V>
class XCachedHash : public QHash<K, V>, public XCachedHashQObject
{
  public:
    virtual ~XCachedHash() { }

    virtual const V value(const K &key)
    {
      if (! QHash<K,V>::contains(key))
        (void)refresh(key);

      return QHash<K,V>::value(key);
    }

  protected:
    void setNotification(QStringList pNotification)
    {
      _notice << pNotification;
      foreach(QString notice, _notice)
      {
        if (!_db.driver()->subscribedToNotifications().contains(notice))
          _db.driver()->subscribeToNotification(notice);
      }
    }

    XCachedHash(QObject *pParent = 0, const QString &pNotification = QString(), QSqlDatabase pDb = QSqlDatabase::database())
      : QHash<K, V>(),
        XCachedHashQObject(pParent, pDb),
        _db(pDb)
    {
      (void)setNotification(QStringList() << pNotification);
    }

    XCachedHash(QObject *pParent, const QStringList &pNotification, QSqlDatabase pDb = QSqlDatabase::database())
      : QHash<K, V>(),
        XCachedHashQObject(pParent, pDb),
        _db(pDb)
    {
      (void)setNotification(pNotification);
    }

    virtual void clear()
    {
      QHash<K, V>::clear();
    }

    virtual bool refresh(const K &key) = 0;

    QSqlDatabase _db;
};

#endif

