/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPITEMCOSTDETAIL_H
#define DSPITEMCOSTDETAIL_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspItemCostDetail.h"

class dspItemCostDetail : public XWidget, public Ui::dspItemCostDetail
{
    Q_OBJECT

public:
    dspItemCostDetail(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspItemCostDetail();

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPopulate();
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sViewBomitem();
    virtual void sViewMaterialCosting();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool pLocale );

protected slots:
    virtual void languageChange();

};

#endif // DSPITEMCOSTDETAIL_H
