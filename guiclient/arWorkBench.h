/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ARWORKBENCH_H
#define ARWORKBENCH_H

#include "guiclient.h"

#include "xwidget.h"
#include <QMenu>

#include <parameter.h>

#include "ui_arWorkBench.h"

class arWorkBench : public XWidget, public Ui::arWorkBench
{
    Q_OBJECT

public:
    arWorkBench(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~arWorkBench();

    virtual SetResponse set( const ParameterList & pParams );

public slots:
    virtual bool setParams(ParameterList &params);
    virtual void sApplyAropenCM();
    virtual void sCCRefundCM();
    virtual void sDeleteCashrcpt();
    virtual void sEditAropen();
    virtual void sNewAropenCM();
    virtual void sEditAropenCM();
    virtual void sEditAropenOnlyCM();
    virtual void sEditCashrcpt();
    virtual void sFillAropenCMList();
    virtual void sFillAropenList();
    virtual void sFillCashrcptList();
    virtual void sFillList();
    virtual void sFillPreauthList();
    virtual void sNewCashrcpt();
    virtual void sPopulateAropenCMMenu(QMenu*);
    virtual void sPopulateAropenMenu(QMenu*);
    virtual void sPopulateCashRcptMenu(QMenu*);
    virtual void sPopulatePreauthMenu(QMenu*);
    virtual void sPostCashrcpt();
    virtual void sPostPreauth();
    virtual void sViewAropen();
    virtual void sViewInvoice();
    virtual void sViewInvoiceDetails();
    virtual void sViewAropenCM();
    virtual void sViewAropenOnlyCM();
    virtual void sViewCashrcpt();
    virtual void sVoidPreauth();
    virtual void sgetCCAmount();
    virtual void sClear();
    virtual void sSearchDocNumChanged();
    virtual void sPopulateAropenButtonMenu();
    virtual void sIncident();
    virtual void sViewIncident();
    virtual void sEditIncident();
    
protected slots:
    virtual void languageChange();

};

#endif // ARWORKBENCH_H
