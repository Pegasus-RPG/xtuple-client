/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSINGLELEVELWHEREUSED_H
#define DSPSINGLELEVELWHEREUSED_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspSingleLevelWhereUsed.h"

class dspSingleLevelWhereUsed : public XWidget, public Ui::dspSingleLevelWhereUsed
{
    Q_OBJECT

public:
    dspSingleLevelWhereUsed(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspSingleLevelWhereUsed();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * menu );
    virtual void sEditBOM();
    virtual void sEditItem();
    virtual void sViewInventoryHistory();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool pLocal );

protected slots:
    virtual void languageChange();

};

#endif // DSPSINGLELEVELWHEREUSED_H
