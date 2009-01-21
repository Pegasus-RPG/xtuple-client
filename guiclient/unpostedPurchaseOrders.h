/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDPURCHASEORDERS_H
#define UNPOSTEDPURCHASEORDERS_H

#include "xwidget.h"

#include "ui_unpostedPurchaseOrders.h"

class unpostedPurchaseOrders : public XWidget, public Ui::unpostedPurchaseOrders
{
    Q_OBJECT

public:
    unpostedPurchaseOrders(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~unpostedPurchaseOrders();
    
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual void sDelete();
    virtual void sDeliver();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sNew();
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*);
    virtual void sPost();
    virtual void sPrint();
    virtual void sView();

protected slots:
    virtual void languageChange();

};

#endif // UNPOSTEDPURCHASEORDERS_H
