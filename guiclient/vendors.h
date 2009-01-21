/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDORS_H
#define VENDORS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_vendors.h"

class vendors : public XWidget, public Ui::vendors
{
    Q_OBJECT

public:
    vendors(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~vendors();

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // VENDORS_H
