/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPUNINVOICEDRECEIVINGS_H
#define DSPUNINVOICEDRECEIVINGS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspUninvoicedReceivings.h"

class dspUninvoicedReceivings : public XWidget, public Ui::dspUninvoicedReceivings
{
    Q_OBJECT

public:
    dspUninvoicedReceivings(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspUninvoicedReceivings();

public slots:
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sMarkAsInvoiced();
    virtual void sCorrectReceiving();
    virtual void sCreateCreditMemo();
    virtual void sFillList();
    virtual void sPrint();
    virtual bool setParams(ParameterList&);

protected slots:
    virtual void languageChange();

};

#endif // DSPUNINVOICEDRECEIVINGS_H
