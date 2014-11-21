/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvtoolwindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPixmap>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QProgressDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStatusBar>
#include <QTextTableCell>
#include <QTimerEvent>
#include <QVariant>

#include "csvatlas.h"
#include "csvatlaswindow.h"
#include "csvdata.h"
#include "csvimpdata.h"
#include "interactivemessagehandler.h"
#include "logwindow.h"

#include "CSVimpIcon.xpm"

#define DEBUG false

static const bool usetransaction = false;

CSVToolWindow::CSVToolWindow(QWidget *parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags),
  _atlasWindow(0)
{
  setupUi(this);
  if (objectName().isEmpty())
    setObjectName("CSVToolWindow");

  setWindowIcon(QPixmap(CSVimpIcon));
  (void)atlasWindow(); // initializes _atlasWindow
  _log         = new LogWindow(this);
  _data        = 0;
  _dbTimerId   = startTimer(60000);
  _currentDir  = QString::null;
  _msghandler  = new InteractiveMessageHandler(this);

  connect(_atlasWindow, SIGNAL(destroyed(QObject*)),      this, SLOT(cleanup(QObject*)));
  connect(_delim,       SIGNAL(editTextChanged(QString)), this, SLOT(sNewDelimiter(QString)));
}

CSVToolWindow::~CSVToolWindow()
{
  if (_atlasWindow)
  {
    delete _atlasWindow;
    _atlasWindow = 0;
  }
}

void CSVToolWindow::languageChange()
{
  retranslateUi(this);
}

CSVAtlasWindow *CSVToolWindow::atlasWindow()
{
  if(!_atlasWindow)
  {
    _atlasWindow = new CSVAtlasWindow(this);
    connect(_atlasWindow, SIGNAL(delimiterChanged(QString)), _delim, SLOT(setEditText(QString)));
  }
  return _atlasWindow;
}

void CSVToolWindow::clearImportLog()
{
  if (_log)
    _log->_log->clear();
}

void CSVToolWindow::fileNew()
{
  QMessageBox::information(this, tr("Not Yet Implemented"), tr("This function has not been implemented."));
}

void CSVToolWindow::fileOpen(QString filename)
{
  fileOpenAction->setEnabled(false);
  _firstRowHeader->setEnabled(false);

  if (filename.isEmpty())
    filename = QFileDialog::getOpenFileName(this, tr("Select CSV File"),
                                            _currentDir,
                                            QString("CSV Files (*.csv);;All files (*)"));

  if (! filename.isEmpty())
  {
    _currentDir = filename;
    statusBar()->showMessage(tr("Loading %1...").arg(filename));

    if (_data != 0)
    {
      delete _data;
      _data = 0; // must 0 because sNewDelimiter refers to _data
    }
    _data = new CSVData(this, 0, sNewDelimiter(_delim->currentText()));
    if (_msghandler)
      _data->setMessageHandler(_msghandler);

    if (_data->load(filename, this))
    {
      _data->setFirstRowHeaders(_firstRowHeader->isChecked());

      populate();
      statusBar()->showMessage(tr("Done loading %1").arg(filename));
    }
  }

  _firstRowHeader->setEnabled(true);
  fileOpenAction->setEnabled(true);
}

