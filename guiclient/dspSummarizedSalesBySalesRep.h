/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUMMARIZEDSALESBYSALESREP_H
#define DSPSUMMARIZEDSALESBYSALESREP_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspSummarizedSalesBySalesRep.h"

class dspSummarizedSalesBySalesRep : public XWidget, public Ui::dspSummarizedSalesBySalesRep
{
    Q_OBJECT

public:
    dspSummarizedSalesBySalesRep(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSummarizedSalesBySalesRep();

    virtual bool checkParameters();

public slots:
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sViewDetail();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPSUMMARIZEDSALESBYSALESREP_H
