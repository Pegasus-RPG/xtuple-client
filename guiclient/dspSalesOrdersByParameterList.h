/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESORDERSBYPARAMETERLIST_H
#define DSPSALESORDERSBYPARAMETERLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspSalesOrdersByParameterList.h"

class dspSalesOrdersByParameterList : public XWidget, public Ui::dspSalesOrdersByParameterList
{
    Q_OBJECT

public:
    dspSalesOrdersByParameterList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSalesOrdersByParameterList();
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sCreateRA();
    virtual void sDspShipmentStatus();
    virtual void sDspShipments();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPSALESORDERSBYPARAMETERLIST_H
