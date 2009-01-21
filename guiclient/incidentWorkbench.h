/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef INCIDENTWORKBENCH_H
#define INCIDENTWORKBENCH_H

#include "xwidget.h"

#include <parameter.h>
#include "ui_incidentWorkbench.h"

class incidentWorkbench : public XWidget, public Ui::incidentWorkbench
{
    Q_OBJECT

public:
    incidentWorkbench(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~incidentWorkbench();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sHandleAutoUpdate(bool);
    virtual void sPrint();
    virtual void sReset();
    virtual void sView();
    virtual void setParams(ParameterList&);

protected slots:
    virtual void languageChange();

};

#endif // INCIDENTWORKBENCH_H
