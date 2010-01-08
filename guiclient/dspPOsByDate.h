/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOSBYDATE_H
#define DSPPOSBYDATE_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspPOsByDate.h"

class dspPOsByDate : public XWidget, public Ui::dspPOsByDate
{
    Q_OBJECT

public:
    dspPOsByDate(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPOsByDate();

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

#endif // DSPPOSBYDATE_H
