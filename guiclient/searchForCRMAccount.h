/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SEARCHFORCRMACCOUNT_H
#define SEARCHFORCRMACCOUNT_H

#include "guiclient.h"
#include "xwidget.h"

#include <parameter.h>

#include "crmacctcluster.h"
#include "ui_searchForCRMAccount.h"

class searchForCRMAccount : public XWidget, public Ui::searchForCRMAccount
{
    Q_OBJECT

public:
    searchForCRMAccount(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~searchForCRMAccount();

public slots:
    virtual SetResponse	set(const ParameterList &);
    virtual void	sEdit();
    virtual void	sFillList();
    virtual void	sPopulateMenu(QMenu * pMenu);
    virtual void	sView();

protected slots:
    virtual void languageChange();

protected:
    bool				_editpriv;
    CRMAcctLineEdit::CRMAcctSubtype	_subtype;
    bool				_viewpriv;

private:
    virtual void	openSubwindow(const QString &);

};

#endif // SEARCHFORCRMACCOUNT_H
