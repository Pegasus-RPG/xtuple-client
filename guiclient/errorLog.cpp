/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "errorLog.h"
#include "guiclient.h"

#include <QObject>
#include <QVariant>
#include <QMessageBox>
#include <QStringList>
#include <QDateTime>
#include <QSqlError>

static QStringList _errorList;
static errorLogListener * listener = 0;

void errorLogListener::initialize()
{
  listener = new errorLogListener();
}

errorLog::errorLog(QWidget* parent, Qt::WFlags flags)
    : XWidget(parent, flags)
{
  setupUi(this);

//  statusBar()->hide();

  for(int i = 0; i < _errorList.size(); i++)
    _errorLog->append(_errorList.at(i));

  connect(listener, SIGNAL(updated(const QString &)), this, SLOT(updateErrors(const QString &)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
errorLog::~errorLog()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void errorLog::languageChange()
{
  retranslateUi(this);
}

void errorLog::updateErrors(const QString & msg)
{
  _errorLog->append(msg);
}


errorLogListener::errorLogListener(QObject * parent)
  : QObject(parent)
{
  XSqlQuery::addErrorListener(this);
}

errorLogListener::~errorLogListener() {}

void errorLogListener::error(const QString & sql, const QSqlError & error)
{
  QString msg;
  msg = QDateTime::currentDateTime().toString();
  msg += " " + error.text();
  msg += "\n" + sql;

  _errorList.append(msg);
  if(_errorList.size() > 20)
    _errorList.removeFirst();

  emit updated(msg);
  if(omfgThis)
    omfgThis->sNewErrorMessage();
}

