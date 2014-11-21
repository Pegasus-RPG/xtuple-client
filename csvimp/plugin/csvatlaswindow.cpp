/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvatlaswindow.h"

#include <QCloseEvent>
#include <QDomDocument>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QSqlDatabase>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTextStream>

#include <metasqlhighlighter.h>

#include "csvaddmapinputdialog.h"
#include "csvatlas.h"
#include "csvimpdata.h"
#include "interactivemessagehandler.h"
#include "missingfield.h"
#include "rowcontroller.h"

#define DEBUG false

CSVAtlasWindow::CSVAtlasWindow(QWidget *parent) : QMainWindow(parent)
{
  setupUi(this);

  _atlas       = new CSVAtlas();
  _currentDir  = QString::null;
  _filename    = QString::null;
  _msghandler  = new InteractiveMessageHandler(this);
  _selectedMap = QString::null;

  sMapChanged(0);

  MetaSQLHighlighter *tmp = new MetaSQLHighlighter(_preSql);
                      tmp = new MetaSQLHighlighter(_postSql);
  connect(_delimiter, SIGNAL(editTextChanged(QString)), this, SIGNAL(delimiterChanged(QString)));
}

CSVAtlasWindow::~CSVAtlasWindow()
{
}

void CSVAtlasWindow::languageChange()
{
  retranslateUi(this);
}

void CSVAtlasWindow::fileNew()
{
  _map->clear();
  _filename = QString::null;
  sMapChanged(0);
  if(_atlas)
  {
    delete _atlas;
    _atlas = 0;
  }
  _atlas = new CSVAtlas();
}

void CSVAtlasWindow::fileOpen(QString filename)
{
  if (DEBUG)
    qDebug("CSVAtlasWindow::fileOpen(%s) entered with old _filename [%s]",
           qPrintable(filename), qPrintable(_filename));

  if (! filename.isEmpty())
  {
    if (! QFile::exists(filename))
    {
      QString fullpath = _currentDir + QDir::separator() + filename;
      if (DEBUG)
        qDebug("CSVAtlasWindow::fileOpen() retrying with [%s]",
               qPrintable(fullpath));

      if (QFile::exists(fullpath))
        filename = fullpath;
      else
        filename = QString::null;
    }
  }

  if (filename.isEmpty())
    filename = QFileDialog::getOpenFileName(this, tr("Open Atlas File"),
                                            _currentDir,
                                            QString("XML Files (*.xml);;All files (*)"));
  if (filename.isEmpty())
    return;

  _map->clear();
  sMapChanged(0);
  if(_atlas)
  {
    delete _atlas;
    _atlas = 0;
  }

  QFile file(filename);

  QDomDocument doc = QDomDocument();
  QString errMsg;
  int errLine, errCol;
  if(doc.setContent(&file, &errMsg, &errLine, &errCol))
  {
    _atlas = new CSVAtlas(doc.documentElement());
    _map->addItems(_atlas->mapList());
    sMapChanged(0);
    _filename = filename;
    _currentDir = QFileInfo(_filename).absoluteDir().absolutePath();
  }
  else
    _msghandler->message(QtWarningMsg, tr("Error Reading File"),
                         tr("<p>An error was encountered while trying to read "
                            "the Atlas file: %1.").arg(errMsg),
                         QUrl::fromLocalFile(filename),
                         QSourceLocation(QUrl::fromLocalFile(filename),
                                         errLine, errCol));

  if(!_atlas)
    _atlas = new CSVAtlas();
}

void CSVAtlasWindow::fileSave()
{
  if(_filename.isEmpty())
  {
    fileSaveAs();
    if(_filename.isEmpty())
      return;
  }
  sMapChanged(_map->currentIndex());

  QDomDocument doc = QDomDocument("openCSVAtlasDef");
  doc.appendChild(_atlas->createElement(doc));

  QFile file(_filename);
  if(file.open(QIODevice::WriteOnly))
  {
    QTextStream ts(&file);
    ts << doc.toString();
    file.close();
  }
  else
    _msghandler->message(QtWarningMsg, tr("Error Opening File"),
                         tr("<p>Could not open the file %1 for writing: %2")
                           .arg(_filename, file.errorString()),
                         QUrl::fromLocalFile(_filename), QSourceLocation());
}

