/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef __IMPORTHELPER_H__
#define __IMPORTHELPER_H__

#include <QDomDocument>
#include <QObject>
#include <QString>

#include <parameter.h>

class QScriptEngine;

class ImportHelper : public QObject
{
  Q_OBJECT

  public:
    static bool importXML(const QString &pFileName, QString &errmsg);
    static bool openDomDocument(const QString &pFileName, QDomDocument &pDoc, QString &errmsg);
};

void setupImportHelper(QScriptEngine *engine);

#endif
