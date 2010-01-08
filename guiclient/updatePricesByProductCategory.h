/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEPRICESBYPRODUCTCATEGORY_H
#define UPDATEPRICESBYPRODUCTCATEGORY_H

#include "xdialog.h"
#include "ui_updatePricesByProductCategory.h"

class updatePricesByProductCategory : public XDialog, public Ui::updatePricesByProductCategory
{
    Q_OBJECT

public:
    updatePricesByProductCategory(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~updatePricesByProductCategory();

public slots:
    virtual void sUpdate();
    virtual void sHandleCharPrice();

protected slots:
    virtual void languageChange();

};

#endif // UPDATEPRICESBYPRODUCTCATEGORY_H
