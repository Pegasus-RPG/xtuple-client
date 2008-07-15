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

#include "screen.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>

#include <openreports.h>

Screen::Screen(QWidget *parent) : 
  QWidget(parent)
{
  _keyColumns=1;
  _searchType=Query;
  _shown=false;
  
  _model = new XSqlTableModel;
  _mapper = new XDataWidgetMapper;
  
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
}

Screen::~Screen()
{
  if (isDirty())
    if (QMessageBox::question(this, tr("Unsaved work"),
                                   tr("Would you like to save your work so you do not lose your changes?"),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No ) == QMessageBox::Yes)
      save();
}

/* When the widget is first shown, set up table mappings if they exist*/
void Screen::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;
    setTable(_schemaName, _tableName);

    // now set the sort order
    for (int i = 0; i < _model->columnCount(); i++)
    { 
      if (_model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString() == _sortColumn)
      {
        _model->setSort(i, Qt::AscendingOrder);
        break;
      }
    }
  }
  QWidget::showEvent(event);
}

Screen::Modes Screen::mode()
{
  return _mode;
}

Screen::SearchTypes Screen::searchType()
{
  return _searchType;
}

bool Screen::isDirty()
{
  for (int i = 0; i < _model->columnCount()-1; i++)
    if (_model->isDirty(_model->index(_mapper->currentIndex(),i)))
      return true;
  return false;
}

void Screen::insert()
{
  _model->insertRows(_model->rowCount(),1);
  _mapper->toLast();
  _mapper->clear();
}

void Screen::save()
{
  int i=_mapper->currentIndex();
  _mapper->submit();
  _model->submitAll();
  _mapper->setCurrentIndex(i);
  if (_model->lastError().type() != QSqlError::NoError && _model->lastError().driverText() != "No Fields to update")
  {
    QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                          tr("Error saving %1 at %2::%3\n\n%4")
                          .arg(_tableName).arg(__FILE__).arg(__LINE__)
                          .arg(_model->lastError().databaseText()));
    return;
  }
  if (_mode==New)
    insert();
  emit saved(true);
}

void Screen::search(QString criteria)
{
  QString filter;
  if (criteria.stripWhiteSpace().length())
  {
    QStringList parts;
    for (int i = 0; i < _model->columnCount(); i++)
    { 
      if (_model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString().length())
        parts.append("(CAST(" +
                     _model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString() +
                     " AS TEXT) ~* '" + criteria + "')");
    }
    filter=parts.join(" OR ");
  }
  setFilter(filter);
  select();
}

void Screen::select()
{
  _model->select();
  if (_model->rowCount())
  {
    _mapper->toFirst();
    emit newModel(_model);
  }
}


void Screen::setMode(Modes p)
{
  if (p==New)
  {
    _mode=New;
    insert();
  }
  else if (p==Edit)
    _mode=Edit;
  else
    _mode=View;

  for (int i = 0; _mapper->model() && i < _mapper->model()->columnCount(); i++)
    if (_mapper->mappedWidgetAt(i))
      _mapper->mappedWidgetAt(i)->setEnabled(_mode != View);
}

void Screen::setSearchType(SearchTypes p)
{
  _searchType=p;
}

void Screen::setSortColumn(QString p)
{
  _sortColumn = p;
}

void Screen::setCurrentIndex(int index)
{
   _mapper->setCurrentIndex(index);
}

void Screen::toNext()
{
   _mapper->toNext();
}

void Screen::toPrevious()
{
  _mapper->toPrevious(); 
}

void Screen::setSchemaName(QString schema)
{ 
  _schemaName = schema;  
}

void Screen::setTableName(QString table)
{
  _tableName = table;
}

/* Pass in a schema name and a table name.  If schema name is blank, it will be ignored.
     If there is a table name to set, data widget mappings will be set as well. */
void Screen::setTable(QString schema, QString table)
{
  if (table.length())
  {
    QString tablename="";
    if (schema.length())
      tablename=schema + ".";
    tablename+=table;
    if (_model->tableName() != tablename)
    {
      _model->setTable(tablename,_keyColumns);
      setDataWidgetMapper(_model);
      emit newModel(_model);
    }
  }
}

void Screen::setDataWidgetMapper(QSqlTableModel *p)
{
  _mapper->setModel(p);
  emit newDataWidgetMapper(_mapper);
}
