/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOO_H
#define BOO_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_boo.h"

class boo : public XWidget, public Ui::boo
{
    Q_OBJECT

public:
    boo(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~boo();
    
    virtual bool checkSitePrivs( int booid );

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sExpire();
    virtual void sMoveUp();
    virtual void sMoveDown();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _booheadid;

};

#endif // BOO_H
