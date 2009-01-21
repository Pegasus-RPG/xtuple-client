/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPAROPENITEMSBYCUSTOMER_H
#define DSPAROPENITEMSBYCUSTOMER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspAROpenItemsByCustomer.h"

class dspAROpenItemsByCustomer : public XWidget, public Ui::dspAROpenItemsByCustomer
{
    Q_OBJECT

public:
    dspAROpenItemsByCustomer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspAROpenItemsByCustomer();
    virtual bool setParams(ParameterList&);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *pItem );
    virtual void sEdit();
    virtual void sView();
    virtual void sViewInvoice();
    virtual void sViewInvoiceDetails();
    virtual void sIncident();
    virtual void sEditIncident();
    virtual void sViewIncident();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPAROPENITEMSBYCUSTOMER_H
