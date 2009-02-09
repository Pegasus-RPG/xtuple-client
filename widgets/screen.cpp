/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "screen.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QScriptContext>
#include <QScriptEngine>

#include <openreports.h>


Screen::Screen(QWidget *parent) : 
  QWidget(parent)
{
  _keyColumns=1;
  _shown=false;
  _mode=Edit;
  
  _model = new XSqlTableModel;
  _mapper = new XDataWidgetMapper;
  
  connect(_mapper, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
}

Screen::~Screen()
{
}

Screen::Disposition Screen::check()
{
  if (isDirty())
  {
    if (QMessageBox::question(this, tr("Unsaved work"),
                                   tr("You have made changes that have not been saved.  Would you like an opportunity to save your work?"),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No ) == QMessageBox::Yes)
      return Save;
    else
      return Cancel;
  }
  else if (_mode == New)
    return Cancel;
  return NoChanges;
}

/* When the widget is first shown, set up table mappings if they exist*/
void Screen::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;
    setTable(_schemaName, _tableName);
  }

  QWidget::showEvent(event);
}

Screen::Modes Screen::mode()
{
  return _mode;
}

bool Screen::cancel()
{
  switch (check())
  {
    case Save:
      return false;
    case Cancel:
      revertRow(currentIndex());
      return true;
    default: //No Change
      return true;
  }
}

bool Screen::isDirty()
{
  if (_mode==New) //If New and nothing has been changed yet on new row, it's not really dirty
  {
    for (int i = 0; _mapper->model() && i < _mapper->model()->columnCount(); i++)
      if (_mapper->mappedWidgetAt(i))
      {
        QString def = _mapper->mappedWidgetAt(i)->property(_mapper->mappedDefaultName(_mapper->mappedWidgetAt(i))).toString();
        QString cur = _mapper->mappedWidgetAt(i)->property(_mapper->mappedPropertyName(_mapper->mappedWidgetAt(i))).toString();
        if (!def.isEmpty() || !cur.isEmpty())
          if (def != cur)
            return true; 
      }
  }
  else  //Use usual technique to determine if edits occured
  {
    for (int c = 0; c < _model->columnCount(); c++)
      if (_model->isDirty(_model->index(currentIndex(),c)))
        return true;
  }
  return false;
}

bool Screen::throwScriptException(const QString &message)
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

bool Screen::submit()
{
  bool isSaved;
  disconnect(_mapper, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
  int i=_mapper->currentIndex();
  _mapper->submit();  // Make sure changes committed even if user hasn't tabbed off widget
  isSaved=_model->submitAll(); // Apply to the database
  _mapper->setCurrentIndex(i);
  if (!isSaved)
  {
    if(!throwScriptException(_model->lastError().databaseText()))
        QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                          tr("Error saving %1 at %2::%3\n\n%4")
                          .arg(_tableName).arg(__FILE__).arg(__LINE__)
                          .arg(_model->lastError().databaseText()));
  }
  connect(_mapper, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
  
  return isSaved;
}

int Screen::currentIndex()
{
  return _mapper->currentIndex();
}

void Screen::deleteCurrent()
{
  removeCurrent();
  save();
}

void Screen::clear()
{
  _mapper->clear();
}

void Screen::insert()
{
  _mapper->model()->insertRows(_model->rowCount(),1);
  _mapper->toLast();
  _mapper->clear();
}

void Screen::newMappedWidget(QWidget *widget)
{
  widget->setEnabled(_mode != View);
}

void Screen::removeCurrent()
{
  removeRows(_mapper->currentIndex(),1);
}

void Screen::removeRows(int row, int count)
{
  _mapper->model()->removeRows(row, count);
}

void Screen::revert()
{
  _mapper->revert();
  emit reverted(currentIndex());
}

void Screen::revertAll()
{
  _model->revertAll();
  emit revertedAll();
}

void Screen::revertRow(int row)
{
  _model->revertRow(row);
}

void Screen::save()
{ 
  if (submit())
  {
    emit saved();
    if (_mode==New)
      insert();
  }
}

void Screen::search(QString criteria)
{
  QString filter;
  if (criteria.trimmed().length())
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
    _mapper->toFirst();
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

void Screen::setModel(XSqlTableModel *model)
{
  _model=model;
  _model->setKeys(_keyColumns);
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  setDataWidgetMapper(_model);
  emit newModel(_model);
}

void Screen::setSortColumn(QString p)
{
  _sortColumn = p;
}

void Screen::setCurrentIndex(int index)
{
  _mode=Screen::Edit;
  _mapper->setCurrentIndex(index);
}

void Screen::toNext()
{
  if (_mapper->currentIndex() < _model->rowCount()-1)
    _mapper->toNext();
}

void Screen::toPrevious()
{
  if (_mapper->currentIndex() > 0)
  {
    _mode=Screen::Edit;
    _mapper->toPrevious(); 
  }
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
      _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
      setDataWidgetMapper(_model);
      emit newModel(_model);
    }
  }
}

void Screen::setDataWidgetMapper(XSqlTableModel *model)
{
  _mapper->setModel(model);
  emit newDataWidgetMapper(_mapper);
}
