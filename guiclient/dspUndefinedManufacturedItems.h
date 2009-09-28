/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPUNDEFINEDMANUFACTUREDITEMS_H
#define DSPUNDEFINEDMANUFACTUREDITEMS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspUndefinedManufacturedItems.h"

class dspUndefinedManufacturedItems : public XWidget, public Ui::dspUndefinedManufacturedItems
{
    Q_OBJECT

public:
    dspUndefinedManufacturedItems(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspUndefinedManufacturedItems();

public slots:
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sCreateBOM();
    virtual void sEditItem();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool pLocal );

protected slots:
    virtual void languageChange();

};

#endif // DSPUNDEFINEDMANUFACTUREDITEMS_H
