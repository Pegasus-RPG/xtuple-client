/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xsqlrelationaldelegate.h"

#include <QSqlField>
#include <QSqlRelationalDelegate>

#include "addresscluster.h"
// #include "calendarTools.h"
// #include "cmcluster.h"
// #include "contactcluster.h"
// #include "crmacctcluster.h"
#include "custcluster.h"
// #include "datecluster.h"
// #include "deptcluster.h"
// #include "empcluster.h"
// #include "empgroupcluster.h"
// #include "expensecluster.h"
// #include "glcluster.h"
// #include "imagecluster.h"
// #include "incidentcluster.h"
// #include "invoiceCluster.h"
// #include "itemcluster.h"
// #include "lotserialcluster.h"
// #include "opportunitycluster.h"
// #include "ordercluster.h"
// #include "plCluster.h"
// #include "plannedOrderList.h"
// #include "pocluster.h"
// #include "projectcluster.h"
// #include "racluster.h"
// #include "revisioncluster.h"
// #include "shiftcluster.h"
// #include "shipmentcluster.h"
// #include "shiptocluster.h"
// #include "socluster.h"
// #include "tocluster.h"
// #include "usernamecluster.h"
// #include "vendorcluster.h"
// #include "vendorgroup.h"
// #include "warehouseCluster.h"
// #include "wcombobox.h"
// #include "wocluster.h"
// #include "womatlcluster.h"
// #include "workcentercluster.h"

#define DEBUG false

XSqlRelationalDelegate::XSqlRelationalDelegate(QObject *parent)
  : QSqlRelationalDelegate(parent)
{
  if (DEBUG) qDebug("XSqlRelationalDelegate::XSqlRelationalDelegate(%p)",
                    parent);
}

XSqlRelationalDelegate::~XSqlRelationalDelegate()
{
}

QWidget *XSqlRelationalDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
  if (DEBUG) qDebug("XSqlRelationalDelegate::createEditor(%p, option, index)",
                    parent);
  const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());


  QWidget *editor = 0;
  QString colname = sqlModel->record().field(index.column()).name();
  if (DEBUG) qDebug("createEditor operating on colname %s", qPrintable(colname));
  /* TODO: how do we make an AddressCluster full-size?
     the following currently scales the cluster down to fit inside a single
     cell in an xtreeview
  if (colname == "addr_number")
  {
    if (DEBUG) qDebug("createEditor creating AddressCluster");
    AddressCluster *widget = new AddressCluster(parent);
    widget->adjustSize();
    editor = widget;
  }
  else
  */
  if (colname == "cust_number" || colname == "customer_number")
  {
    if (DEBUG) qDebug("createEditor creating CLineEdit");
    CLineEdit *widget = new CLineEdit(parent);
    widget->setType(CLineEdit::AllCustomers);
    editor = widget;
  }
  else
  {
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    if (!childModel)
        return QItemDelegate::createEditor(parent, option, index);

    if (DEBUG) qDebug("createEditor defaulting to QComboBox");
    QComboBox *widget = new QComboBox(parent);
    widget->setModel(childModel);
    widget->setModelColumn(childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn()));
    editor = widget;
  }

  editor->installEventFilter(const_cast<XSqlRelationalDelegate *>(this));
  return editor;
}

void XSqlRelationalDelegate::setEditorData(QWidget *editor,
                                           const QModelIndex &index) const
{
  if (DEBUG) qDebug("setEditorData(%p = %s, index)",
                    editor, editor->metaObject()->className());
  const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());

  if (sqlModel)
  {
    if (editor->inherits("AddressCluster"))
    {
      AddressCluster *widget = qobject_cast<AddressCluster *>(editor);
      if (widget)
      {
        widget->setNumber(sqlModel->data(index).toString());
        return;
      }
    }
    else if (editor->inherits("CLineEdit"))
    {
      CLineEdit *widget = qobject_cast<CLineEdit *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index).toInt());
        return;
      }
    }
    else if (editor->inherits("QComboBox"))
    {
      QComboBox *widget = qobject_cast<QComboBox *>(editor);
      if (widget)
      {
        widget->setCurrentIndex(widget->findText(sqlModel->data(index).toString()));
        return;
      }
    }
  }

  // default case - if we haven't already returned then do this
  QItemDelegate::setEditorData(editor, index);
}

void XSqlRelationalDelegate::setModelData(QWidget *editor,
                                          QAbstractItemModel *model,
                                          const QModelIndex &index) const
{
  if (DEBUG) qDebug("setModelData(%p = %s, model, index)",
                    editor, editor->metaObject()->className());
  if (!index.isValid())
    return;

  QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
  if (sqlModel)
  {
    if (editor->inherits("AddressCluster"))
    {
      AddressCluster *widget = qobject_cast<AddressCluster *>(editor);
      if (widget)
      {
        widget->save(AddressCluster::CHANGEONE);
        sqlModel->setData(index, widget->number(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),     Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("CLineEdit"))
    {
      CLineEdit *widget = qobject_cast<CLineEdit *>(editor);
      if (widget)
      {
        if (DEBUG) qDebug("setModelData CLineEdit %d %s",
                          widget->id(), qPrintable(widget->text()));
        sqlModel->setData(index, widget->text(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),   Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("QComboBox"))
    {
      QComboBox *widget = qobject_cast<QComboBox *>(editor);
      QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;

      if (childModel && widget)
      {
        int currentItem = widget->currentIndex();
        int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
        int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
        sqlModel->setData(index, childModel->data(childModel->index(currentItem,
                                                                    childColIndex),
                                                  Qt::DisplayRole), Qt::DisplayRole);
        sqlModel->setData(index, childModel->data(childModel->index(currentItem,
                                                                    childEditIndex),
                                                  Qt::EditRole), Qt::EditRole);
        return;
      }
    }
  }

  // default case - if we haven't already returned then do this
  QItemDelegate::setModelData(editor, model, index);
}
