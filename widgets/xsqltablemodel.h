/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XSQLTABLEMODEL_H

#define XSQLTABLEMODEL_H

#include <QSqlRelationalTableModel>
#include <QHash>
#include "widgets.h"

class XTUPLEWIDGETS_EXPORT XSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

    public:
      XSqlTableModel(QObject *parent = 0);
      ~XSqlTableModel();
    
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole);
    virtual void setTable(const QString &tableName, int keyColumns);
    virtual void setKeys(int keyColumns);

    virtual QString selectStatement() const;
    
    private:
      QHash<QPair<QModelIndex, int>, QVariant> roles;
};

#endif