void CSVToolWindow::populate()
{
  if (! _data)
    return;

  // limit the preview to just the first N rows, or ALL rows if N == 0
  int cols = _data->columns();
  int rows = _preview->value();
  if (_preview->value() == 0 || _data->rows() < _preview->value())
    rows = _data->rows();
  _table->setColumnCount(cols);
  _table->setRowCount(rows);

  if(_firstRowHeader->isChecked())
  {
    for(int h = 0; h < cols; h++)
    {
      QString header = _data->header(h);
      if(header.isEmpty())
        header = QString(h + 1);
      else
        header = QString("%1 (%2)").arg(h+1).arg(header);
      _table->setHorizontalHeaderItem(h, new QTableWidgetItem(header));
    }
  }
  QString progresstext(tr("Displaying Record %1 of %2"));
  QProgressDialog progress(progresstext.arg(0).arg(rows),
                           tr("Stop"), 0, rows, this);
  progress.setWindowModality(Qt::WindowModal);

  QString v = QString::null;
  for (int r = 0; r < rows; r++)
  {
    if (progress.wasCanceled())
      break;

    for(int c = 0; c < cols; c++)
    {
      v = _data->value(r, c);
      if(QString::null == v)
        v = tr("(NULL)");
      _table->setItem(r, c, new QTableWidgetItem(v));
    }
    if ((r % 1000) == 0)
    {
      progress.setLabelText(progresstext.arg(r).arg(rows));
      progress.setValue(r);
    }
  }
  progress.setValue(rows);
}

void CSVToolWindow::fileSave()
{
  QMessageBox::information(this, tr("Not Yet Implemented"), tr("This function has not been implemented."));
}

void CSVToolWindow::fileSaveAs()
{
  QMessageBox::information(this, tr("Not Yet Implemented"), tr("This function has not been implemented."));
}

