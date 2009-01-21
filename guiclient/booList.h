/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOOLIST_H
#define BOOLIST_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_booList.h"

class booList : public XWidget, public Ui::booList
{
    Q_OBJECT

public:
    booList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~booList();

public slots:
    virtual void sCopy();
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList( int pItemid, bool pLocal );
    virtual void sFillList();
    virtual void sPrint();
    virtual void sHandleButtons();
    virtual void sSearch( const QString & pTarget );
    virtual void sPopulateMenu( QMenu *, QTreeWidgetItem * );

protected slots:
    virtual void languageChange();

};

#endif // BOOLIST_H
