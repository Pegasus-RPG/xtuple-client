/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PROSPECTS_H
#define PROSPECTS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_prospects.h"

class prospects : public XWidget, public Ui::prospects
{
    Q_OBJECT

public:
    prospects(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~prospects();

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sNew();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sView();

protected slots:
    virtual void languageChange();

};

#endif // PROSPECTS_H
