/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptEditor.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QFileDialog>
#include <QTextStream>
#include <QScriptEngine>

#include "jsHighlighter.h"

#define DEBUG false

static QString lastSaveDir = QString();

scriptEditor::scriptEditor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  setWindowModality(Qt::WindowModal);

  connect(_export,          SIGNAL(clicked()), this, SLOT(sExport()));
  connect(_find,            SIGNAL(clicked()), this, SLOT(sFind()));
  connect(_findText,SIGNAL(editingFinished()), this, SLOT(sFind()));
  connect(_import,          SIGNAL(clicked()), this, SLOT(sImport()));
  connect(_line,    SIGNAL(editingFinished()), this, SLOT(sGoto()));
  connect(_save,            SIGNAL(clicked()), this, SLOT(sSave()));

  _highlighter = new JSHighlighter(_source->document());
  _document = _source->document();
  _document->setDefaultFont(QFont("Courier"));

  connect(_document, SIGNAL(blockCountChanged(int)), this, SLOT(sBlockCountChanged(int)));
  connect(_document, SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
  connect(_source, SIGNAL(cursorPositionChanged()), this, SLOT(sPositionChanged()));

  _scriptid = -1;
}

scriptEditor::~scriptEditor()
{
  // no need to delete child widgets, Qt does it all for us
}

void scriptEditor::languageChange()
{
  retranslateUi(this);
}

void scriptEditor::closeEvent(QCloseEvent *event)
{
  if (_document->isModified())
  {
    switch (QMessageBox::question(this, tr("Save Changes?"),
                                  tr("Do you want to save your changes?"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
      case QMessageBox::Yes:
        if (sSave())
          event->accept();
        else
          event->ignore();
        break;
      case QMessageBox::No:
        event->accept();
        break;
      case QMessageBox::Cancel:
      default:
        event->ignore();
        break;
    }
  }
  else
    event->accept();
}

enum SetResponse scriptEditor::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      setMode(cNew);
    else if (param.toString() == "edit")
      setMode(cEdit);
    else if (param.toString() == "view")
      setMode(cView);
  }

  // follow setMode because populate() might change it
  param = pParams.value("script_id", &valid);
  if (valid)
  {
    _scriptid = param.toInt();
    populate();
  }

  return NoError;
}

void scriptEditor::setMode(const int pmode)
{
  if (DEBUG)
    qDebug("scriptEditor::setMode(%d)", pmode);
  switch (pmode)
  {
    case cNew:
    case cEdit:
      if (DEBUG) qDebug("scriptEditor::setMode(%d) case new/edit", pmode);
      _name->setEnabled(true);
      _order->setEnabled(true);
      _notes->setReadOnly(false);
      _source->setReadOnly(false);
      _enabled->setEnabled(true);
      _save->show();
      if (pmode == cNew)
        _name->setFocus();
      else
        _save->setFocus();
      break;

    case cView:
    default:
      if (DEBUG) qDebug("scriptEditor::setMode(%d) case view/default", pmode);
      _name->setEnabled(FALSE);
      _order->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _source->setReadOnly(TRUE);
      _enabled->setEnabled(FALSE);
      _save->hide();
      _close->setFocus();
  };

  _mode = pmode;
}

bool scriptEditor::sSave()
{
  if (_name->text().trimmed().length() == 0)
  {
    QMessageBox::warning( this, tr("Script Name is Invalid"),
                          tr("<p>You must enter a valid name for this Script.") );
    _name->setFocus();
    return false;
  }

  QScriptEngine engine;
  if (!engine.canEvaluate(_source->toPlainText()) || _source->text().length() == 0)
  {
    if (QMessageBox::question(this, windowTitle(),
                          tr("<p>The script appears incomplete are you sure you want to save?"),
                             QMessageBox::Yes,
                             QMessageBox::No  | QMessageBox::Default) != QMessageBox::Yes)
    return false;
  }
  
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('script_script_id_seq') AS _script_id");
    if (q.first())
      _scriptid = q.value("_script_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }

    q.prepare( "INSERT INTO script "
               "(script_id, script_name, script_notes, script_order, script_enabled, script_source) "
               "VALUES "
               "(:script_id, :script_name, :script_notes, :script_order, :script_enabled, :script_source);" );

  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE script "
               "SET script_name=:script_name, script_notes=:script_notes,"
               "    script_order=:script_order, script_enabled=:script_enabled,"
               "    script_source=:script_source "
               "WHERE (script_id=:script_id);" );

  q.bindValue(":script_id", _scriptid);
  q.bindValue(":script_name", _name->text());
  q.bindValue(":script_order", _order->value());
  q.bindValue(":script_enabled", QVariant(_enabled->isChecked()));
  q.bindValue(":script_source", _source->toPlainText());
  q.bindValue(":script_notes", _notes->text());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  _document->setModified(false);
  setMode(cEdit);
  return true;
}

