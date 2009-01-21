/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCAPACITYUOMSBYPRODUCTCATEGORY_H
#define DSPCAPACITYUOMSBYPRODUCTCATEGORY_H

#include "xwidget.h"

#include "ui_dspCapacityUOMsByProductCategory.h"

class dspCapacityUOMsByProductCategory : public XWidget, public Ui::dspCapacityUOMsByProductCategory
{
    Q_OBJECT

public:
    dspCapacityUOMsByProductCategory(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCapacityUOMsByProductCategory();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sEditItem();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool pLocalUpdate );

protected slots:
    virtual void languageChange();

};

#endif // DSPCAPACITYUOMSBYPRODUCTCATEGORY_H
