/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDAVAILABILITY_H
#define DSPTIMEPHASEDAVAILABILITY_H

#include "xwidget.h"
#include <QList>

#include "ui_dspTimePhasedAvailability.h"

class dspTimePhasedAvailability : public XWidget, public Ui::dspTimePhasedAvailability
{
    Q_OBJECT

public:
    dspTimePhasedAvailability(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedAvailability();

public slots:
    virtual void sPrint();
    virtual void sSubmit();
    virtual void sViewAvailability();
    virtual void sViewOrders();
    virtual void sViewAllocations();
    virtual void sCreateWO();
    virtual void sCreatePR();
    virtual void sCreatePO();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected, int pColumn );
    virtual void sCalculate();

protected slots:
    virtual void languageChange();

private:
    int _column;
    QList<DatePair> _columnDates;

    ParameterList buildParameters();
};

#endif // DSPTIMEPHASEDAVAILABILITY_H
