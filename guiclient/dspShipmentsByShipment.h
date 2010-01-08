/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSHIPMENTSBYSHIPMENT_H
#define DSPSHIPMENTSBYSHIPMENT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspShipmentsByShipment.h"

class dspShipmentsByShipment : public XWidget, public Ui::dspShipmentsByShipment
{
    Q_OBJECT

public:
    dspShipmentsByShipment(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspShipmentsByShipment();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pItem );
    virtual void sPrint();
    virtual void sPrintShippingForm();
    virtual void sFillList( int pSoheadid );
    virtual void sFillURL();

protected slots:
    virtual void languageChange();

};

#endif // DSPSHIPMENTSBYSHIPMENT_H
