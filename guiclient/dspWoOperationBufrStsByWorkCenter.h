/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOOPERATIONBUFRSTSBYWORKCENTER_H
#define DSPWOOPERATIONBUFRSTSBYWORKCENTER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspWoOperationBufrStsByWorkCenter.h"

class dspWoOperationBufrStsByWorkCenter : public XWidget, public Ui::dspWoOperationBufrStsByWorkCenter
{
    Q_OBJECT

public:
    dspWoOperationBufrStsByWorkCenter(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoOperationBufrStsByWorkCenter();

    virtual ParameterList buildParameters();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sViewOperation();
    virtual void sEditOperation();
    virtual void sDeleteOperation();
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOOPERATIONBUFRSTSBYWORKCENTER_H
