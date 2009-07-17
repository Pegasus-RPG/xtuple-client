/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WORKCENTER_H
#define WORKCENTER_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_workCenter.h"

class workCenter : public XWidget, public Ui::workCenter
{
    Q_OBJECT

public:
    workCenter(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~workCenter();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();
    virtual void sPopulateOverheadRate();
    virtual void sPopulateSetupRate();
    virtual void sPopulateRunRate();
    virtual void populate();
    virtual void sPopulateLocations();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _wrkcntid;
    XSqlQuery _lbrrate;

};

#endif // WORKCENTER_H
