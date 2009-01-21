/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINCIDENTSBYCRMACCOUNT_H
#define DSPINCIDENTSBYCRMACCOUNT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspIncidentsByCRMAccount.h"

class dspIncidentsByCRMAccount : public XWidget, public Ui::dspIncidentsByCRMAccount
{
    Q_OBJECT

public:
    dspIncidentsByCRMAccount(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspIncidentsByCRMAccount();

public slots:
    virtual void sEditCRMAccount();
    virtual void sEditIncident();
    virtual void sEditTodoItem();
    virtual void sFillList();
    virtual void sPopulateMenu(QMenu *);
    virtual void sPrint();
    virtual void sViewCRMAccount();
    virtual void sViewIncident();
    virtual void sViewTodoItem();
    virtual enum SetResponse set(const ParameterList &);
    virtual bool setParams(ParameterList &);

protected slots:
    virtual void languageChange();
};
#endif // DSPINCIDENTSBYCRMACCOUNT_H
