/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __KEYTOOLS_H__
#define __KEYTOOLS_H__

#include <QString>
#include <QDate>

class XTupleProductKeyPrivate;

class XTupleProductKey
{
  public:
    XTupleProductKey(const QString &);
    virtual ~XTupleProductKey();

    bool valid() const;

    int version() const;
    QDate expiration() const;
    int users() const;
    QString customerId() const;
    bool perpetual() const;

  private:
    XTupleProductKeyPrivate * _private;
};

#endif
