/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINVENTORYAVAILABILITYBYCUSTOMERTYPE_H
#define DSPINVENTORYAVAILABILITYBYCUSTOMERTYPE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspInventoryAvailabilityByCustomerType.h"

class dspInventoryAvailabilityByCustomerType : public XWidget, public Ui::dspInventoryAvailabilityByCustomerType
{
    Q_OBJECT

public:
    dspInventoryAvailabilityByCustomerType(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspInventoryAvailabilityByCustomerType();

    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sRunningAvailability();
    virtual void sViewSubstituteAvailability();
    virtual void sIssuePO();
    virtual void sIssueWO();
    virtual void sIssueCountTag();
    virtual void sFillList();
    virtual void sAutoUpdateToggled(bool);
    virtual void sHandleReservationNetting(bool);
    virtual void sReserveStock();
    virtual void sReserveLineBalance();
    virtual void sUnreserveStock();
    virtual void sShowReservations();
    virtual void sAddToPackingListBatch();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _showByGroupInt;

};

#endif // DSPINVENTORYAVAILABILITYBYCUSTOMERTYPE_H
