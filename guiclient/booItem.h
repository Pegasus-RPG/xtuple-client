/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOOITEM_H
#define BOOITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_booItem.h"

class booItem : public XDialog, public Ui::booItem
{
    Q_OBJECT

public:
    booItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~booItem();

public slots:
    virtual SetResponse set( ParameterList & pParams );
    virtual void sSave();
    virtual void sHandleStdopn( int pStdopnid );
    virtual void sCalculateInvRunTime();
    virtual void sHandleFont( bool pFixed );
    virtual void populate();
    virtual void sPopulateLocations();
    virtual void sNewImage();
    virtual void sEditImage();
    virtual void sDeleteImage();
    virtual void sFillImageList();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _booitemid;
	int _revisionid;

};

#endif // BOOITEM_H
