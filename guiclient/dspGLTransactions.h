/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPGLTRANSACTIONS_H
#define DSPGLTRANSACTIONS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspGLTransactions.h"

class dspGLTransactions : public XWidget, public Ui::dspGLTransactions
{
    Q_OBJECT

public:
    dspGLTransactions(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspGLTransactions();
    virtual bool setParams(ParameterList &params);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*);
    virtual void sPrint();
    virtual void sFillList();
    virtual void sViewTrans();
    virtual void sViewSeries();
    virtual void sViewDocument();

protected slots:
    virtual void languageChange();
    virtual bool forwardUpdate();

private slots:
    void handleTotalCheckbox();

};

#endif // DSPGLTRANSACTIONS_H
