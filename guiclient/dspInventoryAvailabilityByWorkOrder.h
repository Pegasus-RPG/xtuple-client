/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINVENTORYAVAILABILITYBYWORKORDER_H
#define DSPINVENTORYAVAILABILITYBYWORKORDER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspInventoryAvailabilityByWorkOrder.h"

class dspInventoryAvailabilityByWorkOrder : public XWidget, public Ui::dspInventoryAvailabilityByWorkOrder
{
    Q_OBJECT

public:
    dspInventoryAvailabilityByWorkOrder(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspInventoryAvailabilityByWorkOrder();

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sViewHistory();
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sRunningAvailability();
    virtual void sViewSubstituteAvailability();
    virtual void sCreatePR();
    virtual void sCreatePO();
    virtual void sCreateWO();
    virtual void sPostMiscProduction();
    virtual void sIssueCountTag();
    virtual void sEnterMiscCount();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPINVENTORYAVAILABILITYBYWORKORDER_H
