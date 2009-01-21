/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUMMARIZEDBACKLOGBYWAREHOUSE_H
#define DSPSUMMARIZEDBACKLOGBYWAREHOUSE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspSummarizedBacklogByWarehouse.h"

class dspSummarizedBacklogByWarehouse : public XWidget, public Ui::dspSummarizedBacklogByWarehouse
{
    Q_OBJECT

public:
    dspSummarizedBacklogByWarehouse(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSummarizedBacklogByWarehouse();

public slots:
    virtual void sHandlePrices( bool pShowPrices );
    virtual void sPrint();
    virtual void sInventoryAvailabilityBySalesOrder();
    virtual void sEdit();
    virtual void sView();
    virtual void sReschedule();
    virtual void sDelete();
    virtual void sPrintPackingList();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();
    virtual bool setParams(ParameterList &);

};

#endif // DSPSUMMARIZEDBACKLOGBYWAREHOUSE_H
