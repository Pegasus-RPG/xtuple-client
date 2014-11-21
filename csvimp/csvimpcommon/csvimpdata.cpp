/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvimpdata.h"

#include <QObject>

QString CSVImp::build     = QObject::tr("%1 %2").arg(__DATE__, __TIME__);
QString CSVImp::copyright = QObject::tr("Copyright (c) 1999-2014, OpenMFG, LLC");
QString CSVImp::name      = QObject::tr("CSV Import");
QString CSVImp::version   = QString("0.5.0");
