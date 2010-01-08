/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOHISTORYBYITEM_H
#define DSPWOHISTORYBYITEM_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspWoHistoryByItem.h"

class dspWoHistoryByItem : public XWidget, public Ui::dspWoHistoryByItem
{
    Q_OBJECT

public:
    dspWoHistoryByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoHistoryByItem();

    virtual bool checkParameters();

public slots:
    virtual void sPrint();
    virtual void sView();
    virtual void sEdit();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sHandleCosts( bool pShowCosts );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOHISTORYBYITEM_H
