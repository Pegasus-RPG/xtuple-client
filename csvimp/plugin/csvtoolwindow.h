/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CSVTOOLWINDOW_H
#define CSVTOOLWINDOW_H

#include "ui_csvtoolwindow.h"

class CSVAtlasWindow;
class CSVData;
class QTimerEvent;
class LogWindow;
class XAbstractMessageHandler;

class CSVToolWindow : public QMainWindow, public Ui::CSVToolWindow
{
  Q_OBJECT

  public:
    CSVToolWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~CSVToolWindow();
    CSVAtlasWindow *atlasWindow();

    XAbstractMessageHandler *messageHandler() const;
    void                     setMessageHandler(XAbstractMessageHandler *handler);

  public slots:
    void clearImportLog();
    void fileExit();
    void fileNew();
    void fileOpen(QString filename = QString());
    void filePrint();
    void fileSave();
    void fileSaveAs();
    void helpAbout();
    void helpContents();
    void helpIndex();
    bool importStart();
    void mapEdit();
    void sFirstRowHeader(bool yes);
    void sImportViewLog();
    QChar sNewDelimiter(QString delim);
    void setDir(QString dirname);
    void timerEvent(QTimerEvent *e);

  protected slots:
    void languageChange();
    void cleanup(QObject *deadobj);

  protected:
    CSVAtlasWindow *_atlasWindow;
    QString         _currentDir;
    CSVData        *_data;
    int             _dbTimerId;
    LogWindow      *_log;
    XAbstractMessageHandler *_msghandler;
    void populate();
};

#endif
