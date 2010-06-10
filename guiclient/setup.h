/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SETUP_H
#define SETUP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "configureCC.h"
#include "configureCRM.h"
#include "configureEncryption.h"
#include "configureGL.h"
#include "configureIE.h"
#include "configureIM.h"
#include "configureMS.h"
#include "configurePD.h"
#include "configurePO.h"
#include "configureSO.h"
#include "configureWO.h"

#include "bankAccounts.h"
#include "costCategories.h"
#include "expenseCategories.h"
#include "apAccountAssignments.h"
#include "arAccountAssignments.h"
#include "salesCategories.h"

#include "termses.h"
#include "checkFormats.h"
#include "customerTypes.h"
#include "vendorTypes.h"
#include "reasonCodes.h"
#include "bankAdjustmentTypes.h"

#include "ui_setup.h"

class setup : public XDialog, public Ui::setup
{
    Q_OBJECT

public:
    setup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~setup();

    enum Modules
    {
      All, Accounting, Sales, CRM, Manufacture, Purchase, Schedule, Inventory, Products, System
    };

public slots:
    enum SetResponse set(const ParameterList & pParams );
    void append(XTreeWidgetItem* parent, const QString &uiName, const QString &title, const QString &privileges, const QString &method = QString());
    void apply();
    void languageChange();
    void populate(int module = All);
    void save(bool close = true);

signals:
    void saving();

private slots:
    void setCurrentIndex(XTreeWidgetItem* item);

private:
    int                         _mode;
    QString                     _module;
    QMap<QString, int>          _idxmap;
    QMap<QString, QString>      _methodMap;

    configureCC*          _configCC;
    configureCRM*         _configCRM;
    configureEncryption*  _configEncr;
    configureGL*          _configGL;
    configureIE*          _configIE;
    configureIM*          _configIM;
    configureMS*          _configMS;
    configurePD*          _configPD;
    configurePO*          _configPO;
    configureSO*          _configSO;
    configureWO*          _configWO;

    costCategories*       _costCategories;
    expenseCategories*    _expenseCategories;
    apAccountAssignments* _apAccountAssignments;
    arAccountAssignments* _arAccountAssignments;
    salesCategories*      _salesCategories;

    bankAccounts*         _bankAccounts;
    termses*              _termses;
    checkFormats*         _checkFormats;
    customerTypes*        _customerTypes;
    vendorTypes*          _vendoryTypes;
    reasonCodes*          _reasonCodes;
    bankAdjustmentTypes*  _bankAdjustmentTypes;
};

#endif // SETUP_H
