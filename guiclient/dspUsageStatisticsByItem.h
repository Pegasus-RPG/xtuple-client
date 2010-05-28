/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPUSAGESTATISTICSBYITEM_H
#define DSPUSAGESTATISTICSBYITEM_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspUsageStatisticsByItem.h"

class dspUsageStatisticsByItem : public XWidget, public Ui::dspUsageStatisticsByItem
{
    Q_OBJECT

public:
    dspUsageStatisticsByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspUsageStatisticsByItem();

    virtual enum SetResponse set(const ParameterList &);
    virtual void setParams(ParameterList & params);
    virtual void viewTransactions(QString);

public slots:
    virtual void sPrint();
    virtual void sViewAll();
    virtual void sViewReceipt();
    virtual void sViewIssue();
    virtual void sViewSold();
    virtual void sViewScrap();
    virtual void sViewAdjustment();
    virtual void sViewTransfer();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem *, int pColumn );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPUSAGESTATISTICSBYITEM_H
