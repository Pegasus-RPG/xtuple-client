/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDSALESBYCUSTOMER_H
#define DSPTIMEPHASEDSALESBYCUSTOMER_H

#include "xwidget.h"
#include <QList>
#include <parameter.h>
#include "ui_dspTimePhasedSalesByCustomer.h"

class dspTimePhasedSalesByCustomer : public XWidget, public Ui::dspTimePhasedSalesByCustomer
{
    Q_OBJECT

public:
    dspTimePhasedSalesByCustomer(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedSalesByCustomer();

    virtual ParameterList buildParameters();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sViewShipments();
    virtual void sPopulateMenu( QMenu * menuThis, QTreeWidgetItem *, int pColumn );
    virtual void sCalculate();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    int _column;
    QList<DatePair> _columnDates;

};

#endif // DSPTIMEPHASEDSALESBYCUSTOMER_H
