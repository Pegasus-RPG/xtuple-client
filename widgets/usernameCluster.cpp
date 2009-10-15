/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QtScript>

#include <parameter.h>
#include <xsqlquery.h>

#include "usernameList.h"
#include "usernamecluster.h"

UsernameLineEdit::UsernameLineEdit(QWidget * parent, const char * name)
  : XLineEdit(parent, name)
{
  _id = -1;
  _type = UsersAll;

  setMaximumWidth(100);

  connect(this, SIGNAL(lostFocus()), SLOT(sParse()));
}

void UsernameLineEdit::setId(int pId)
{
  QString sql("SELECT usr_username"
              "  FROM usr"
              " WHERE ((usr_id=:id)");

  if(UsersActive == _type)
    sql += " AND (usr_active)";
  else if(UsersInactive == _type)
    sql += " AND (NOT usr_active)";

  sql += " );";

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":id", pId);
  query.exec();
  if(query.first())
  {
    _id = pId;
    _valid = true;
    _username = query.value("usr_username").toString();
  }
  else
  {
    _id = -1;
    _valid = false;
    _username = "";
  }

  setText(_username);
  emit newId(_id);
  emit valid(_valid);

  _parsed = true;
}

void UsernameLineEdit::setUsername(const QString & pUsername)
{
  XSqlQuery query;
  query.prepare("SELECT usr_id"
                "  FROM usr"
                " WHERE (usr_username=:username);");
  query.bindValue(":username", pUsername);
  query.exec();
  if(query.first())
    setId(query.value("usr_id").toInt());
  else
    setId(-1);
}

void UsernameLineEdit::clear()
{
  setId(-1);
}

void UsernameLineEdit::sParse()
{
  if(!_parsed)
  {
    _parsed = true;
    setUsername(text());
  }
}

int UsernameLineEdit::id()
{
  if(hasFocus())
    sParse();
  return _id;
}

const QString & UsernameLineEdit::username()
{
  if(hasFocus())
    sParse();
  return _username;
}

UsernameCluster::UsernameCluster(QWidget * parent, const char * name)
  : QWidget(parent, name)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(6);

  _label = new QLabel(tr("Username:"), this);
  _label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(_label);

  _username = new UsernameLineEdit(this);
  layout->addWidget(_username);

  _list = new QPushButton(tr("..."), this);
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#else
  _list->setMinimumWidth(60);
  _list->setMinimumHeight(32);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(_list);

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addItem(spacer);

  setLayout(layout);

  connect(_list, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_username, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_username, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_username);
}

void UsernameCluster::sList()
{
  ParameterList params;
  params.append("id", _username->id());
  params.append("type", (int)_username->type());

  usernameList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if((id = newdlg.exec()) != QDialog::Rejected)
    _username->setId(id);
}

void UsernameCluster::setReadOnly(bool pReadOnly)
{
  if(pReadOnly)
    _list->hide();
  else
    _list->show();
  _username->setEnabled(!pReadOnly);
}

QString UsernameCluster::label() const
{
  return _label->text();
}

void UsernameCluster::setLabel(QString t)
{
  _label->setText(t);
}

void UsernameCluster::setUsername(const QString & pUsername)
{
  _username->setUsername(pUsername);
}

// script exposure ////////////////////////////////////////////////////////////

QScriptValue UsernameLineEdittoScriptValue(QScriptEngine *engine, UsernameLineEdit* const &item)
{
  return engine->newQObject(item);
}

void UsernameLineEditfromScriptValue(const QScriptValue &obj, UsernameLineEdit* &item)
{
  item = qobject_cast<UsernameLineEdit*>(obj.toQObject());
}

QScriptValue constructUsernameLineEdit(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  UsernameLineEdit *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate UsernameLineEdit constructor");

  return engine->toScriptValue(obj);
}

void setupUsernameLineEdit(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, UsernameLineEdittoScriptValue, UsernameLineEditfromScriptValue);

  QScriptValue widget = engine->newFunction(constructUsernameLineEdit);

  widget.setProperty("UsersAll",     QScriptValue(engine, UsernameLineEdit::UsersAll),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersActive",  QScriptValue(engine, UsernameLineEdit::UsersActive),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UsersInactive",QScriptValue(engine, UsernameLineEdit::UsersInactive),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("UsernameLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue UsernameClustertoScriptValue(QScriptEngine *engine, UsernameCluster* const &item)
{
  return engine->newQObject(item);
}

void UsernameClusterfromScriptValue(const QScriptValue &obj, UsernameCluster* &item)
{
  item = qobject_cast<UsernameCluster*>(obj.toQObject());
}

QScriptValue constructUsernameCluster(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  UsernameCluster *obj = 0;

  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameCluster(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new UsernameCluster(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               qPrintable(context->argument(1).toString()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate UsernameCluster constructor");

  return engine->toScriptValue(obj);
}

void setupUsernameCluster(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, UsernameClustertoScriptValue, UsernameClusterfromScriptValue);

  QScriptValue widget = engine->newFunction(constructUsernameCluster);

  engine->globalObject().setProperty("UsernameCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
