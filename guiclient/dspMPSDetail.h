/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPMPSDETAIL_H
#define DSPMPSDETAIL_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspMPSDetail.h"

class dspMPSDetail : public XWidget, public Ui::dspMPSDetail
{
    Q_OBJECT

public:
    dspMPSDetail(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspMPSDetail();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *, int pColumn );
    virtual void sViewAllocations();
    virtual void sViewOrders();
    virtual void sIssuePR();
    virtual void sIssuePO();
    virtual void sIssueWO();
    virtual void sFillItemsites();
    virtual void sFillMPSDetail();

protected slots:
    virtual void languageChange();

private:
    int _column;

};

#endif // DSPMPSDETAIL_H
