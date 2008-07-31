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
#include <QSqlIndex>
#include <QBuffer>
#include <QMessageBox>

XTreeView::XTreeView(QWidget *parent) : 
  QTreeView(parent)
{
  _keyColumns=1;
  
  setAlternatingRowColors(true);

  _mapper = new XDataWidgetMapper(this);
  _model.setEditStrategy(QSqlTableModel::OnManualSubmit);
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
    _model.select();
  }
}

void XTreeView::save()
{
  _model.submitAll();
}

void XTreeView::setDataWidgetMap(XDataWidgetMapper* mapper)
{  
  if (!_tableName.isEmpty())
  {
    QString tablename=_tableName;
    if (!_schemaName.isEmpty())
      tablename = _schemaName + "." + tablename;
    _model.setTable(tablename,_keyColumns);
    
    setModel(&_model);
    populate(mapper->currentIndex());
    _mapper=mapper;
    connect(_mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(populate(int)));
    connect(_mapper, SIGNAL(saved(bool)), this, SLOT(save()));
  }
}

void XTreeView::setModel(XSqlTableModel * model)
{
  QTreeView::setModel(model);
  //Set headers case proper 
  for (int i = 0; i < QTreeView::model()->columnCount(); ++i)
  {
      QString h=QTreeView::model()->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
      h=h.replace(0,1,h.left(1).upper());
      while (h.lastIndexOf("_") != -1)
      {
        h=h.replace(h.lastIndexOf("_")+1,1,h.mid(h.lastIndexOf("_")+1,1).upper());
        h=h.replace("_"," ");
      }
      QTreeView::model()->setHeaderData(i,Qt::Horizontal,h);
  }
}



