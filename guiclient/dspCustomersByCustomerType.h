/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCUSTOMERSBYCUSTOMERTYPE_H
#define DSPCUSTOMERSBYCUSTOMERTYPE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspCustomersByCustomerType.h"

class dspCustomersByCustomerType : public XWidget, public Ui::dspCustomersByCustomerType
{
    Q_OBJECT

public:
    dspCustomersByCustomerType(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCustomersByCustomerType();

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sEdit();
    virtual void sView();
    virtual void sReassignCustomerType();
    virtual void sFillList();
    virtual void sHandleRefreshButton(bool pState);

protected slots:
    virtual void languageChange();

};

#endif // DSPCUSTOMERSBYCUSTOMERTYPE_H
