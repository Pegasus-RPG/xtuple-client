/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GLTRANSACTION_H
#define GLTRANSACTION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_glTransaction.h"

class glTransaction : public XDialog, public Ui::glTransaction
{
    Q_OBJECT

public:
    glTransaction(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~glTransaction();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sPost();
    virtual void clear();
    virtual void populate();
    virtual void setupDocuments();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _glsequence;
    bool _captive;
    int _placeholder;

};

#endif // GLTRANSACTION_H
