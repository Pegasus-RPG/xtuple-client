/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIFT_H
#define SHIFT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_shift.h"

class shift : public XDialog, public Ui::shift
{
    Q_OBJECT

public:
    shift(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~shift();

public slots:
    virtual SetResponse set( ParameterList & );
    virtual void sSave();
    virtual void sClose();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _shiftid;

    void init();

};

#endif // SHIFT_H
