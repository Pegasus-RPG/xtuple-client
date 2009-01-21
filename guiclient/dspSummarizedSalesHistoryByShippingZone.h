/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUMMARIZEDSALESHISTORYBYSHIPPINGZONE_H
#define DSPSUMMARIZEDSALESHISTORYBYSHIPPINGZONE_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspSummarizedSalesHistoryByShippingZone.h"

class dspSummarizedSalesHistoryByShippingZone : public XWidget, public Ui::dspSummarizedSalesHistoryByShippingZone
{
    Q_OBJECT

public:
    dspSummarizedSalesHistoryByShippingZone(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSummarizedSalesHistoryByShippingZone();

public slots:
    virtual void sFillList();
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // DSPSUMMARIZEDSALESHISTORYBYSHIPPINGZONE_H
