/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CSVIMPPLUGININTERFACE_H__
#define __CSVIMPPLUGININTERFACE_H__
#include <QtCore/QObject>

class QMainWindow;

class CSVImpPluginInterface
{
	
  public:
    virtual ~CSVImpPluginInterface() {};

    virtual QMainWindow *getCSVAtlasWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0) = 0;
    virtual QMainWindow *getCSVToolWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0) = 0;
    virtual bool    importCSV()      = 0;
    virtual bool    isInteractive()  = 0;
    virtual QString lastError()      = 0;
    virtual bool    openAtlas(QString filename = QString()) = 0;
    virtual bool    openCSV(QString filename = QString())   = 0;
    virtual void    setAtlasDir(QString dirname)            = 0;
    virtual bool    setAtlasMap(const QString dirname)      = 0;
    virtual void    setCSVDir(QString dirname)              = 0;
    virtual bool    setFirstLineHeader(bool isheader)       = 0;
    virtual void    setInteractive(bool isinteractive)      = 0;

};

Q_DECLARE_INTERFACE(CSVImpPluginInterface,
                    "org.xtuple.Plugin.CSVImpPluginInterface/0.4");
#endif
