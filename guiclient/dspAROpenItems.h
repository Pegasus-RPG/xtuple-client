/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPAROPENITEMS_H
#define DSPAROPENITEMS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspAROpenItems.h"

class dspAROpenItems : public XWidget, public Ui::dspAROpenItems
{
    Q_OBJECT

public:
    dspAROpenItems(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspAROpenItems();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool setParams(ParameterList &);
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

#endif // DSPAROPENITEMS_H
