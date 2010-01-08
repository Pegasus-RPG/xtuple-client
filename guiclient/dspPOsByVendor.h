/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOSBYVENDOR_H
#define DSPPOSBYVENDOR_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspPOsByVendor.h"

class dspPOsByVendor : public XWidget, public Ui::dspPOsByVendor
{
    Q_OBJECT

public:
    dspPOsByVendor(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPOsByVendor();

public slots:
    virtual void sPrint();
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sFillList();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected);
    virtual bool setParams(ParameterList &);

protected slots:
    virtual void languageChange();

};

#endif // DSPPOSBYVENDOR_H
