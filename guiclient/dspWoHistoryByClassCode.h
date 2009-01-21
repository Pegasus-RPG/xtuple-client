/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPWOHISTORYBYCLASSCODE_H
#define DSPWOHISTORYBYCLASSCODE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspWoHistoryByClassCode.h"

class dspWoHistoryByClassCode : public XWidget, public Ui::dspWoHistoryByClassCode
{
    Q_OBJECT

public:
    dspWoHistoryByClassCode(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspWoHistoryByClassCode();

    virtual bool checkParameters();

public slots:
    virtual void sPrint();
    virtual void sView();
    virtual void sEdit();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * );
    virtual void sHandleCosts( bool pShowCosts );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPWOHISTORYBYCLASSCODE_H
