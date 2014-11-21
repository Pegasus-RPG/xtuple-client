/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include "ui_logwindow.h"

class LogWindow : public QMainWindow, public Ui::LogWindow
{
  Q_OBJECT

  public:
    LogWindow(QWidget *parent = 0);
    ~LogWindow();

  protected slots:
    void languageChange();
    void sPrint();
};

#endif
