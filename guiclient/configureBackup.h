/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGUREBACKUP_H
#define CONFIGUREBACKUP_H

#include "xdialog.h"

#include "ui_configureBackup.h"

class configureBackup : public XDialog, public Ui::configureBackup
{
    Q_OBJECT

public:
    configureBackup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureBackup();

    virtual void init();

protected slots:
    virtual void languageChange();

    virtual void sSave();


};

#endif // CONFIGUREBACKUP_H
