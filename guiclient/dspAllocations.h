/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPALLOCATIONS_H
#define DSPALLOCATIONS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspAllocations.h"

class dspAllocations : public XWidget, public Ui::dspAllocations
{
    Q_OBJECT

public:
    dspAllocations(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspAllocations();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sEditCustomerOrder();
    virtual void sEditTransferOrder();
    virtual void sFillList();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sViewCustomerOrder();
    virtual void sViewTransferOrder();
    virtual void sViewWorkOrder();

protected slots:
    virtual void languageChange();

};

#endif // DSPALLOCATIONS_H
