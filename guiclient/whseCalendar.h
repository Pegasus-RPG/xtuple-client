/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WHSECALENDAR_H
#define WHSECALENDAR_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_whseCalendar.h"

class whseCalendar : public XDialog, public Ui::whseCalendar
{
    Q_OBJECT

  public:
    whseCalendar(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~whseCalendar();

  public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();

  protected slots:
    virtual void languageChange();

    virtual void sSave();

  private:
    int _mode;
    int _whsecalid;

};

#endif // WHSECALENDAR_H
