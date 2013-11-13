#ifndef ASSESSFINANCECHARGES_H
#define ASSESSFINANCECHARGES_H

/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "guiclient.h"
#include "xwidget.h"
#include "ui_assessFinanceCharges.h"

class assessFinanceCharges : public XWidget, public Ui::assessFinanceCharges
{
    Q_OBJECT

public:
    assessFinanceCharges(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~assessFinanceCharges();

private:
    Ui::assessFinanceCharges *ui;
    QStringList tableHeader;
    QString rstrip(const QString&);
    void postFinanceCharge(QString, QString);

public slots:

private slots:
    void cellSelected(int nRow, int nCol);
    void markAllInvoices_clicked();
    void unmarkAllInvoice_clicked();
    void assessCharges_clicked();
    void filterCharges_clicked();
};

#endif // ASSESSFINANCECHARGES_H
