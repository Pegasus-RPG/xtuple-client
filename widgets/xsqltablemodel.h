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
    
    enum itemDataRole { FormatRole = (Qt::UserRole + 1),
                  EditorRole,  
                  MenuRole, /* Other roles for xtreewidget?
                  RawRole,
                  IdRole,
                  RunningSetRole,
                  RunningInitRole,
                  TotalSetRole,
                  TotalInitRole,
                  IndentRole */ 
    };
    
    enum FormatFlags { Money, Qty, Curr, Percent, Cost, QtyPer, 
      SalesPrice, PurchPrice, UOMRatio, ExtPrice, Weight
    };

    virtual void applyColumnRole(int column, int role, QVariant value);
    virtual void applyColumnRoles();
    virtual void applyColumnRoles(int row);
    virtual void setColumnRole(int column, int role, QVariant value);
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant formatValue(const QVariant &dataValue, const QVariant &formatValue) const;
    virtual bool select();
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual void setTable(const QString &tableName, int keyColumns);
    virtual void setKeys(int keyColumns);

    virtual QString selectStatement() const;
    
    private:
      QHash<QPair<QModelIndex, int>, QVariant> roles;
      QMultiHash<int, QPair<QVariant, int> > _columnRoles;
      QList<QString> _locales;
};

#endif
