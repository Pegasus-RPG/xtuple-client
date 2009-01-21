/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BBOM_H
#define BBOM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_bbom.h"

class bbom : public XWidget, public Ui::bbom
{
    Q_OBJECT

public:
    bbom(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~bbom();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sMoveUp();
    virtual void sMoveDown();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool );
    virtual void sPopulateMenu( QMenu * );

protected slots:
    virtual void languageChange();

private:
    int _mode;

};

#endif // BBOM_H
