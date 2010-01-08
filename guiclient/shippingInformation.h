/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIPPINGINFORMATION_H
#define SHIPPINGINFORMATION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_shippingInformation.h"

class shippingInformation : public XDialog, public Ui::shippingInformation
{
    Q_OBJECT

public:
    shippingInformation(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~shippingInformation();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sSalesOrderList();
    virtual void sPopulateMenu(QMenu * menuThis );
    virtual void sIssueStock();
    virtual void sReturnAllLineStock();
    virtual void sViewLine();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // SHIPPINGINFORMATION_H
