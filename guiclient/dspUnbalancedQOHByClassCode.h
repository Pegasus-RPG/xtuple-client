/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPUNBALANCEDQOHBYCLASSCODE_H
#define DSPUNBALANCEDQOHBYCLASSCODE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspUnbalancedQOHByClassCode.h"

class dspUnbalancedQOHByClassCode : public XWidget, public Ui::dspUnbalancedQOHByClassCode
{
    Q_OBJECT

public:
    dspUnbalancedQOHByClassCode(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspUnbalancedQOHByClassCode();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sBalance();
    virtual void sView();
    virtual void sEdit();
    virtual void sInventoryAvailability();
    virtual void sIssueCountTag();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPUNBALANCEDQOHBYCLASSCODE_H
