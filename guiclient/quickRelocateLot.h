/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 2013 Specter Vision Solutions, Inc.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef QUICKRELOCATELOT_H
#define QUICKRELOCATELOT_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_quickRelocateLot.h"
#include "lotSerialUtils.h"

class quickRelocateLot : public XDialog, public Ui_quickRelocateLot
{
    Q_OBJECT

public:
    quickRelocateLot(QWidget *parent, const char *name = 0, bool modal = false, Qt::WFlags f1 = 0);
    ~quickRelocateLot();

public slots:
    void sPost();

protected slots:
    virtual void languageChange();
};

#endif // QUICKRELOCATELOT_H
