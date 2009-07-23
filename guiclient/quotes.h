/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef QUOTES_H
#define QUOTES_H

#include "guiclient.h"
#include "xwidget.h"
#include "ui_quotes.h"

class quotes : public XWidget, public Ui::quotes
{
    Q_OBJECT

public:
    quotes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~quotes();
    
    virtual bool checkSitePrivs(int orderid);

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulateMenu( QMenu * );
    virtual void sPrint();
    virtual void sConvert();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();

signals:
    void finishedPrinting(int);

protected slots:
    virtual void languageChange();
};

#endif // QUOTES_H
