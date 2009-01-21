/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WOOPERATION_H
#define WOOPERATION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include <ui_woOperation.h>

class woOperation : public XDialog, public Ui::woOperation
{
    Q_OBJECT

public:
    woOperation(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~woOperation();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sCalculateInvRunTime();
    virtual void sPopulateWoInfo( int pWoid );
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sHandleFont( bool pFixed );
    virtual void sHandleStdopn( int pStdopnid );


private:
    int _mode;
    int _wooperid;
    bool _captive;
    double _cachedQtyOrdered;

};

#endif // WOOPERATION_H
