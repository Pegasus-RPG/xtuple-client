/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPMATERIALUSAGEVARIANCEBYBOMITEM_H
#define DSPMATERIALUSAGEVARIANCEBYBOMITEM_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspMaterialUsageVarianceByBOMItem.h"

class dspMaterialUsageVarianceByBOMItem : public XWidget, public Ui::dspMaterialUsageVarianceByBOMItem
{
    Q_OBJECT

public:
    dspMaterialUsageVarianceByBOMItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspMaterialUsageVarianceByBOMItem();

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * );
    virtual void sPopulateComponentItems( int pItemid );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPMATERIALUSAGEVARIANCEBYBOMITEM_H
