/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XWEBSYNC_H__
#define __XWEBSYNC_H__

#include <QObject>

class XWebSyncPrivate;

class XWebSync : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)
  Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

  public:
    explicit XWebSync(QObject *parent = 0);
    ~XWebSync();

    void setData(const QString &data);
    void setQuery(const QString &query);
    void setTitle(const QString &title);
    QString data() const;
    QString query() const;
    QString title() const;

  Q_SIGNALS:
    void executeRequest();
    void dataChanged(const QString &data);
    void queryChanged(const QString &query);
    void titleChanged(const QString &title);

  private:
    Q_DISABLE_COPY(XWebSync)
    Q_DECLARE_PRIVATE(XWebSync)
    QScopedPointer<XWebSyncPrivate> d_ptr;
};

#endif
