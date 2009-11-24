/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPRECURRINGINVOICES_H
#define DSPRECURRINGINVOICES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_dspRecurringInvoices.h"

class dspRecurringInvoices : public XWidget, public Ui::dspRecurringInvoices
{
    Q_OBJECT

public:
    dspRecurringInvoices(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspRecurringInvoices();
    virtual bool checkSitePrivs(int invcid);

public slots:
    virtual void sEdit();
    virtual void sView();
    virtual void sNew();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPRECURRINGINVOICES_H