void scriptEditor::populate()
{
  q.prepare( "SELECT script.*, COALESCE(pkghead_indev,true) AS editable "
      	     "  FROM script, pg_class, pg_namespace "
             "  LEFT OUTER JOIN pkghead ON (nspname=pkghead_name) "
             " WHERE ((script.tableoid=pg_class.oid)"
             "    AND (relnamespace=pg_namespace.oid) "
             "    AND (script_id=:script_id));" );
  q.bindValue(":script_id", _scriptid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("script_name").toString());
    _order->setValue(q.value("script_order").toInt());
    _enabled->setChecked(q.value("script_enabled").toBool());
    _source->setText(q.value("script_source").toString());
    _notes->setText(q.value("script_notes").toString());
    setWindowTitle("[*]" + tr("Script Editor - %1").arg(_name->text()));
    if (DEBUG)
      qDebug("scriptEditor::populate() editable = %d",
             q.value("editable").toBool());
    if (!q.value("editable").toBool())
      setMode(cView);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void scriptEditor::sImport()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QString::null, tr("Script (*.script *.js)"));
  if(filename.isNull())
    return;

  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Could not import file"), file.errorString());
    return;
  }
  QTextStream ts(&file);
  _source->setText(ts.readAll());
  file.close();
}

void scriptEditor::sExport()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save Script File"),
                                                  lastSaveDir +
                                                  QDir::separator() +
                                                  _name->text().trimmed() +
                                                  ".script",
                                                  tr("Script (*.script *.js)"));
  if(filename.isNull())
    return;

  QFileInfo fi(filename);
  if(fi.suffix().isEmpty())
    filename += ".script";

  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(this, tr("Could not export file"), file.errorString());
    return;
  }

  QTextStream ts(&file);
  ts.setCodec("UTF-8");
  ts << _source->toPlainText();
  file.close();

  lastSaveDir = fi.absolutePath();
}

void scriptEditor::sGoto()
{
  QTextCursor cursor = QTextCursor(_document->findBlockByNumber(_line->value() - 1));
  _source->setTextCursor(cursor);
  _source->ensureCursorVisible();
  _source->setFocus();
}

void scriptEditor::sFind()
{
  if (_findText->text().isEmpty())
    return;

  QTextCursor oldposition = _source->textCursor();
  bool found = _source->find(_findText->text());
  if (!found && ! oldposition.atStart())
  {
    int answer = QMessageBox::question(this, tr("Continue Search?"),
                                       tr("<p>'%1' was not found. "
                                          "Start search from the beginning?")
                                         .arg(_findText->text()),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::Yes);
    if (answer == QMessageBox::Yes)
    {
      QTextCursor newposition = oldposition;
      newposition.movePosition(QTextCursor::Start);
      _source->setTextCursor(newposition);
      found = _source->find(_findText->text());
    }
    else
      found = true;     // a lie to prevent a useless message
  }
  if (!found)
  {
    QMessageBox::information(this, tr("Not Found"),
                             tr("<p>'%1' was not found.")
                               .arg(_findText->text()));
    _source->setTextCursor(oldposition);
  }
  _source->setFocus();
}

void scriptEditor::sBlockCountChanged(const int p)
{
  _line->setMaximum(p);
}

void scriptEditor::sPositionChanged()
{
  _line->setValue(_source->textCursor().blockNumber() + 1);
}
