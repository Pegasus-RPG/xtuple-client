/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPRESERVATIONS_H
#define DSPRESERVATIONS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspReservations.h"

class dspReservations : public XWidget, public Ui::dspReservations
{
    Q_OBJECT

public:
    dspReservations(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspReservations();

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

#endif // DSPRESERVATIONS_H
