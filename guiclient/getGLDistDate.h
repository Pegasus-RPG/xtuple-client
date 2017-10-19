/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GETGLDISTDATE_H
#define GETGLDISTDATE_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_getGLDistDate.h"

class getGLDistDate : public XDialog, public Ui::getGLDistDate
{
    Q_OBJECT

public:
    getGLDistDate(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~getGLDistDate();

    virtual QDate date() const;
    virtual QDate seriesDate() const;

public slots:
    virtual void sSetDefaultLit(const QString &);
    virtual void sSetSeriesLit(const QString &);

protected slots:
    virtual void languageChange();

};

#endif // GETGLDISTDATE_H
