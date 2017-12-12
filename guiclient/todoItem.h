/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TODOITEM_H
#define TODOITEM_H

#include "applock.h"
#include "guiclient.h"
#include "xdialog.h"
#include "parameter.h"

#include "ui_todoItem.h"

class todoItem : public XDialog, public Ui::todoItem
{
    Q_OBJECT

public:
    todoItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);

    static bool userHasPriv(const int = cView, const int = 0);

    Q_INVOKABLE virtual int id();

    virtual SetResponse set(const ParameterList & pParams );
    virtual void	languageChange();

protected slots:
    virtual void	sClose();
    virtual void        setViewMode();
    virtual void	sHandleIncident();
    virtual void	sPopulate();
    virtual void	sSave();

    virtual void        setVisible(bool);

private:
    int _mode;
    int	_todoitemid;
    bool _close;
    AppLock _lock;
};

#endif // TODOITEM_H
