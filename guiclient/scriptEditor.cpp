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

  connect(_export,       SIGNAL(clicked()), this, SLOT(sExport()));
  connect(_import,       SIGNAL(clicked()), this, SLOT(sImport()));
  connect(_line, SIGNAL(editingFinished()), this, SLOT(sGoto()));
  connect(_save,         SIGNAL(clicked()), this, SLOT(sSave()));

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
  q.prepare( "SELECT script.*, relname ~* 'pkgscript' AS inPackage "
      	     "  FROM script, pg_class"
             " WHERE ((script.tableoid=pg_class.oid)"
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
    if (DEBUG)
      qDebug("scriptEditor::populate() inPackage = %d",
             q.value("inPackage").toBool());
    if (q.value("inPackage").toBool())
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

void scriptEditor::sBlockCountChanged(const int p)
{
  _line->setMaximum(p);
}

void scriptEditor::sPositionChanged()
{
  _line->setValue(_source->textCursor().blockNumber() + 1);
}
