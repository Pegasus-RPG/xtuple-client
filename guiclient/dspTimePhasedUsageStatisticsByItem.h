/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDUSAGESTATISTICSBYITEM_H
#define DSPTIMEPHASEDUSAGESTATISTICSBYITEM_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspTimePhasedUsageStatisticsByItem.h"

#include <Q3ValueList>

class dspTimePhasedUsageStatisticsByItem : public XWidget, public Ui::dspTimePhasedUsageStatisticsByItem
{
    Q_OBJECT

public:
    dspTimePhasedUsageStatisticsByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedUsageStatisticsByItem();

public slots:
    virtual void sPrint();
    virtual void sSubmit();
    virtual void sViewTransactions();
    virtual void sPopulateMenu( QMenu * menu, QTreeWidgetItem *, int pColumn );
    virtual void sCalculate();

protected slots:
    virtual void languageChange();

private:
    int _column;
    Q3ValueList<DatePair> _columnDates;

    ParameterList buildParameters();
};

#endif // DSPTIMEPHASEDUSAGESTATISTICSBYITEM_H
