/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDDEMANDBYPLANNERCODE_H
#define DSPTIMEPHASEDDEMANDBYPLANNERCODE_H

#include "xwidget.h"
#include <QList>

#include "ui_dspTimePhasedDemandByPlannerCode.h"

class dspTimePhasedDemandByPlannerCode : public XWidget, public Ui::dspTimePhasedDemandByPlannerCode
{
    Q_OBJECT

public:
    dspTimePhasedDemandByPlannerCode(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedDemandByPlannerCode();

public slots:
    virtual void sPrint();
    virtual void sSubmit();
    virtual void sViewDemand();
    virtual void sPopulateMenu( QMenu * menu, QTreeWidgetItem *, int pColumn );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    int _column;
    QList<DatePair> _columnDates;

    ParameterList buildParameters();
};

#endif // DSPTIMEPHASEDDEMANDBYPLANNERCODE_H
