/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTWOTRAVELER_H
#define PRINTWOTRAVELER_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_printWoTraveler.h"

class printWoTraveler : public XDialog, public Ui::printWoTraveler
{
    Q_OBJECT

public:
    printWoTraveler(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printWoTraveler();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sHandleOptions( int pWoid );
    virtual void sPrint();
    virtual void sHandlePrintButton();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // PRINTWOTRAVELER_H
