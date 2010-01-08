/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDSALESBYCUSTOMERGROUP_H
#define DSPTIMEPHASEDSALESBYCUSTOMERGROUP_H

#include "xwidget.h"
#include <QList>
#include <parameter.h>
#include "ui_dspTimePhasedSalesByCustomerGroup.h"

class dspTimePhasedSalesByCustomerGroup : public XWidget, public Ui::dspTimePhasedSalesByCustomerGroup
{
    Q_OBJECT

public:
    dspTimePhasedSalesByCustomerGroup(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedSalesByCustomerGroup();

    virtual ParameterList buildParameters();

public slots:
    virtual void sPrint();
    virtual void sViewShipments();
    virtual void sPopulateMenu( QMenu * menuThis, QTreeWidgetItem *, int pColumn );
    virtual void sFillList();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    int _column;
    QList<DatePair> _columnDates;

};

#endif // DSPTIMEPHASEDSALESBYCUSTOMERGROUP_H
