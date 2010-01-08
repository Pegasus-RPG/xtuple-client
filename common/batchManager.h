/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BATCHMANAGER_H
#define BATCHMANAGER_H

#include <QMainWindow>
#include <QSqlDatabase>

#include "ui_batchManager.h"

class batchManager : public QMainWindow, public Ui::batchManager
{
    Q_OBJECT

public:
    batchManager(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~batchManager();

    virtual void setDatabase( QSqlDatabase db );

public slots:
    virtual void sReschedule();
    virtual void sCancel();
    virtual void sView();
    virtual void setViewOtherEvents( bool viewOtherEvents );

protected slots:
    virtual void languageChange();

    virtual void sPopulateMenu( QMenu * );
    virtual void sFillList();


private:
    QTimer *_timer;
    QSqlDatabase _db;

};

#endif // BATCHMANAGER_H
