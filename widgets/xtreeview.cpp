/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

// Copyright (c) 2006-2008, OpenMFG, LLC

#include "xtreeview.h"
#include "xsqltablemodel.h"
//#include "xuiloader.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlIndex>
#include <QBuffer>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QStack>
#include <QModelIndex>
#include <QScriptEngine>

XTreeView::XTreeView(QWidget *parent) : 
  QTreeView(parent)
{
  _keyColumns=1;
  
  setAlternatingRowColors(true);

  _mapper = new XDataWidgetMapper(this);
  _model.setEditStrategy(QSqlTableModel::OnManualSubmit);
}

bool XTreeView::isRowHidden(int row)
{
  return QTreeView::isRowHidden(row,model()->parent(model()->index(row,0)));
}

bool XTreeView::throwScriptException(const QString &message)
{
   QObject *ancestor = this;
   QScriptEngine *engine = 0;
   for ( ; ancestor; ancestor = ancestor->parent())
   {
     engine = ancestor->findChild<QScriptEngine*>();
     if (engine)
       break;
   } 
   if (engine)
   {
      QScriptContext *ctx = engine->currentContext();
      ctx->throwError(message);
      return true;
   }
   return false;
}

int XTreeView::size()
{
  return model()->rowCount(model()->parent(model()->index(0,0)));
}

QVariant XTreeView::value(int row, int column)
{
  return model()->data(model()->index(row,column));
}

QVariant XTreeView::selectedValue(int column)
{
  QModelIndexList selected=selectedIndexes();
  return value(selected.first().row(),column);
}

void XTreeView::select()
{
  setTable();
  _model.select();
}

void XTreeView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
  if (!selected.indexes().isEmpty())
  {
    emit rowSelected(selected.indexes().first().row());
    emit valid(true);
  }
  else
    emit valid(FALSE);
  QTreeView::selectionChanged(selected, deselected);
}

void XTreeView::insert()
{ 
  int row=_model.rowCount();
  _model.insertRows(row,1);
  //Set default values for foreign keys
  for (int i = 0; i < _idx.count(); ++i)
    _model.setData(_model.index(row,i),_idx.value(i));
  selectRow(row);
}

void XTreeView::populate(int p)
{ 
  XSqlTableModel *t=static_cast<XSqlTableModel*>(_mapper->model());
  if (t)
  {
    _idx=t->primaryKey();

    for (int i = 0; i < _idx.count(); ++i)
    {
      _idx.setValue(i, t->data(t->index(p,i)));
      setColumnHidden(i,true);
    }
    
    QString clause = QString(_model.database().driver()->sqlStatement(QSqlDriver::WhereStatement, t->tableName(),_idx, false)).mid(6);
    _model.setFilter(clause);
    if (!_model.query().isActive())
      _model.select();
  }
}

void XTreeView::removeRows(int row, int count)
{
  _model.removeRows(row, count);
}

void XTreeView::removeSelected()
{
  QModelIndexList selected=selectedIndexes();
  for (int i = 0; i < selected.count(); i++)
  {
    _model.removeRows(selected.value(i).row(), 1);
    setRowHidden(selected.value(i).row(),selected.value(i).parent(),true);
  }
}

void XTreeView::revertAll()
{
  _model.revertAll();
  if (_mapper)
    populate(_mapper->currentIndex());
}

void XTreeView::save()
{ 
  if(!_model.submitAll())
  {
    if(!throwScriptException(_model.lastError().databaseText()))
          QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                            tr("Error saving %1 at %2::%3\n\n%4")
                            .arg(_tableName).arg(__FILE__).arg(__LINE__)
                            .arg(_model.lastError().databaseText()));
  }
  else
    emit saved();
}

void XTreeView::selectRow(int index)
{
  selectionModel()->select(QItemSelection(model()->index(index,0),QTreeView::model()->index(index,0)),
                                        QItemSelectionModel::ClearAndSelect |
                                        QItemSelectionModel::Rows);
}

void XTreeView::setDataWidgetMap(XDataWidgetMapper* mapper)
{  
  setTable();
  _mapper=mapper; 
}

void XTreeView::setTable()
{
  if (_model.tableName() == _tableName)
    return;
      
  if (!_tableName.isEmpty())
  {
    QString tablename=_tableName;
    if (!_schemaName.isEmpty())
      tablename = _schemaName + "." + tablename;
    _model.setTable(tablename,_keyColumns);
    
    setModel(&_model);
  }
}

void XTreeView::setModel(XSqlTableModel * model)
{
  QTreeView::setModel(model);
  //Set headers case proper 
  for (int i = 0; i < QTreeView::model()->columnCount(); ++i)
  {
      QString h=QTreeView::model()->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
      h=h.replace(0,1,h.left(1).toUpper());
      while (h.lastIndexOf("_") != -1)
      {
        h=h.replace(h.lastIndexOf("_")+1,1,h.mid(h.lastIndexOf("_")+1,1).toUpper());
        h=h.replace("_"," ");
      }
      QTreeView::model()->setHeaderData(i,Qt::Horizontal,h);
  }
  emit newModel(model);
}

void XTreeView::setValue(int row, int column, QVariant value)
{
  _model.setData(_model.index(row,column), value);
}

