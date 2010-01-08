/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPITEMSBYPRODUCTCATEGORY_H
#define DSPITEMSBYPRODUCTCATEGORY_H

#include "xwidget.h"

#include "ui_dspItemsByProductCategory.h"

class dspItemsByProductCategory : public XWidget, public Ui::dspItemsByProductCategory
{
    Q_OBJECT

public:
    dspItemsByProductCategory(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspItemsByProductCategory();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sEdit();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool pLocal );

protected slots:
    virtual void languageChange();

};

#endif // DSPITEMSBYPRODUCTCATEGORY_H
