/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESHISTORYBYSHIPTO_H
#define DSPSALESHISTORYBYSHIPTO_H

#include "xwidget.h"

#include "ui_dspSalesHistoryByShipTo.h"

class dspSalesHistoryByShipTo : public XWidget, public Ui::dspSalesHistoryByShipTo
{
    Q_OBJECT

public:
    dspSalesHistoryByShipTo(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSalesHistoryByShipTo();

    virtual bool checkParameters();

public slots:
    virtual void sHandleParams();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sEdit();
    virtual void sView();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESHISTORYBYSHIPTO_H
