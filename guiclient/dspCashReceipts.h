/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCASHRECEIPTS_H
#define DSPCASHRECEIPTS_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "xerrormessage.h"

#include "ui_dspCashReceipts.h"

class dspCashReceipts : public XWidget, public Ui::dspCashReceipts
{
    Q_OBJECT

public:
    dspCashReceipts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCashReceipts();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sEditAropen();
    virtual void sEditCashrcpt();
    virtual void sFillList();
    virtual void sHandleButtons(bool);
    virtual void sNewCashrcpt();
    virtual void sPostCashrcpt();
    virtual void sPrint();
    virtual void sViewAropen();
    virtual void sViewCashrcpt();
    virtual void sReversePosted();

protected slots:
    virtual void languageChange();
    virtual void sPopulateMenu(QMenu *);
  
private:
    XErrorMessage *_upgradeWarn;

};

#endif // DSPCASHRECEIPTS_H
