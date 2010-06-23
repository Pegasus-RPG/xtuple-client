/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOSOSTATUSMISMATCH_H
#define DSPWOSOSTATUSMISMATCH_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspWoSoStatusMismatch.h"

class dspWoSoStatusMismatch : public XWidget, public Ui::dspWoSoStatusMismatch
{
    Q_OBJECT

public:
    dspWoSoStatusMismatch(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoSoStatusMismatch();
    virtual bool setParams(ParameterList & );

public slots:
    virtual void sPrint();
    virtual void sCloseWo();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sFillList();
    virtual void sViewWomatlreq();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOSOSTATUSMISMATCH_H
