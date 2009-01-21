/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTOPERATIONS_H
#define POSTOPERATIONS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_postOperations.h"

class postOperations : public XDialog, public Ui::postOperations
{
    Q_OBJECT

public:
    postOperations(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postOperations();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sHandleWoid( int pWoid );
    virtual void sHandleWooperid( int );
    virtual void sHandleQty();
    virtual void sScrap();
    virtual void sPost();
    virtual void sHandlePostRunTime( bool pPostTime );
    virtual void sHandlePostSetupTime( bool pPostTime );
    virtual void sSetupChanged();
    virtual void sCatchWooperid(int);

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    double _rnqtyper;
    double _invProdUOMRatio;
    bool _usingWotc;
    int _wrkcntid;
    QVariant _wotc_id;
    double _wotcTime;
    double _balance;

};

#endif // POSTOPERATIONS_H
