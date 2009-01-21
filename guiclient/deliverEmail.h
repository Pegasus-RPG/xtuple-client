/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DELIVEREMAIL_H
#define DELIVEREMAIL_H

#include <QSqlError>

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_deliverEmail.h"

class deliverEmail : public XDialog, public Ui::deliverEmail
{
    Q_OBJECT

public:
    deliverEmail(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~deliverEmail();
    
    static bool profileEmail(QWidget *parent, int profileid, ParameterList & pParams, ParameterList & pRptParams);
    static bool submitEmail(QWidget *parent, const QString to, const QString cc, const QString subject, const QString body);
    static bool submitEmail(QWidget *parent, const QString from, const QString to, const QString cc, const QString subject, const QString body);
    static bool submitEmail(QWidget *parent, const QString from, const QString to, const QString cc, const QString subject, const QString body, const bool emailHTML);
    static bool submitReport(QWidget *parent, const QString reportName, const QString fileName, const QString from, const QString to, const QString cc, const QString subject, const QString body, const bool emailHTML, ParameterList & rptParams);
    
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void setReportName(const QString & name) {_reportName = name;};
    virtual void setReportParameters(const ParameterList & rptParams) {_reportParams = rptParams;};

protected slots:
    virtual void languageChange();
    virtual void sSubmit();

private:    
    bool          _captive;
    QString       _reportName;
    ParameterList _reportParams;

};

#endif // DELIVEREMAIL_H
