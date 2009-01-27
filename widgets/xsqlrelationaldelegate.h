/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _XSQLRELATIONALDELEGATE_H_
#define _XSQLRELATIONALDELEGATE_H_

#include <QSqlRelationalDelegate>

class QWidget;

class XSqlRelationalDelegate : public QSqlRelationalDelegate
{
  Q_OBJECT

  public:
    XSqlRelationalDelegate(QObject *parent = 0);
    ~XSqlRelationalDelegate();

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void     setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void     setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif
