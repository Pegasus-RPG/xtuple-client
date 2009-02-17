/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "wotc.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

wotc::wotc(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save,   SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_user, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_wo,    SIGNAL(newId(int)), this, SLOT(sPopulateWooper()));
  connect(_wo,   SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _wooper->setAllowNull(true);
  _wooper->setHidden(! (_metrics->value("WOTCPostStyle") == "Operations"));
}

wotc::~wotc()
{
  // no need to delete child widgets, Qt does it all for us
}

void wotc::languageChange()
{
    retranslateUi(this);
}

enum SetResponse wotc::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _wo->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _close->setText(tr("&Close"));
      _save->hide();
      _wo->setEnabled(false);
      _user->setEnabled(false);
      _close->setFocus();
    }
  }
  
  param = pParams.value("wotc_id", &valid);
  if (valid)
    _wotcid = param.toInt();
  
  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  param = pParams.value("usr_id", &valid);
  if (valid)
    _user->setId(param.toInt());

  populate();

  return NoError;
}

void wotc::sHandleButtons()
{
  bool enable = _wo->isValid() && _user->isValid() &&
		(_mode == cNew || _mode == cEdit);
  
  _dateIn->setEnabled(enable);
  _dateOut->setEnabled(enable);
  _timeIn->setEnabled(enable);
  _timeOut->setEnabled(enable);
  _save->setEnabled(enable);
}

void wotc::populate()
{
  if (_wotcid != -1)
  {
    XSqlQuery pop;
    pop.prepare("SELECT wotc_wo_id, wotc_username,"
	        "       wotc_timein, wotc_timeout,"
	        "       wotc_wooper_id "
	        "FROM wotc "
	        "WHERE (wotc_id=:wotc_id);");
    pop.bindValue(":wotc_id", _wotcid);
    pop.exec();
    if (pop.first())
    {
      _wo->setId(pop.value("wotc_wo_id").toInt());
      _user->setUsername(pop.value("wotc_username").toString());
      _dateIn->setDate(pop.value("wotc_timein").toDate());
      _dateOut->setDate(pop.value("wotc_timeout").toDate());
      _timeIn->setTime(pop.value("wotc_timein").toTime());
      _timeOut->setTime(pop.value("wotc_timeout").toTime());
      if (! pop.value("wotc_wooper_id").isNull())
	_wooper->setId(pop.value("wotc_wooper_id").toInt());
    }
    else if (pop.lastError().type() != QSqlError::NoError)
      systemError(this, pop.lastError().databaseText(), __FILE__, __LINE__);
  }
}

void wotc::sSave()
{
  QWidget* invalidWidget = 0;
  QString msg;
  
  QDateTime dtIn(_dateIn->date(), _timeIn->time());
  QDateTime dtOut(_dateOut->date(), _timeOut->time());
  
  if (! _wo->isValid())
  {
    msg = tr("Please specify a work order before saving this time clock entry.");
    invalidWidget = _wo;
  }
  else if (! _user->isValid())
  {
    msg = tr("Please specify a valid user before saving this time clock entry.");
    invalidWidget = _user;
  }
  else if (! _dateIn->isValid())			// see mantis bug 4341
  {
    msg = tr("Please specify a valid clock in date/time.");
    invalidWidget = _dateIn;
  }
  else if (_dateOut->isValid() && dtIn > dtOut)		// see mantis bug 4341
  {
    msg = tr("Please specify a clock in date/time earlier than the clock out date/time.");
    invalidWidget = _dateIn;
  }
  else if (! _dateOut->isValid())
  {
    QDate tmp;
    dtOut.setDate(tmp);					// see mantis bug 4341
  }
  
  if (invalidWidget)
  {
    QMessageBox::critical(this, tr("Invalid Work Order Time Clock Entry"), msg);
    invalidWidget->setFocus();
    return;
  }
  
  if (_mode == cNew)
    q.prepare("INSERT INTO wotc (wotc_wo_id, wotc_username,"
	      "                  wotc_timein, wotc_timeout, wotc_wooper_id)"
	      "    VALUES (:wotc_wo_id, :wotc_username, "
	      "            :wotc_timein, :wotc_timeout, :wotc_wooper_id);");
  else if (_mode == cEdit)
  {
    q.prepare("UPDATE wotc SET"
	      " wotc_wo_id=:wotc_wo_id, wotc_username=:wotc_username,"
	      " wotc_timein=:wotc_timein, wotc_timeout=:wotc_timeout,"
	      " wotc_wooper_id=:wotc_wooper_id "
	      "WHERE (wotc_id=:wotc_id);");
    q.bindValue(":wotc_id", _wotcid);
  }
  q.bindValue(":wotc_wo_id", _wo->id());
  q.bindValue(":wotc_username", _user->username());
  q.bindValue(":wotc_timein", dtIn);
  q.bindValue(":wotc_timeout", (dtOut.isNull()) ? "" : dtOut.toString()); // bug 6655

  if (_wooper->id() != -1)
    q.bindValue(":wotc_wooper_id", _wooper->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  accept();
}

void wotc::sPopulateWooper()
{
  // apparent timing problem using _wo->status()
  QString woStatus;
  q.prepare("SELECT wo_status FROM wo WHERE (wo_id=:wo_id);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
    woStatus = q.value("wo_status").toString();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (woStatus == "C")
  {
    _wooper->setEnabled(false);
    _wooper->clear();
  }
  else if (_metrics->value("WOTCPostStyle") == "Operations")
  {
    _wooper->setEnabled(_mode == cEdit || _mode == cNew);
     QString sql("SELECT wooper_id, (wooper_seqnumber || ' - ' || wooper_descrip1 || ' - ' || wooper_descrip2) "
		 "FROM wooper "
		 "WHERE ((wooper_wo_id=%1) "
		 // "  AND  (NOT wooper_rncomplete)) "
		 ") "
		 "ORDER BY wooper_seqnumber;");
     XSqlQuery query;
     query.exec(sql.arg(_wo->id()));
     if (query.lastError().type() != QSqlError::NoError)
     {
       systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
       return;
     }
     _wooper->populate(query);

     if (_wotcid != -1)
     {
       query.prepare("SELECT wotc_wooper_id "
		     "FROM wotc "
		     "WHERE ((wotc_username=:username)"
		     "  AND  (wotc_wo_id=:wo_id)"
		     "  AND  (wotc_timeout IS NULL));");
       query.bindValue(":username", _user->username());
       query.bindValue(":wo_id", _wo->id());
       query.exec();
       if (query.first())
	 _wooper->setId(query.value("wotc_wooper_id").toInt());
       else if (query.lastError().type() != QSqlError::NoError)
	 systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
       else
	 _wooper->setId(_wooper->id(1));
     }
   }
}
