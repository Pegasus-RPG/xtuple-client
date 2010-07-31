/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its postSubLedger.
 */

#ifndef POSTSUBLEDGER_H
#define POSTSUBLEDGER_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_postSubLedger.h"

class postSubLedger : public XDialog, public Ui::postSubLedger
{
    Q_OBJECT

public:
    postSubLedger(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~postSubLedger();

    QPushButton* _post;
    QPushButton* _query;
    QPushButton* _selectAll;

public slots:
    void sPost();
    void sFillList();
    void sHandlePreview();
    void sHandleSelection();

protected slots:
    void languageChange();
};

#endif // POSTSUBLEDGER_H
