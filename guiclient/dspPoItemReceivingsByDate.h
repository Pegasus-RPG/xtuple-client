/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOITEMRECEIVINGSBYDATE_H
#define DSPPOITEMRECEIVINGSBYDATE_H

#include "xwidget.h"

#include "ui_dspPoItemReceivingsByDate.h"

class dspPoItemReceivingsByDate : public XWidget, public Ui::dspPoItemReceivingsByDate
{
    Q_OBJECT

public:
    dspPoItemReceivingsByDate(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPoItemReceivingsByDate();
    virtual bool setParams(ParameterList&);

public slots:
    virtual void sPrint();
    virtual void sHandleVariance( bool pShowVariances );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPPOITEMRECEIVINGSBYDATE_H
