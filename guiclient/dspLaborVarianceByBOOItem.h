/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPLABORVARIANCEBYBOOITEM_H
#define DSPLABORVARIANCEBYBOOITEM_H

#include "xwidget.h"

#include "ui_dspLaborVarianceByBOOItem.h"

class dspLaborVarianceByBOOItem : public XWidget, public Ui::dspLaborVarianceByBOOItem
{
    Q_OBJECT

public:
    dspLaborVarianceByBOOItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspLaborVarianceByBOOItem();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * );
    virtual void sPopulateComponentItems( int pItemid );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPLABORVARIANCEBYBOOITEM_H
