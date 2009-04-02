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

#include <QDesktopWidget>
#include <QDialog>
#include <QMainWindow>
#include <QStandardItemEditorCreator>
#include <QSqlField>
#include <QSqlRelationalDelegate>

#include "addresscluster.h"
#include "contactcluster.h"
#include "crmacctcluster.h"
#include "custcluster.h"
#include "datecluster.h"
#include "deptcluster.h"
#include "empcluster.h"
#include "empgroupcluster.h"
#include "expensecluster.h"
#include "glcluster.h"
#include "imagecluster.h"
#include "incidentcluster.h"
#include "invoiceCluster.h"
#include "itemcluster.h"
#include "lotserialcluster.h"
#include "opportunitycluster.h"
#include "ordercluster.h"
#include "plCluster.h"
#include "projectcluster.h"
#include "revisioncluster.h"
#include "shiftcluster.h"
#include "shipmentcluster.h"
#include "shiptocluster.h"
#include "usernamecluster.h"
#include "vendorcluster.h"
#include "wocluster.h"
#include "workcentercluster.h"

#define DEBUG true

// TODO: move clusters which fall outside the clip region (e.g. address cluster
//       on the last line of an xtreeview
// TODO: make this work with api views (right now it only works well in tables)

/* This subclass of QSqlRelationalDelegate takes the following approach:

   When creating an editor, check the name of the column from which
   the data were selected. If the name matches one of a set of known
   patterns, instantiate a custom xTuple widget to handle the
   editing. In general these are subclasses of XLineEdit, so ^S and
   ^L are available for listing possible values without the space
   overhead of the ellipsis button.

   In many cases where you might think a cluster would be appropriate
   the code uses the Qt default implementation of comboboxes. This
   is because there's no benefit to using a cluster that requires
   too much initialization (e.g. ShiptoCluster requires a customer
   id before it'll show anything) and there's no benefit to using
   an XComboBox over a QComboBox.

   An alternate implementation could do the following:
     Add to the constructor - 
     for each different custom widget
        editorfactory->registerEditor(QVariant::UserType + SOMECONSTANT,
                         new QStandardItemEditorCreator<SOMECUSTOMWIDGET>())

     Remove most of createEditor.

     Override select():
        QSqlRelationalDelegate::select();
        for each column
          find SOMECONSTANT that corresponds to the data in the column
          for each modelIndex in the current column
            create a new QVariant(value = modelIndex.data(),
                                  type = UserType + SOMECONSTANT)
            modelIndexsetData(the new QVariant);

   On the other hand, none of the xTuple custom widgets are written
   to take advantage of Model/View so setModelData and setEditorData
   would still need to be huge if/elseif blocks. There would be no
   code maintenance benefit.

 */

XSqlRelationalDelegate::XSqlRelationalDelegate(QObject *parent)
  : QSqlRelationalDelegate(parent)
{
  QItemEditorFactory *editorfactory = new QItemEditorFactory();
  if (editorfactory)
  {
    editorfactory->registerEditor(QVariant::Date,
                                  new QStandardItemEditorCreator<DLineEdit>());
    setItemEditorFactory(editorfactory);
  }
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
  // address the _fkeymap in XTreeView::setRelations in alphabetical order
  // but only where there is a better alternative than the default combobox
  if (colname.endsWith("accnt_number") ||
      colname.endsWith( "account_number"))
  {
    GLCluster *widget = new GLCluster(parent);
    editor = widget;
  }
  else if (colname.endsWith("addr_number"))
  {
    AddressCluster *widget = new AddressCluster(parent);
    widget->setMode(AddressCluster::Select);
    editor = widget;
  }
  else if (colname.endsWith("cntct_number"))
  {
    ContactCluster *widget = new ContactCluster(parent);
    widget->setMinimalLayout(true);
    widget->setMode(ContactCluster::Select);
    editor = widget;
  }
  else if (colname.endsWith("cohead_number"))
  {
    OrderLineEdit *widget = new OrderLineEdit(parent);
    widget->setAllowedTypes(OrderLineEdit::Sales);
    editor = widget;
  }
  else if (colname.endsWith("crmacct_number"))
  {
    CRMAcctLineEdit *widget = new CRMAcctLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("cust_number") ||
           colname.endsWith("customer_number"))
  {
    CLineEdit *widget = new CLineEdit(parent);
    widget->setType(CLineEdit::AllCustomers);
    editor = widget;
  }
  else if (colname.endsWith("dept_number"))
  {
    DeptClusterLineEdit *widget = new DeptClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("emp_code"))
  {
    EmpClusterLineEdit *widget = new EmpClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("empgrp_name"))
  {
    EmpGroupClusterLineEdit *widget = new EmpGroupClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("expcat_code"))
  {
    ExpenseLineEdit *widget = new ExpenseLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("image_name"))
  {
    ImageClusterLineEdit *widget = new ImageClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("incdt_number"))
  {
    IncidentClusterLineEdit *widget = new IncidentClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("invchead_invcnumber"))
  {
    InvoiceClusterLineEdit *widget = new InvoiceClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("item_number"))
  {
    ItemLineEdit *widget = new ItemLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("ls_number"))
  {
    LotserialLineEdit *widget = new LotserialLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("ophead_name"))
  {
    OpportunityClusterLineEdit *widget = new OpportunityClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("planord_number"))
  {
    PlanOrdCluster *widget = new PlanOrdCluster(parent);
    editor = widget;
  }
  else if (colname.endsWith("pohead_number"))
  {
    OrderLineEdit *widget = new OrderLineEdit(parent);
    widget->setAllowedTypes(OrderLineEdit::Purchase);
    editor = widget;
  }
  else if (colname.endsWith("prj_number"))
  {
    ProjectLineEdit *widget = new ProjectLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("prospect_number"))
  {
    CLineEdit *widget = new CLineEdit(parent);
    widget->setType(CLineEdit::AllProspects);
    editor = widget;
  }
  else if (colname.endsWith("rahead_number"))
  {
    OrderLineEdit *widget = new OrderLineEdit(parent);
    widget->setAllowedTypes(OrderLineEdit::Return);
    editor = widget;
  }
  else if (colname.endsWith("rev_number"))
  {
    RevisionLineEdit *widget = new RevisionLineEdit(parent);
    widget->setMode(RevisionLineEdit::Use);
    editor = widget;
  }
  else if (colname.endsWith("shift_number"))
  {
    ShiftClusterLineEdit *widget = new ShiftClusterLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("shiphead_number"))
  {
    ShipmentClusterLineEdit *widget = new ShipmentClusterLineEdit(parent);
    editor = widget;
  }
  /* cannot use the ShiptoCluster because it requires a customer id!
     use the default combobox
  else if (colname.endsWith("shipto_name"))
  {
    ShiptoCluster *widget = new ShiptoCluster(parent);
    editor = widget;
  }
  */
  else if (colname.endsWith("tohead_number"))
  {
    OrderLineEdit *widget = new OrderLineEdit(parent);
    widget->setAllowedTypes(OrderLineEdit::Transfer);
    editor = widget;
  }
  else if (colname.endsWith("vend_number"))
  {
    VendorLineEdit *widget = new VendorLineEdit(parent);
    editor = widget;
  }
  else if (colname.endsWith("wo_number"))
  {
    WoCluster *widget = new WoCluster(parent);
    editor = widget;
  }
  else if (colname.endsWith("wrkcnt_code"))
  {
    WorkCenterLineEdit *widget = new WorkCenterLineEdit(parent);
    editor = widget;
  }
  else
  {
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    if (!childModel)
        return QItemDelegate::createEditor(parent, option, index);

    QComboBox *widget = new QComboBox(parent);
    widget->setModel(childModel);
    widget->setModelColumn(childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn()));
    editor = widget;
  }

  editor->installEventFilter(const_cast<XSqlRelationalDelegate *>(this));
  editor->setAutoFillBackground(true);
  return editor;
}

void XSqlRelationalDelegate::setEditorData(QWidget *editor,
                                           const QModelIndex &index) const
{
  const QSqlRelationalTableModel *sqlModel = qobject_cast<const QSqlRelationalTableModel *>(index.model());
  if (DEBUG) qDebug("setEditorData(%p = %s, index %s ~= %s)",
                    editor, editor->metaObject()->className(),
                    qPrintable(sqlModel->data(index,Qt::DisplayRole).toString()),
                    qPrintable(sqlModel->data(index,Qt::EditRole).toString())
                    );

  if (sqlModel)
  {
    // order of handlers:
    // 1) classes registered with the editor factory
    // 2) classes which inherit from VirtualClusterLineEdit and VirtualCluster
    // 3) everything else appears in the same order as in createEditor except
    //    where already covered by inheritance (eg Prospects <- CustCluster).
    // keep in sync with setModelData below
    if (editor->inherits("DLineEdit"))
    {
      DLineEdit *widget = qobject_cast<DLineEdit *>(editor);
      if (widget)
      {
        widget->setDate(sqlModel->data(index).toDate());
        return;
      }
    }
    else if (editor->inherits("VirtualClusterLineEdit"))
    {
      VirtualClusterLineEdit *widget = qobject_cast<VirtualClusterLineEdit *>(editor);
      if (widget)
      {
        widget->setNumber(sqlModel->data(index).toString());
        return;
      }
    }
    else if (editor->inherits("VirtualCluster"))
    {
      VirtualCluster *widget = qobject_cast<VirtualCluster *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index, Qt::EditRole).toInt());
        return;
      }
    }
    else if (editor->inherits("GLCluster"))
    {
      GLCluster *widget = qobject_cast<GLCluster *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index).toInt());
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
    else if (editor->inherits("ItemLineEdit"))
    {
      ItemLineEdit *widget = qobject_cast<ItemLineEdit *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index).toInt());
        return;
      }
    }
    else if (editor->inherits("PlanOrdCluster"))
    {
      PlanOrdCluster *widget = qobject_cast<PlanOrdCluster *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index).toInt());
        return;
      }
    }
    else if (editor->inherits("ShiptoCluster"))
    {
      ShiptoCluster *widget = qobject_cast<ShiptoCluster *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index).toInt());
        return;
      }
    }
    else if (editor->inherits("VendorLineEdit"))
    {
      VendorLineEdit *widget = qobject_cast<VendorLineEdit *>(editor);
      if (widget)
      {
        widget->setId(sqlModel->data(index).toInt());
        return;
      }
    }
    else if (editor->inherits("WoCluster"))
    {
      WoCluster *widget = qobject_cast<WoCluster *>(editor);
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
  if (DEBUG) qDebug("setModelData(%p = %s, model, index %s ~= %s)",
                    editor, editor->metaObject()->className(),
                    qPrintable(model->data(index,Qt::DisplayRole).toString()),
                    qPrintable(model->data(index,Qt::EditRole).toString()));
  if (!index.isValid())
    return;

  QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
  if (sqlModel)
  {
    // keep in sync with setEditorData above
    if (editor->inherits("DLineEdit"))
    {
      DLineEdit *widget = qobject_cast<DLineEdit *>(editor);
      if (widget)
      {
        sqlModel->setData(index, widget->date(), Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("VirtualClusterLineEdit"))
    {
      VirtualClusterLineEdit *widget = qobject_cast<VirtualClusterLineEdit *>(editor);
      if (widget)
      {
        if (DEBUG) qDebug("VirtualClusterLineEdit->number() = %s, id() = %d",
                          qPrintable(widget->text()), widget->id());
        widget->setNumber(widget->text());      // force sParse()
        sqlModel->setData(index, widget->text(), Qt::DisplayRole);
        if (widget->id() >= 0)
          sqlModel->setData(index, widget->id(), Qt::EditRole);
        else
          sqlModel->setData(index, QVariant(QVariant::Int),   Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("VirtualCluster"))
    {
      VirtualCluster *widget = qobject_cast<VirtualCluster *>(editor);
      if (widget)
      {
        if (DEBUG) qDebug("VirtualCluster->number() = %s, id() = %d",
                          qPrintable(widget->number()), widget->id());
        widget->setId(widget->id());      // force sParse()
        sqlModel->setData(index, widget->number(), Qt::DisplayRole);
        if (widget->id() >= 0)
          sqlModel->setData(index, widget->id(), Qt::EditRole);
        else
          sqlModel->setData(index, QVariant(QVariant::Int),   Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("GLCluster"))
    {
      GLCluster *widget = qobject_cast<GLCluster *>(editor);
      if (widget)
      {
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
        sqlModel->setData(index, widget->text(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),   Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("ItemLineEdit"))
    {
      ItemLineEdit *widget = qobject_cast<ItemLineEdit *>(editor);
      if (widget)
      {
        sqlModel->setData(index, widget->itemNumber(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),         Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("PlanOrdCluster"))
    {
      PlanOrdCluster *widget = qobject_cast<PlanOrdCluster *>(editor);
      if (widget)
      {
        sqlModel->setData(index, widget->number(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),     Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("ShiptoCluster"))
    {
      ShiptoCluster *widget = qobject_cast<ShiptoCluster *>(editor);
      if (widget)
      {
        sqlModel->setData(index, widget->name(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),   Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("VendorLineEdit"))
    {
      VendorLineEdit *widget = qobject_cast<VendorLineEdit *>(editor);
      if (widget)
      {
        sqlModel->setData(index, widget->text(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),   Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("WoCluster"))
    {
      WoCluster *widget = qobject_cast<WoCluster *>(editor);
      if (widget)
      {
        sqlModel->setData(index, widget->woNumber(), Qt::DisplayRole);
        sqlModel->setData(index, widget->id(),       Qt::EditRole);
        return;
      }
    }
    else if (editor->inherits("QComboBox"))
    {
      QComboBox *widget = qobject_cast<QComboBox *>(editor);
      QSqlTableModel *childModel = sqlModel ?
                                  sqlModel->relationModel(index.column()) : 0;

      if (childModel && widget)
      {
        int currentItem = widget->currentIndex();
        int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
        int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem,
                                                             childColIndex),
                                           Qt::DisplayRole), Qt::DisplayRole);
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem,
                                                             childEditIndex),
                                           Qt::EditRole), Qt::EditRole);
        return;
      }
    }
  }

  // default case - if we haven't already returned then do this
  QItemDelegate::setModelData(editor, model, index);
}

void XSqlRelationalDelegate::updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}
