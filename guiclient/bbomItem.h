/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BBOMITEM_H
#define BBOMITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_bbomItem.h"

class bbomItem : public XDialog, public Ui::bbomItem
{
    Q_OBJECT

public:
    bbomItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~bbomItem();

    virtual void init();

public slots:
    virtual enum SetResponse set( ParameterList & pParams );
    virtual void sSave();
    virtual void sHandleItemType( const QString & pItemType );
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _bbomitemid;
    int _itemid;

};

#endif // BBOMITEM_H
