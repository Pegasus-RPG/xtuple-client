/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPITEMSBYCLASSCODE_H
#define DSPITEMSBYCLASSCODE_H

#include "xwidget.h"

#include "ui_dspItemsByClassCode.h"

class dspItemsByClassCode : public XWidget, public Ui::dspItemsByClassCode
{
    Q_OBJECT

public:
    dspItemsByClassCode(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspItemsByClassCode();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sEdit();
    virtual void sEditBOM();
    virtual void sViewBOM();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool pLocal );

protected slots:
    virtual void languageChange();

};

#endif // DSPITEMSBYCLASSCODE_H
