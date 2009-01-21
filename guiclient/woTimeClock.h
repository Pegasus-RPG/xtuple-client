/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WOTIMECLOCK_H
#define WOTIMECLOCK_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_woTimeClock.h"

class woTimeClock : public XWidget, public Ui::woTimeClock
{
    Q_OBJECT

public:
    woTimeClock(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~woTimeClock();

    virtual bool close( bool alsoDelete );
    virtual int callPostProduction();
    virtual int callPostOperations(int wotc_id);

public slots:
    virtual SetResponse set( ParameterList & pParams );
    virtual void sClockIn();
    virtual void sClockOut();
    virtual void sPostProduction();
    virtual void sScrap();
    virtual void clear();
    virtual void sCheckValid();
    virtual void sHandleButtons();
    virtual void sSetTimer();
    virtual void sPopulateWooper();
    virtual void sWooperScanned(int);

protected slots:
    virtual void languageChange();

private:
    bool _captive;

    void init();

};

#endif // WOTIMECLOCK_H