void CSVToolWindow::filePrint()
{
  if (QMessageBox::question(this, tr("Are you sure?"),
                            tr("<p>Printing does not work well yet. Files "
                               "with more than a handful of columns print "
                               "each column only a few characters wide.<p>"
                               "Are you sure you want to print?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
  {
    /* TODO: make this split wide files across multiple pages. */
    QTextDocument    textdoc(_table);
    QTextCursor      cursor(&textdoc);
    QTextTableFormat tblfmt;

    // to avoid attempting to access text from an empty
    // element (pointer)
    QTableWidgetItem *cell;

    QFont docfont = textdoc.defaultFont();
    docfont.setPointSize(8);
    textdoc.setDefaultFont(docfont);

    cursor.insertTable(_table->rowCount(), _table->columnCount());

    if (_firstRowHeader->isChecked())
    {
      tblfmt.setHeaderRowCount(1);
      for (int i = 0; i < _table->columnCount(); i++)
      {
        cell = _table->horizontalHeaderItem(i);
        if(cell)
          cursor.insertText(cell->text());
        cursor.movePosition(QTextCursor::NextCell);
      }
    }

    for (int row = 0; row < _table->rowCount(); row++)
      for (int col = 0; col < _table->columnCount(); col++)
      {
        cell = _table->item(row, col);
        if(cell)
          cursor.insertText(cell->text());
        cursor.movePosition(QTextCursor::NextCell);
      }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog printdlg(&printer, this);
    if (printdlg.exec() == QDialog::Accepted)
      textdoc.print(&printer);
  }
}

void CSVToolWindow::fileExit()
{
  if(_atlasWindow)
    _atlasWindow->close();
  close();
}

void CSVToolWindow::helpIndex()
{
  QMessageBox::information(this, tr("Not Yet Implemented"), tr("This function has not been implemented."));
}

void CSVToolWindow::helpContents()
{
  QMessageBox::information(this, tr("Not Yet Implemented"), tr("This function has not been implemented."));
}

void CSVToolWindow::helpAbout()
{
  QMessageBox::about(this, tr("About %1").arg(CSVImp::name),
    tr("%1 version %2"
       "\n\n%3 is a tool for importing CSV files into a database."
       "\n\n%4, All Rights Reserved")
          .arg(CSVImp::name, CSVImp::version, CSVImp::name, CSVImp::copyright));
}

void CSVToolWindow::mapEdit()
{
  if (! _atlasWindow)
    atlasWindow(); // initializes _atlasWindow

  _atlasWindow->show();
}

XAbstractMessageHandler *CSVToolWindow::messageHandler() const
{
  return _msghandler;
}

void CSVToolWindow::sFirstRowHeader( bool firstisheader )
{
  if(_data && _data->firstRowHeaders() != firstisheader)
  {
    _data->setFirstRowHeaders(firstisheader);
    int cols = _data->columns();

    QString header;
    for(int h = 0; h < cols; h++)
    {
      QString header = _data->header(h);
      if (header.trimmed().isEmpty())
        header = QString::number(h + 1);
      else
        header = QString("%1 (%2)").arg(h+1).arg(header);
      _table->setHorizontalHeaderItem(h, new QTableWidgetItem(header));
    }

    if(firstisheader)
      _table->removeRow(0);
    else if(_data->rows() > 0)
    {
      _table->insertRow(0);

      QString v = QString::null;
      for(int c = 0; c < cols; c++)
      {
        v = _data->value(0, c);
        if(QString::null == v)
          v = tr("(NULL)");
        _table->setItem(0, c, new QTableWidgetItem(v));
      }
    }
  }
}

QChar CSVToolWindow::sNewDelimiter(QString delim)
{
  QChar newdelim = ',';
  if (delim == tr("{ tab }"))
    newdelim = '\t';
  else if (! delim.isNull())
    newdelim = delim.at(0);

  if (_delim->currentText() != delim)
  {
    int delimidx = _delim->findText(delim);
    if (delimidx >= 0)
      _delim->setCurrentIndex(delimidx);
    else if (! delim.isEmpty())
      _delim->addItem(delim);
    else
      _delim->setCurrentIndex(0);
  }

  if (_data)
  {
    _data->setDelimiter(newdelim);
    populate();
    statusBar()->showMessage(tr("Done reloading"));
  }

  return newdelim;
}

bool CSVToolWindow::importStart()
{
  QString mapname = atlasWindow()->map();
  CSVAtlas *atlas = _atlasWindow->getAtlas();

  if (mapname.isEmpty())
  {
    QStringList mList = atlas->mapList();

    if(mList.isEmpty())
    {
      _msghandler->message(QtWarningMsg, tr("No Maps Loaded"),
                           tr("<p>There are no maps loaded to select from. "
                              "Either load an atlas that contains maps or "
                              "create a new one before continuing."));
      return false;
    }

    mList.sort();
    bool valid;
    mapname = QInputDialog::getItem(this, tr("Select Map"), tr("Select Map:"),
                                    mList, 0, false, &valid);
    if (!valid)
      return false;
  }

  CSVMap map = atlas->map(mapname);
  map.simplify();
  QList<CSVMapField> fields = map.fields();

  if (map.name() != mapname || fields.isEmpty())
  {
    _msghandler->message(QtWarningMsg, tr("Invalid Map"),
                         tr("<p>The selected map does not appear to be valid."));
    return false;
  }

  CSVMap::Action action = map.action();
  if (action != CSVMap::Insert)
  {
    _msghandler->message(QtWarningMsg, tr("Action not implemented"),
                         tr("<p>The action %1 for this map is not supported.")
                         .arg(CSVMap::actionToName(action)));
    return false;
  }

  if (!_data || _data->rows() < 1)
  {
    _msghandler->message(QtWarningMsg, tr("No data"),
                         tr("<p>There are no data to process. "
                            "Load a CSV file before continuing."));
    return false;
  }

  int total = _data->rows();
  int current = 0, error = 0, ignored = 0;

  if (! _log)
    _log = new LogWindow(this);

  if(usetransaction) QSqlQuery begin("BEGIN;");

  QString errMsg;
  if(!map.sqlPre().trimmed().isEmpty())
  {
    if(usetransaction) QSqlQuery savepoint("SAVEPOINT presql;");
    QSqlQuery pre;
    if(!pre.exec(map.sqlPre()))
    {
      errMsg = QString("ERROR Running Pre SQL query: %1").arg(pre.lastError().text());
      _log->_log->append("\n\n----------------------\n");
      _log->_log->append(errMsg);
      _log->show();
      _log->raise();
      if(map.sqlPreContinueOnError())
      {
        _log->_log->append(tr("\n\nContinuing with rest of import\n\n"));
        if(usetransaction) QSqlQuery sprollback("ROLLBACK TO SAVEPOINT presql;");
        if(usetransaction) QSqlQuery savepoint("RELEASE SAVEPOINT presql;");
      }
      else
      {
        if(usetransaction) QSqlQuery rollback("ROLLBACK;");
        _msghandler->message(QtWarningMsg, tr("Error"),
                             tr("<p>There was an error running the Pre SQL "
                                "query. Please see the log for more details. "
                                "Aborting transaction."));
        return false;
      }
    }
  }

  QString progresstext(tr("Importing %1: %2 rows out of %3"));
  int expected = total;
  QProgressDialog *progress = new QProgressDialog(progresstext
                                        .arg(map.name()).arg(0).arg(expected),
                                        tr("Cancel"), 0, expected, this);
  progress->setWindowModality(Qt::WindowModal);
  bool userCanceled = false;

  QString query;
  QString front;
  QString back;
  QString value;
  QString label;
  QVariant var;

  QStringList errorList;

  for(current = 0; current < total; ++current)
  {
    if(usetransaction) QSqlQuery savepoint("SAVEPOINT csvinsert;");
    if(action == CSVMap::Insert)
    {
      query = QString("INSERT INTO %1 ").arg(map.table());
      front = "(";
      back = " VALUES(";
      QList<CSVMapField> fields = map.fields();
      QMap<QString,QVariant> values;
      for (int i = 0; i < fields.size(); i++)
      {
        switch(fields.at(i).action())
        {
          case CSVMapField::Action_UseColumn:
          {
            value = _data->value(current, fields.at(i).column()-1);
            if(value.isNull())
            {
              switch (fields.at(i).ifNullAction())
              {
                case CSVMapField::UseDefault:
                  continue;
                case CSVMapField::UseEmptyString:
                {
                  var = QVariant(QString(""));
                  break;
                }
                case CSVMapField::UseAlternateValue:
                {
                  var = QVariant(fields.at(i).valueAlt());
                  break;
                }
                case CSVMapField::UseAlternateColumn:
                {
                  value = _data->value(current, fields.at(i).columnAlt()-1);
                  if(value.isNull())
                  {
                    switch (fields.at(i).ifNullActionAlt())
                    {
                      case CSVMapField::UseDefault:
                        continue;
                      case CSVMapField::UseEmptyString:
                      {
                        var = QVariant(QString(""));
                        break;
                      }
                      case CSVMapField::UseAlternateValue:
                      {
                        var = QVariant(fields.at(i).valueAlt());
                        break;
                      }
                      default: // Nothing
                        var = QVariant(QString::null);
                    }
                  }
                  else
                    var = QVariant(value);
                  break;
                }
                default: // Nothing
                  var = QVariant(QString::null);
              }
            }
            else
              var = QVariant(value);
            break;
          }
          case CSVMapField::Action_UseEmptyString:
          {
            var = QVariant(QString(""));
            break;
          }
          case CSVMapField::Action_UseAlternateValue:
          {
            var = QVariant(fields.at(i).valueAlt());
            break;
          }
          case CSVMapField::Action_UseNull:
          {
            var = QVariant(QString::null);
            break;
          }
          default:
            continue;
        }

        label = ":" + fields.at(i).name();
        if(!values.empty())
        {
          front += ", ";
          back  += ", ";
        }
        values.insert(label, var);
        front += fields.at(i).name();
        back  += label;
      }

      if(values.empty())
      {
        ignored++;
        errMsg = QString("IGNORED Record %1: There are no columns to insert").arg(current+1);
        errorList.append(errMsg);
        continue;
      }

      front += ") ";
      back += ")";
      query += front + back;
      QSqlQuery qry;
      qry.prepare(query);

      QMap<QString,QVariant>::iterator vit;
      for(vit = values.begin(); vit != values.end(); ++vit)
        qry.bindValue(vit.key(), vit.value());

      if(!qry.exec())
      {
        if(usetransaction) QSqlQuery sprollback("ROLLBACK TO SAVEPOINT csvinsert;");
        error++;
        errMsg = QString("ERROR Record %1: %2").arg(current+1).arg(qry.lastError().text());
        errorList.append(errMsg);
      }
    }
    if (progress->wasCanceled())
    {
      userCanceled = true;
      break;
    }
    if(! (current % 1000))
    {
      progress->setLabelText(progresstext.arg(map.name()).arg(current).arg(expected));
      progress->setValue(current);
    }
  }
  progress->setValue(total);

  if (error || ignored || userCanceled)
  {
    _log->_log->append(tr("Map: %1\n"
                          "Table: %2\n"
                          "Method: %3\n\n"
                          "Total Records: %4\n"
                          "# Processed:   %5\n"
                          "# Ignored:     %6\n"
                          "# Errors:      %7\n\n")
                          .arg(map.name()).arg(map.table())
                          .arg(CSVMap::actionToName(map.action()))
                          .arg(total).arg(current).arg(ignored).arg(error));
    _log->_log->append(errMsg);
    _log->_log->append(errorList.join("\n"));
    _log->show();
    _log->raise();
    if (_msghandler &&  // log messages there's a non-interactive message handler
        qobject_cast<XAbstractMessageHandler*>(_msghandler) &&
        ! qobject_cast<InteractiveMessageHandler*>(_msghandler))
      _msghandler->message(error ? QtCriticalMsg : QtWarningMsg,
                           tr("Import Processing Status"),
                           _log->_log->toPlainText());
  }

  if (! userCanceled && ! map.sqlPost().trimmed().isEmpty())
  {
    QSqlQuery post;
    if(!post.exec(map.sqlPost()))
    {
      errMsg = QString("ERROR Running Post SQL query: %1").arg(post.lastError().text());
      _log->_log->append("\n\n----------------------\n");
      _log->_log->append(errMsg);
      _log->show();
      _log->raise();
      if(usetransaction) QSqlQuery rollback("ROLLBACK;");
      _msghandler->message(QtCriticalMsg, tr("Error"),
                           tr("<p>There was an error running the post sql "
                              "query and changes were rolled back. "
                              "Please see the log for more details."));
      return false;
    }
  }

  if (userCanceled)
  {
    if(usetransaction) QSqlQuery rollback("ROLLBACK;");
    _log->_log->append(tr("\n\nImport canceled by user. Changes were rolled back."));

    return false;
  }

  if(usetransaction) QSqlQuery commit("COMMIT");
  if (! error)
  {
    _msghandler->message(QtDebugMsg, tr("Import Complete"),
                         tr("The import of %1 completed successfully.")
                         .arg(_currentDir));
    return true;
  }

  return false;
}

void CSVToolWindow::sImportViewLog()
{
  _log->show();
}

void CSVToolWindow::setDir(QString dirname)
{
  if (DEBUG)
    qDebug("%s::setDir(%s)", qPrintable(objectName()), qPrintable(dirname));
  _currentDir = dirname;
}

void CSVToolWindow::setMessageHandler(XAbstractMessageHandler *handler)
{
  if (handler != _msghandler)
  {
    _msghandler = handler;
    if (_data)
      _data->setMessageHandler(handler);
  }
}

void CSVToolWindow::timerEvent( QTimerEvent * e )
{
  if(e->timerId() == _dbTimerId)
  {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if(db.isOpen())
    {
      QSqlQuery qry("SELECT CURRENT_DATE;");
    }
    // if we are not connected then we have some problems!
  }
}

void CSVToolWindow::cleanup(QObject *deadobj)
{
  if (deadobj == _atlasWindow)
    _atlasWindow = 0;
  else if (deadobj == _log)
    _log = 0;
  else if (deadobj == _data)
    _data = 0;
}
