/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "doctypemodel.h"

static DocTypeModel docTypeModelSingleton;

class DocTypeModelPrivate {
  public:
    DocTypeModel *_model;

    DocTypeModelPrivate(DocTypeModel *model)
      : _model(model)
    {
    }


};

DocTypeModel::DocTypeModel()
  : QAbstractTableModel()
{
  _p = new DocTypeModelPrivate(this);
}

DocTypeModel::~DocTypeModel()
{
  delete _p;
  _p = 0;
}

DocTypeModel &DocTypeModel::docTypeModel()
{
  if (docTypeModelSingleton.rowCount() <= 0) 
  {
    ; // populate the model
  }

  return docTypeModelSingleton;
}

int DocTypeModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 0;
}

QVariant DocTypeModel::data(const QModelIndex&index, int role) const
{
  Q_UNUSED(index); Q_UNUSED(role);
  return QVariant();
}

Qt::ItemFlags DocTypeModel::flags(const QModelIndex &index) const
{
  Q_UNUSED(index);
  return Qt::ItemIsEditable; // consider: Qt::ItemIsSelectable Qt::ItemIsEnabled
}

QVariant DocTypeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(section); Q_UNUSED(orientation); Q_UNUSED(role);
  return QVariant();
}

int DocTypeModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 0;
}

bool DocTypeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Q_UNUSED(index); Q_UNUSED(value); Q_UNUSED(role);
  return false;
}

