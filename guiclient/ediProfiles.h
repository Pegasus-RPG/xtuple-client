/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EDIPROFILES_H
#define EDIPROFILES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_ediProfiles.h"

class ediProfiles : public XWidget, public Ui::ediProfiles
{
    Q_OBJECT

public:
    ediProfiles(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~ediProfiles();

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private slots:
    virtual void init();

};

#endif // EDIPROFILES_H
