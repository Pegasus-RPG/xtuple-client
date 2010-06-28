/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUMMARIZEDSALESBYCUSTOMER_H
#define DSPSUMMARIZEDSALESBYCUSTOMER_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspSummarizedSalesByCustomer.h"

class dspSummarizedSalesByCustomer : public XWidget, public Ui::dspSummarizedSalesByCustomer
{
    Q_OBJECT

public:
    dspSummarizedSalesByCustomer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSummarizedSalesByCustomer();

    virtual bool checkParameters();

public slots:
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sViewDetail();
    virtual void sPrint();
    virtual void sFillList();
    virtual void setParams(ParameterList & params);

protected slots:
    virtual void languageChange();

};

#endif // DSPSUMMARIZEDSALESBYCUSTOMER_H
