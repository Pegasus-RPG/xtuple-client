/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOITEMSBYVENDOR_H
#define DSPPOITEMSBYVENDOR_H

#include "guiclient.h"
#include "xwidget.h"

#include <parameter.h>

#include "ui_dspPoItemsByVendor.h"

class dspPoItemsByVendor : public XWidget, public Ui::dspPoItemsByVendor
{
    Q_OBJECT

public:
    dspPoItemsByVendor(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPoItemsByVendor();

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sRunningAvailability();
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sEditItem();
    virtual void sViewItem();
    virtual void sReschedule();
    virtual void sChangeQty();
    virtual void sCloseItem();
    virtual void sOpenItem();
    virtual void sFillList();
    virtual void sPopulatePo();
    virtual void sSearch( const QString & pTarget );
    virtual void sSearchNext();

protected slots:
    virtual void languageChange();
    virtual void setParams(ParameterList &);

};

#endif // DSPPOITEMSBYVENDOR_H
