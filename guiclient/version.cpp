/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "version.h"

QString _Name        = "xTuple ERP: %1 Edition";
QString _Version     = "4.11.3";
QString _dbVersion   = "4.11.3";
QString _Copyright   = "Copyright (c) 1999-2018, OpenMFG, LLC.";
QString _ConnAppName = "xTuple ERP (qt-client)";

#ifdef __USEALTVERSION__
#include "altVersion.cpp"
#else
QString _Build     = "%1 %2";
#endif
