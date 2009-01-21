/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDROUGHCUTBYWORKCENTER_H
#define DSPTIMEPHASEDROUGHCUTBYWORKCENTER_H

#include "xwidget.h"
#include <QList>

#include "ui_dspTimePhasedRoughCutByWorkCenter.h"

class dspTimePhasedRoughCutByWorkCenter : public XWidget, public Ui::dspTimePhasedRoughCutByWorkCenter
{
    Q_OBJECT

public:
    dspTimePhasedRoughCutByWorkCenter(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedRoughCutByWorkCenter();

public slots:
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    QList<DatePair> _columnDates;

};

#endif // DSPTIMEPHASEDROUGHCUTBYWORKCENTER_H
