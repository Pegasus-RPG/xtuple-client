/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSALESORDERSBYITEM_H
#define DSPSALESORDERSBYITEM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspSalesOrdersByItem.h"

class dspSalesOrdersByItem : public XWidget, public Ui::dspSalesOrdersByItem
{
    Q_OBJECT

public:
    dspSalesOrdersByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSalesOrdersByItem();
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual SetResponse set(const ParameterList & pParams );
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

#endif // DSPSALESORDERSBYITEM_H
