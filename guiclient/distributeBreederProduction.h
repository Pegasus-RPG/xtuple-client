/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DISTRIBUTEBREEDERPRODUCTION_H
#define DISTRIBUTEBREEDERPRODUCTION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_distributeBreederProduction.h"

class distributeBreederProduction : public XDialog, public Ui::distributeBreederProduction
{
    Q_OBJECT

public:
    distributeBreederProduction(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~distributeBreederProduction();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sDistribute();
    virtual void sChangeQty();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    int _woid;

};

#endif // DISTRIBUTEBREEDERPRODUCTION_H
