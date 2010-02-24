/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPDETAILEDINVENTORYHISTORYBYLOTSERIAL_H
#define DSPDETAILEDINVENTORYHISTORYBYLOTSERIAL_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspDetailedInventoryHistoryByLotSerial.h"

class dspDetailedInventoryHistoryByLotSerial : public XWidget, public Ui::dspDetailedInventoryHistoryByLotSerial
{
    Q_OBJECT

public:
    dspDetailedInventoryHistoryByLotSerial(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspDetailedInventoryHistoryByLotSerial();

public slots:
    virtual void sPrint();
    virtual void sViewTransInfo();
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sFillList();
	virtual void sSelect();

protected slots:
    virtual void languageChange();

};

#endif // DSPDETAILEDINVENTORYHISTORYBYLOTSERIAL_H