void CSVAtlasWindow::fileSaveAs()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save Atlas File"),
                                                  _filename, QString::null);
  if(filename.isEmpty())
    return;

  _filename = filename;
  _currentDir = QFileInfo(_filename).absoluteDir().absolutePath();
  fileSave();
}

void CSVAtlasWindow::filePrint()
{
  _msghandler->message(QtWarningMsg, tr("Print not yet implemented"));
}

void CSVAtlasWindow::helpIndex()
{
  _msghandler->message(QtWarningMsg, tr("Help Index not yet implemented"));
}

void CSVAtlasWindow::helpContents()
{
  _msghandler->message(QtWarningMsg, tr("Help Contents not yet implemented"));
}

void CSVAtlasWindow::helpAbout()
{
  QMessageBox::about(this, tr("About %1").arg(CSVImp::name),
    tr("%1 version %2"
       "\n\n%3 is a tool for importing CSV files into a database."
       "\n\n%4, All Rights Reserved")
          .arg(CSVImp::name, CSVImp::version, CSVImp::name, CSVImp::copyright));
}

QString CSVAtlasWindow::map() const
{
  return _map->currentText();
}

XAbstractMessageHandler *CSVAtlasWindow::messageHandler() const
{
  return _msghandler;
}

void CSVAtlasWindow::sRenameMap()
{
  _msghandler->message(QtWarningMsg, tr("Renaming Maps feature has not yet been implemented."));
}

void CSVAtlasWindow::setDir(QString dirname)
{
  _currentDir = dirname;
}

bool CSVAtlasWindow::setMap(const QString mapname)
{
  _map->setCurrentIndex(_map->findText(mapname));
  int mapidx = _map->currentIndex();
  if (mapidx >= 0)
    sMapChanged(mapidx);
  return (mapidx >= 0);
}

void CSVAtlasWindow::setMessageHandler(XAbstractMessageHandler *handler)
{
  if (handler != _msghandler)
    _msghandler = handler;
}

void CSVAtlasWindow::sAddMap()
{
  QSqlDatabase db = QSqlDatabase::database();
  if(db.isValid())
  {
    QString name  = QString::null;
    QString table = QString::null;
    QString schema= QString::null;

    while (true)
    {
      if (DEBUG)
        qDebug("sAddMap() has map %s, schema %s, table %s",
               qPrintable(name), qPrintable(schema), qPrintable(table));
      CSVAddMapInputDialog addmapdlg(this);
      addmapdlg.setMapname(name);
      addmapdlg.setSchema(schema);
      addmapdlg.setTable(table);
      if (addmapdlg.exec() != QDialog::Accepted)
        return;

      name  = addmapdlg.mapname();
      table = addmapdlg.qualifiedTable();
      schema= addmapdlg.schema();

      if (name.isEmpty())
      {
        QMessageBox::warning(this, tr("Must enter name"),
                             tr("<p>Please enter a name for the new map."));
        continue;
      }

      if (_atlas->mapList().contains(name))
      {
        QMessageBox::warning(this, tr("Must enter unique name"),
                             tr("<p>The new map name you entered already "
                                "exists. Please enter in a unique map name."));
        continue;
      }

      break;
    }

    CSVMap map(name);
    map.setTable(table);
    _atlas->setMap(map);

    _map->clear();
    _map->insertItems(-1, _atlas->mapList());
    _map->setCurrentIndex(_atlas->mapList().indexOf(name));
    sMapChanged(_map->currentIndex());
  }
  else
    QMessageBox::critical(this, tr("No Database"),
                          tr("Could not get the database connection."));
}

void CSVAtlasWindow::sDeleteMap()
{
  _atlas->removeMap(_map->currentText());
  _map->clear();
  _map->insertItems(-1, _atlas->mapList());
  if(_map->currentIndex() >= _atlas->mapList().size())
    _map->setCurrentIndex(qMax(_atlas->mapList().size() - 1, 0));
  sMapChanged(_map->currentIndex());
  if (DEBUG)
  {
    qDebug("CSVAtlasWindow::sDeleteMap - _map contains :");
    for (int i = 0; i < _map->count(); i++)
      qDebug("\t%d [%s]", i, qPrintable(_map->itemText(i)));
  }
}

