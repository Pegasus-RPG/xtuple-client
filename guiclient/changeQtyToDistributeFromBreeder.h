/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHANGEQTYTODISTRIBUTEFROMBREEDER_H
#define CHANGEQTYTODISTRIBUTEFROMBREEDER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_changeQtyToDistributeFromBreeder.h"

class changeQtyToDistributeFromBreeder : public XDialog, public Ui::changeQtyToDistributeFromBreeder
{
    Q_OBJECT

public:
    changeQtyToDistributeFromBreeder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~changeQtyToDistributeFromBreeder();

public slots:
    virtual enum SetResponse set( ParameterList & pParams );
    virtual void sUpdateQtyPer();
    virtual void sSave();

protected slots:
    virtual void languageChange();

private:
    int _brddistid;
    double _cachedOpenWoQty;

    void init();

};

#endif // CHANGEQTYTODISTRIBUTEFROMBREEDER_H
