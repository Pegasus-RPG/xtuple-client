/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BATCHITEM_H
#define BATCHITEM_H

#include <QDialog>
#include <QSqlDatabase>

#include "ui_batchItem.h"

class batchItem : public QDialog, public Ui::batchItem
{
    Q_OBJECT

public:
    batchItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~batchItem();

    static int Reschedule( int pBatchid, QWidget * pParent );
    static int Reschedule( int pBatchid, QWidget * pParent, QSqlDatabase pDb );

public slots:
    virtual void sSave();
    virtual void populate();

protected:

protected slots:
    virtual void languageChange();

private:
    int _batchid;
    int _mode;
    QSqlDatabase _db;

};

#endif // BATCHITEM_H
