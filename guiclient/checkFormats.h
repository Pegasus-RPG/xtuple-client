/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHECKFORMATS_H
#define CHECKFORMATS_H

#include "xwidget.h"

#include "ui_checkFormats.h"

class checkFormats : public XWidget, public Ui::checkFormats
{
    Q_OBJECT

public:
    checkFormats(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~checkFormats();

public slots:
    virtual void sFillList();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();

};

#endif // CHECKFORMATS_H