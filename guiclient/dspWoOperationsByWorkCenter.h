/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOOPERATIONSBYWORKCENTER_H
#define DSPWOOPERATIONSBYWORKCENTER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspWoOperationsByWorkCenter.h"

class dspWoOperationsByWorkCenter : public XWidget, public Ui::dspWoOperationsByWorkCenter
{
    Q_OBJECT

public:
    dspWoOperationsByWorkCenter(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoOperationsByWorkCenter();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sViewOperation();
    virtual void sEditOperation();
    virtual void sDeleteOperation();
    virtual void sFillList();
    virtual void sPostProduction();
    virtual void sPostOperations();
    virtual void sPrintPickLists();
    virtual void sRunningAvailability();
    virtual void sMPSDetail();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOOPERATIONSBYWORKCENTER_H
