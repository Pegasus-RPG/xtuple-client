/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCAPACITYBUFFERSTATUSBYWORKCENTER_H
#define DSPCAPACITYBUFFERSTATUSBYWORKCENTER_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspCapacityBufferStatusByWorkCenter.h"

class dspCapacityBufferStatusByWorkCenter : public XWidget, public Ui::dspCapacityBufferStatusByWorkCenter
{
    Q_OBJECT

public:
    dspCapacityBufferStatusByWorkCenter(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCapacityBufferStatusByWorkCenter();

public slots:
    virtual void sPrint();
    virtual void sQuery();

protected slots:
    virtual void languageChange();

};

#endif // DSPCAPACITYBUFFERSTATUSBYWORKCENTER_H
