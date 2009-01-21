/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOEFFORTBYUSER_H
#define DSPWOEFFORTBYUSER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspWoEffortByUser.h"

class dspWoEffortByUser : public XWidget, public Ui::dspWoEffortByUser
{
    Q_OBJECT

public:
    dspWoEffortByUser(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoEffortByUser();

public slots:
    virtual void sPrint();
    virtual void sViewWO();
    virtual void sCloseWO();
    virtual void sViewWomatl();
    virtual void sViewWooper();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );

protected:
    virtual int getWoId();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOEFFORTBYUSER_H
