/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOSOSTATUS_H
#define DSPWOSOSTATUS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspWoSoStatus.h"

class dspWoSoStatus : public XWidget, public Ui::dspWoSoStatus
{
    Q_OBJECT

public:
    dspWoSoStatus(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoSoStatus();

public slots:
    virtual void sPrint();
    virtual void sCloseWo();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );
    virtual void sViewWomatlreq();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOSOSTATUS_H
