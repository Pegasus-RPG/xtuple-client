/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef __EXPORTHELPER_H__
#define __EXPORTHELPER_H__

#include <QFile>
#include <QObject>
#include <QString>

#include <parameter.h>

class ExportHelper : public QObject
{
  Q_OBJECT

  public:
    Q_INVOKABLE static bool exportXML(const int qryheadid, ParameterList &params, QString &filename, QString &errmsg, const int xsltmapid = -1);
    Q_INVOKABLE static bool XSLTConvert(QString inputfilename, QString outputfilename, int xsltmapid, QString &errmsg);
};

#endif
