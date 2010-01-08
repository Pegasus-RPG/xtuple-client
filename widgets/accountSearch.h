/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ACCOUNTSEARCH_H
#define ACCOUNTSEARCH_H

#include "widgets.h"
#include <QDialog>

#include "ui_accountSearch.h"

class ParameterList;

class XTUPLEWIDGETS_EXPORT accountSearch : public QDialog, public Ui::accountSearch
{
    Q_OBJECT

public:
    accountSearch(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~accountSearch();

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sClear();
    virtual void sFillList();
    virtual void sClose();
    virtual void sSelect();

protected slots:
    virtual void languageChange();

private:
    int _accntid;
    bool         _showExternal;
    unsigned int _typeval;

};

#endif // ACCOUNTNUMBER_H
