/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINVENTORYLOCATOR_H
#define DSPINVENTORYLOCATOR_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspInventoryLocator.h"

class dspInventoryLocator : public XWidget, public Ui::dspInventoryLocator
{
    Q_OBJECT

public:
    dspInventoryLocator(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspInventoryLocator();

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sRelocateInventory();
    virtual void sReassignLotSerial();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPINVENTORYLOCATOR_H
