/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOPRICEVARIANCESBYVENDOR_H
#define DSPPOPRICEVARIANCESBYVENDOR_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspPoPriceVariancesByVendor.h"

class dspPoPriceVariancesByVendor : public XWidget, public Ui::dspPoPriceVariancesByVendor
{
    Q_OBJECT

public:
    dspPoPriceVariancesByVendor(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPoPriceVariancesByVendor();

public slots:
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPPOPRICEVARIANCESBYVENDOR_H
