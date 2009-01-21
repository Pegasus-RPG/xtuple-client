/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOOITEMLIST_H
#define BOOITEMLIST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_booItemList.h"

class booItemList : public XDialog, public Ui::booItemList
{
    Q_OBJECT

public:
    booItemList(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~booItemList();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sClose();
    virtual void sClear();
    virtual void sSelect();
    virtual void sFillList();

protected slots:
    virtual void languageChange();


private:
	int _booitemseqid;

};

#endif // BOOITEMLIST_H
