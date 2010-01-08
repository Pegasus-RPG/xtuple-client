/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOITEMSBYDATE_H
#define DSPPOITEMSBYDATE_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspPoItemsByDate.h"

class dspPoItemsByDate : public XWidget, public Ui::dspPoItemsByDate
{
    Q_OBJECT

public:
    dspPoItemsByDate(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPoItemsByDate();

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu(QMenu *pMenu, QTreeWidgetItem * pSelected );
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sEditItem();
    virtual void sViewItem();
    virtual void sReschedule();
    virtual void sChangeQty();
    virtual void sCloseItem();
    virtual void sOpenItem();
    virtual void sFillList();
    virtual void sRunningAvailability();

protected slots:
    virtual void languageChange();
    virtual void setParams(ParameterList &params);

};

#endif // DSPPOITEMSBYDATE_H
