/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEPRICESBYPRICINGSCHEDULE_H
#define UPDATEPRICESBYPRICINGSCHEDULE_H

#include "xdialog.h"
#include "ui_updatePricesByPricingSchedule.h"

class updatePricesByPricingSchedule : public XDialog, public Ui::updatePricesByPricingSchedule
{
    Q_OBJECT

public:
    updatePricesByPricingSchedule(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~updatePricesByPricingSchedule();

public slots:
    virtual void sUpdate();
    virtual void sIPSChanged();

protected slots:
    virtual void languageChange();

};

#endif // UPDATEPRICESBYPRICINGSCHEDULE_H
