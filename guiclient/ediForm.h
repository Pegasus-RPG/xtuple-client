/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EDIFORM_H
#define EDIFORM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_ediForm.h"

class ediForm : public XDialog, public Ui::ediForm
{
    Q_OBJECT

public:
    ediForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~ediForm();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sCSVNew();
    virtual void sCSVEdit();
    virtual void sCSVDelete();

protected:
    int _ediformid;
    int _ediprofileid;
    int _mode;

    virtual bool save();
    virtual void populate();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

    virtual void sTypeSelected( int );

};

#endif // EDIFORM_H
