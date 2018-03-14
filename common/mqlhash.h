/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef mqlcache_h
#define mqlcache_h

#include "xcachedhash.h"

class MqlHash : public XCachedHash<QString, QString>
{
  Q_OBJECT

  public:
    MqlHash(QObject *pParent = 0, QSqlDatabase pDb = QSqlDatabase::database());
    using XCachedHash::value;

    virtual       bool    refresh(const QString &key);
    virtual const QString value(const QString &pGroup, const QString &pName);
};

#endif

