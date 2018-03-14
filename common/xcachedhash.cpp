/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xcachedhash.h"

XCachedHashQObject::XCachedHashQObject(QObject *pParent, QSqlDatabase pDb)
  : QObject(pParent)
{
  connect(pDb.driver(), SIGNAL(notification(const QString&)), this, SLOT(sNotified(const QString&)));
}

void XCachedHashQObject::sConnectionLost()
{
  clear();
}

void XCachedHashQObject::sNotified(const QString &pNotification)
{
  if (_notice.contains(pNotification))
    clear();
}