void CSVAtlasWindow::sMapChanged( int )
{
  CSVMap map;
  if(!_selectedMap.isEmpty())
  {
    map = _atlas->map(_selectedMap);
    if(tr("Insert") == _action->currentText())
      map.setAction(CSVMap::Insert);
    else if(tr("Update") == _action->currentText())
      map.setAction(CSVMap::Update);
    else if(tr("Append") == _action->currentText())
      map.setAction(CSVMap::Append);
    map.setDelimiter(_delimiter->currentText());
    map.setDescription(_description->toPlainText());
    map.setSqlPre(_preSql->toPlainText().trimmed());
    map.setSqlPreContinueOnError(_sqlPreContinueOnError->isChecked());
    map.setSqlPost(_postSql->toPlainText().trimmed());
    for(int r = 0; r < _fields->rowCount(); r++)
    {
      CSVMapField field = map.field(_fields->item(r, 1)->data(Qt::EditRole).toString());
      field.setName(_fields->item(r, 1)->data(Qt::EditRole).toString());

      if (qobject_cast<QCheckBox*>(_fields->cellWidget(r, 0)))
        field.setIsKey(qobject_cast<QCheckBox*>(_fields->cellWidget(r,0))->isChecked());
      else
        field.setIsKey(false);

      field.setType(QVariant::nameToType(_fields->item(r, 2)->data(Qt::EditRole).toString().toLatin1().data()));

      if (qobject_cast<QComboBox*>(_fields->cellWidget(r, 4)))
        field.setAction(CSVMapField::nameToAction(qobject_cast<QComboBox*>(_fields->cellWidget(r, 4))->currentText()));
      else
        field.setAction(CSVMapField::Action_Default);

      if (qobject_cast<QSpinBox*>(_fields->cellWidget(r, 5)))
        field.setColumn(qobject_cast<QSpinBox*>(_fields->cellWidget(r,5))->value());
      else
        field.setColumn(0);

      if (qobject_cast<QComboBox*>(_fields->cellWidget(r, 6)))
        field.setIfNullAction(CSVMapField::nameToIfNull(qobject_cast<QComboBox*>(_fields->cellWidget(r, 6))->currentText()));
      else
        field.setIfNullAction(CSVMapField::Nothing);

      if (qobject_cast<QSpinBox*>(_fields->cellWidget(r, 7)))
        field.setColumnAlt(qobject_cast<QSpinBox*>(_fields->cellWidget(r, 7))->value());
      else
        field.setColumnAlt(1);

      if (qobject_cast<QComboBox*>(_fields->cellWidget(r, 8)))
        field.setIfNullActionAlt(CSVMapField::nameToIfNull(qobject_cast<QComboBox*>(_fields->cellWidget(r, 8))->currentText()));
      else
        field.setIfNullActionAlt(CSVMapField::Nothing);

      field.setValueAlt(_fields->item(r, 9)->data(Qt::EditRole).toString());
      map.setField(field);
    }
    map.simplify();
    _atlas->setMap(map);
  }

  QSqlDatabase db = QSqlDatabase::database();
  if (db.isValid())
  {
    _fields->setRowCount(0);
    if(_map->count() && ! _map->currentText().isEmpty())
    {
      // CODE TO SELECT MAP
      _selectedMap = _map->currentText();
      map = _atlas->map(_selectedMap);

      _table->setTitle(tr("Table: ") + map.table());
      _table->setEnabled(true);

      _action->setCurrentIndex(map.action());
      _description->setText(map.description());

      int delimidx = _delimiter->findText(map.delimiter());
      if (delimidx >= 0)
        _delimiter->setCurrentIndex(delimidx);
      else if (! map.delimiter().isEmpty())
        _delimiter->addItem(map.delimiter());
      else
        _delimiter->setCurrentIndex(0);

      _preSql->setText(map.sqlPre());
      _sqlPreContinueOnError->setChecked(map.sqlPreContinueOnError());
      _postSql->setText(map.sqlPost());

      QSqlRecord record = db.record(map.table());
      QStringList fieldnames;
      if (record.isEmpty())
      {
        _msghandler->message(QtWarningMsg, tr("No Existing Table"),
                             tr("<p>The table %1 does not exist in this "
                                "database. You may continue to use and edit "
                                "this map but only those fields that are known "
                                "will be shown.").arg(map.table()),
                             QUrl(), QSourceLocation());
        fieldnames = map.fieldList();
      }
      else
      {
        QStringList fList = map.fieldList();

        for(int i = 0; i < fList.size(); ++i)
        {
          CSVMapField f = map.field(fList.at(i));
          if(!record.contains(fList.at(i)))
          {
            map.removeField(fList.at(i));
            MissingField diag(this, f.name(), record);
            if(diag.exec() == QDialog::Accepted)
            {
              f.setName(diag._fields->currentText());
              map.setField(f);
            }
            _atlas->setMap(map);
          }
        }

        for (int i = 0; i < record.count(); i++)
          fieldnames.append(record.fieldName(i));
      }

      _fields->setRowCount(fieldnames.size());
      for(int row = 0; row < fieldnames.size(); ++row)
      {
        CSVMapField mf = map.field(fieldnames.at(row));

        QCheckBox *check = new QCheckBox(_fields);
        if(!mf.isEmpty())
          check->setChecked(mf.isKey());
        _fields->setCellWidget(row, 0, check);

        _fields->setItem(row, 1, new QTableWidgetItem(fieldnames.at(row)));
        if (record.isEmpty())
        {
          _fields->setItem(row, 2,
                           new QTableWidgetItem(QVariant::typeToName(mf.type())));
          _fields->setItem(row, 3, new QTableWidgetItem(tr("Unknown")));
        }
        else
        {
          if (record.field(row).type() == QVariant::String &&
              record.field(row).length() > 0)
            _fields->setItem(row, 2,
                             new QTableWidgetItem(QVariant::typeToName(record.field(row).type()) +
                                                  QString("(%1)").arg(record.field(row).length())));
          else
            _fields->setItem(row, 2,
                             new QTableWidgetItem(QVariant::typeToName(record.field(row).type())));
          _fields->setItem(row, 3, new QTableWidgetItem(
                           (record.field(row).requiredStatus() == QSqlField::Required) ? tr("Yes") :
                           (record.field(row).requiredStatus() == QSqlField::Optional) ? tr("No")  :
                           tr("Unknown")));
        }

        QComboBox *actcombo = new QComboBox(_fields);
        actcombo->addItems(CSVMapField::actionList());
        if (! mf.isEmpty())
          actcombo->setCurrentIndex(mf.action());
        _fields->setCellWidget(row, 4, actcombo);

        QSpinBox *colspinner = new QSpinBox(_fields);
        colspinner->setRange(1, 999);
        colspinner->setPrefix(tr("Column "));
        if(!mf.isEmpty())
          colspinner->setValue(mf.column());
        _fields->setCellWidget(row, 5, colspinner);

        QComboBox *nullcombo = new QComboBox(_fields);
        nullcombo->addItems(CSVMapField::ifNullList());
        if (! mf.isEmpty())
          nullcombo->setCurrentIndex(mf.ifNullAction());
        _fields->setCellWidget(row, 6, nullcombo);

        QSpinBox *altspinner = new QSpinBox(_fields);
        altspinner->setRange(1, 999);
        altspinner->setPrefix(tr("Column "));
        if (! mf.isEmpty())
          altspinner->setValue(mf.columnAlt());
        _fields->setCellWidget(row, 7, altspinner);

        QComboBox *altnullcombo = new QComboBox(_fields);
        altnullcombo->addItems(CSVMapField::ifNullList(true));
        if (! mf.isEmpty())
          altnullcombo->setCurrentIndex(mf.ifNullActionAlt());
        _fields->setCellWidget(row, 8, altnullcombo);

        _fields->setItem(row, 9, new QTableWidgetItem(mf.valueAlt()));

        RowController *control = new RowController(_fields, row, colspinner);
        control->setAction(actcombo);
        control->setColumn(colspinner);
        control->setIfNull(nullcombo);
        control->setAltColumn(altspinner);
        control->setAltIfNull(altnullcombo);
        control->setAltValue(_fields->item(row, 9));
        control->finishSetup();
      }
    }
    else
    {
      _selectedMap = QString::null;
      _table->setTitle(tr("Table: "));
      _table->setEnabled(false);
    }
  }
  else
    _msghandler->message(QtCriticalMsg, tr("No Database"),
                         tr("Could not get the database connection."));
}

void CSVAtlasWindow::closeEvent( QCloseEvent * e)
{
  sMapChanged(_map->currentIndex());
  e->accept();
}

CSVAtlas* CSVAtlasWindow::getAtlas()
{
  sMapChanged(_map->currentIndex());
  return _atlas;
}
