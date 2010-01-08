/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPRUNNINGAVAILABILITY_H
#define DSPRUNNINGAVAILABILITY_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspRunningAvailability.h"

class dspRunningAvailability : public XWidget, public Ui::dspRunningAvailability
{
    Q_OBJECT

public:
    dspRunningAvailability(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspRunningAvailability();
    virtual void setParams(ParameterList&);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sDeleteOrder();
    virtual void sDspWoScheduleByWorkOrder();
    virtual void sFillList();
    virtual void sFirmOrder();
    virtual void sHandleResort();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sPrint();
    virtual void sReleaseOrder();
    virtual void sSoftenOrder();
    virtual void sViewSo();
    virtual void sViewTo();
    virtual void sViewWo();
    virtual void sViewPo();

protected slots:
    virtual void languageChange();

};

#endif // DSPRUNNINGAVAILABILITY_H
