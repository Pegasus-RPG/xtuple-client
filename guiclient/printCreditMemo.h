/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTCREDITMEMO_H
#define PRINTCREDITMEMO_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_printCreditMemo.h"

class printCreditMemo : public XDialog, public Ui::printCreditMemo
{
    Q_OBJECT

public:
    printCreditMemo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printCreditMemo();

    virtual void populate();
    virtual bool isSetup();
    virtual void setSetup(bool);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPrint();
    virtual void sHandleCopies( int pValue );
    virtual void sEditWatermark();

signals:
    virtual void finishedPrinting(int);

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    bool _setup;
    bool _alert;
    int _cmheadid;
    QPrinter _printer;

};

#endif // PRINTCREDITMEMO_H
