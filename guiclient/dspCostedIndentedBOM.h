/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCOSTEDINDENTEDBOM_H
#define DSPCOSTEDINDENTEDBOM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspCostedIndentedBOM.h"

class dspCostedIndentedBOM : public XWidget, public Ui::dspCostedIndentedBOM
{
    Q_OBJECT

public:
    dspCostedIndentedBOM(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCostedIndentedBOM();

    virtual bool setParams(ParameterList &params);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sMaintainItemCosts();
    virtual void sViewItemCosting();
    virtual void sFillList();
    virtual void sFillList(int, bool);

protected slots:
    virtual void languageChange();

};

#endif // DSPCOSTEDINDENTEDBOM_H
