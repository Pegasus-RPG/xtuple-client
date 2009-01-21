/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITY_H
#define OPPORTUNITY_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_opportunity.h"

class opportunity : public XDialog, public Ui::opportunity
{
    Q_OBJECT

public:
    opportunity(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~opportunity();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sCancel();
    virtual void sSave();
    virtual bool save(bool);
    virtual void sDeleteTodoItem();
    virtual void sEditTodoItem();
    virtual void sFillTodoList();
    virtual void sHandleTodoPrivs();
    virtual void sNewTodoItem();
    virtual void sPopulateTodoMenu(QMenu*);
    virtual void sViewTodoItem();
    virtual void sFillCharList();
    virtual void sNewCharacteristic();
    virtual void sEditCharacteristic();
    virtual void sDeleteCharacteristic();

protected slots:
    virtual void languageChange();

private:
    int		_opheadid;
    int		_mode;
    int		_myUsrId;
    bool	_saved;
};

#endif // OPPORTUNITY_H
