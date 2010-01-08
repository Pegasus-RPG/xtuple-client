/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOMATERIALSBYWORKORDER_H
#define DSPWOMATERIALSBYWORKORDER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspWoMaterialsByWorkOrder.h"

class dspWoMaterialsByWorkOrder : public XWidget, public Ui::dspWoMaterialsByWorkOrder
{
    Q_OBJECT

public:
    dspWoMaterialsByWorkOrder(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoMaterialsByWorkOrder();

    virtual bool checkParameters();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sView();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOMATERIALSBYWORKORDER_H
