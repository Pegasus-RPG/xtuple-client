/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QtScript>

void setupQMessageBox(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("InvalidRole",    QScriptValue(engine, QMessageBox::InvalidRole),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AcceptRole",     QScriptValue(engine, QMessageBox::AcceptRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RejectRole",     QScriptValue(engine, QMessageBox::RejectRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DestructiveRole",QScriptValue(engine, QMessageBox::DestructiveRole),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActionRole",     QScriptValue(engine, QMessageBox::ActionRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("HelpRole",       QScriptValue(engine, QMessageBox::HelpRole),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("YesRole",        QScriptValue(engine, QMessageBox::YesRole),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoRole",         QScriptValue(engine, QMessageBox::NoRole),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ApplyRole",      QScriptValue(engine, QMessageBox::ApplyRole),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ResetRole",      QScriptValue(engine, QMessageBox::ResetRole),      QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("NoIcon",         QScriptValue(engine, QMessageBox::NoIcon),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Question",       QScriptValue(engine, QMessageBox::Question),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Information",    QScriptValue(engine, QMessageBox::Information),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Warning",        QScriptValue(engine, QMessageBox::Warning),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Critical",       QScriptValue(engine, QMessageBox::Critical),       QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("Ok",             QScriptValue(engine, QMessageBox::Ok),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Open",           QScriptValue(engine, QMessageBox::Open),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Save",           QScriptValue(engine, QMessageBox::Save),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Cancel",         QScriptValue(engine, QMessageBox::Cancel),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Close",          QScriptValue(engine, QMessageBox::Close),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Discard",        QScriptValue(engine, QMessageBox::Discard),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Apply",          QScriptValue(engine, QMessageBox::Apply),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Reset",          QScriptValue(engine, QMessageBox::Reset),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RestoreDefaults",QScriptValue(engine, QMessageBox::RestoreDefaults),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Help",           QScriptValue(engine, QMessageBox::Help),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SaveAll",        QScriptValue(engine, QMessageBox::SaveAll),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Yes",            QScriptValue(engine, QMessageBox::Yes),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("YesToAll",       QScriptValue(engine, QMessageBox::YesToAll),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("No",             QScriptValue(engine, QMessageBox::No),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoToAll",        QScriptValue(engine, QMessageBox::NoToAll),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Abort",          QScriptValue(engine, QMessageBox::Abort),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Retry",          QScriptValue(engine, QMessageBox::Retry),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Ignore",         QScriptValue(engine, QMessageBox::Ignore),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoButton",       QScriptValue(engine, QMessageBox::NoButton),       QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("QMessageBox", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
