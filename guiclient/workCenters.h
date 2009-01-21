/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WORKCENTERS_H
#define WORKCENTERS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_workCenters.h"

class workCenters : public XWidget, public Ui::workCenters
{
    Q_OBJECT

public:
    workCenters(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~workCenters();

public slots:
    virtual void sView();

protected slots:
    virtual void languageChange();

    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sCopy();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

};

#endif // WORKCENTERS_H
