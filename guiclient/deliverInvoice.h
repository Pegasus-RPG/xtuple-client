/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DELIVERINVOICE_H
#define DELIVERINVOICE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_deliverInvoice.h"

class deliverInvoice : public XDialog, public Ui::deliverInvoice
{
    Q_OBJECT

public:
    deliverInvoice(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~deliverInvoice();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSubmit();
    virtual void sHandleInvcheadid();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // DELIVERINVOICE_H
