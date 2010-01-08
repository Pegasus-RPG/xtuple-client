/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESHISTORYBYITEM_H
#define DSPSALESHISTORYBYITEM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspSalesHistoryByItem.h"

class dspSalesHistoryByItem : public XWidget, public Ui::dspSalesHistoryByItem
{
    Q_OBJECT

public:
    dspSalesHistoryByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSalesHistoryByItem();

    virtual bool checkParameters();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sHandleParams();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sEdit();
    virtual void sView();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESHISTORYBYITEM_H
