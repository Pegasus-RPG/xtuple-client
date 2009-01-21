/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTWOROUTING_H
#define PRINTWOROUTING_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_printWoRouting.h"

class printWoRouting : public XDialog, public Ui::printWoRouting
{
    Q_OBJECT

public:
    printWoRouting(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printWoRouting();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );

protected slots:
    virtual void languageChange();

    virtual void sPrint();


private:
    bool _captive;

};

#endif // PRINTWOROUTING_H
