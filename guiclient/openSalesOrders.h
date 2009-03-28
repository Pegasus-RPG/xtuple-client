/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPENSALESORDERS_H
#define OPENSALESORDERS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_openSalesOrders.h"

class openSalesOrders : public XWidget, public Ui::openSalesOrders
{
    Q_OBJECT

public:
    openSalesOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~openSalesOrders();
    virtual void setParams(ParameterList &);
    
    virtual bool checkSitePrivs();

public slots:
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sCopy();
    virtual void sReschedule();
    virtual void sDelete();
    virtual void sPrintPackingList();
    virtual void sAddToPackingListBatch();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();
    virtual void sDeliver();
    virtual void sPrintForms(); 
    virtual void sHandleAutoUpdate( bool pAutoUpdate );
    virtual void sDspShipmentStatus();
    virtual void sShipment();

protected slots:
    virtual void languageChange();

};

#endif // OPENSALESORDERS_H
