/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOBUFFERSTATUSBYPARAMETERLIST_H
#define DSPWOBUFFERSTATUSBYPARAMETERLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspWoBufferStatusByParameterList.h"

class dspWoBufferStatusByParameterList : public XWidget, public Ui::dspWoBufferStatusByParameterList
{
    Q_OBJECT

public:
    dspWoBufferStatusByParameterList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoBufferStatusByParameterList();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sView();
    virtual void sPostProduction();
    virtual void sCorrectProductionPosting();
    virtual void sPostOperations();
    virtual void sCorrectOperationsPosting();
    virtual void sReleaseWO();
    virtual void sRecallWO();
    virtual void sExplodeWO();
    virtual void sImplodeWO();
    virtual void sDeleteWO();
    virtual void sCloseWO();
    virtual void sPrintTraveler();
    virtual void sViewWomatl();
    virtual void sViewWooper();
    virtual void sInventoryAvailabilityByWorkOrder();
    virtual void sReprioritizeWo();
    virtual void sRescheduleWO();
    virtual void sChangeWOQty();
    virtual void sViewParentSO();
    virtual void sViewParentWO();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );

protected slots:
    virtual void languageChange();

};

#endif // DSPWOBUFFERSTATUSBYPARAMETERLIST_H
