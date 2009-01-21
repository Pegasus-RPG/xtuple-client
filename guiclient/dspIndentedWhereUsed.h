/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINDENTEDWHEREUSED_H
#define DSPINDENTEDWHEREUSED_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_dspIndentedWhereUsed.h"

class dspIndentedWhereUsed : public XWidget, public Ui::dspIndentedWhereUsed
{
    Q_OBJECT

public:
    dspIndentedWhereUsed(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspIndentedWhereUsed();

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sViewInventoryHistory();
    virtual void sPopulateMenu( QMenu * menu );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // DSPINDENTEDWHEREUSED_H
