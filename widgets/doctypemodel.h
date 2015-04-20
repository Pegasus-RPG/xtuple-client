/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DOCTYPEMODEL_H
#define DOCTYPEMODEL_H

#include <QAbstractTableModel>

class DocTypeModelPrivate;

class DocTypeModel : public QAbstractTableModel
{
  public:
    static DocTypeModel &docTypeModel();
    DocTypeModel();

  public slots:
    ~DocTypeModel();

    virtual int      columnCount(const QModelIndex &parent = QModelIndex())     const;
    virtual QVariant data(const QModelIndex&index, int role = Qt::DisplayRole)  const;
    virtual Qt::ItemFlags flags(const QModelIndex &index)                       const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)   const;
    virtual int      rowCount(const QModelIndex &parent = QModelIndex())        const;
    virtual bool     setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  protected:

  private:
    DocTypeModelPrivate *_p;
};

#endif // DOCTYPEMODEL_H
