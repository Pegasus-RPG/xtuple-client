/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPITEMSITESBYPARAMETERLIST_H
#define DSPITEMSITESBYPARAMETERLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspItemSitesByParameterList.h"

class dspItemSitesByParameterList : public XWidget, public Ui::dspItemSitesByParameterList
{
    Q_OBJECT

public:
    dspItemSitesByParameterList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspItemSitesByParameterList();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sView();
    virtual void sEdit();
    virtual void sInventoryAvailability();
    virtual void sIssueCountTag();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPITEMSITESBYPARAMETERLIST_H
