/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPBRIEFSALESHISTORYBYCUSTOMER_H
#define DSPBRIEFSALESHISTORYBYCUSTOMER_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspBriefSalesHistoryByCustomer.h"

class dspBriefSalesHistoryByCustomer : public XWidget, public Ui::dspBriefSalesHistoryByCustomer
{
    Q_OBJECT

public:
    dspBriefSalesHistoryByCustomer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspBriefSalesHistoryByCustomer();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPBRIEFSALESHISTORYBYCUSTOMER_H
