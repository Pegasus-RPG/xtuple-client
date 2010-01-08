/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINVENTORYAVAILABILITYBYITEM_H
#define DSPINVENTORYAVAILABILITYBYITEM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspInventoryAvailabilityByItem.h"

class dspInventoryAvailabilityByItem : public XWidget, public Ui::dspInventoryAvailabilityByItem
{
    Q_OBJECT

public:
    dspInventoryAvailabilityByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspInventoryAvailabilityByItem();

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * menu, QTreeWidgetItem * selected );
    virtual void sViewHistory();
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sRunningAvailability();
    virtual void sCreateWO();
    virtual void sCreatePR();
    virtual void sCreatePO();
    virtual void sViewSubstituteAvailability();
    virtual void sIssueCountTag();
    virtual void sEnterMiscCount();
    virtual void sHandleShowReorder( bool pValue );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _showByGroupInt;
};

#endif // DSPINVENTORYAVAILABILITYBYITEM_H
