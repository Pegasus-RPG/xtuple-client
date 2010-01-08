/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPOITEMRECEIVINGSBYITEM_H
#define DSPPOITEMRECEIVINGSBYITEM_H

#include "xwidget.h"

#include "ui_dspPoItemReceivingsByItem.h"

class dspPoItemReceivingsByItem : public XWidget, public Ui::dspPoItemReceivingsByItem
{
    Q_OBJECT

public:
    dspPoItemReceivingsByItem(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspPoItemReceivingsByItem();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sHandleVariance( bool pShowVariances );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPPOITEMRECEIVINGSBYITEM_H
