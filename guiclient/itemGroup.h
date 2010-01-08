/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMGROUP_H
#define ITEMGROUP_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_itemGroup.h"

class itemGroup : public XWidget, public Ui::itemGroup
{
    Q_OBJECT

public:
    itemGroup(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemGroup();

    virtual void init();

public slots:
    virtual enum SetResponse set( ParameterList & pParams );
    virtual void sCheck();
    virtual void sClose();
    virtual void sSave();
    virtual void sDelete();
    virtual void sNew();
    virtual void sFillList();
    virtual void populate();
    virtual void dragEnterEvent( QDragEnterEvent * pEvent );
    virtual void dropEvent( QDropEvent * pEvent );

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _itemgrpid;

};

#endif // ITEMGROUP_H
