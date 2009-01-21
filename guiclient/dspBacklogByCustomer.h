/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPBACKLOGBYCUSTOMER_H
#define DSPBACKLOGBYCUSTOMER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspBacklogByCustomer.h"

class dspBacklogByCustomer : public XWidget, public Ui::dspBacklogByCustomer
{
    Q_OBJECT

public:
    dspBacklogByCustomer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspBacklogByCustomer();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sHandlePrices( bool pShowPrices );
    virtual void sPrint();
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sEditItem();
    virtual void sViewItem();
    virtual void sPrintPackingList();
    virtual void sAddToPackingListBatch();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPBACKLOGBYCUSTOMER_H
