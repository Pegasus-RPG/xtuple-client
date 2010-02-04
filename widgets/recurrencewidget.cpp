/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "recurrencewidget.h"

#include <QMessageBox>
#include <QSqlError>

#include "xsqlquery.h"

#include "storedProcErrorLookup.h"

/**
  \class
  
  \brief The RecurrenceWidget gives the user a single interface for telling
         the system how often certain %events occur.

 */
/* TODO: expand to allow selecting Minutes and Hours. In this case we'll need
         to allow time entry as well as dates. If we can find a nice way to
         allow time entry here then we could use the same UI to address the
         issue about backdating inventory transactions to a specific date-time
         instead of just a particular date.
         
         The widget will need properties to set minimum/maximum periods
         (e.g. it doesn't make sense for invoicing to recur hourly).

         The enum value Custom is intended for use by cases like
            Recur every Tuesday and Thursday
            Recur 1st of every month
            Recur every January and July

         It would be nice to have drag-n-drop using iCalendar (IETF RFC 5545)
         but this will have to be associated with the To-do or calendar item,
         not just the recurrence widget.
 */

RecurrenceWidget::RecurrenceWidget(QWidget *parent, const char *pName) :
  QWidget(parent)
{
  setupUi(this);

  if(pName)
    setObjectName(pName);

//_period->append(Never,        tr("Never"),   "");
//_period->append(Minutely,     tr("Minutes"), "m");
//_period->append(Hourly,       tr("Hours"),   "H");
  _period->append(Daily,        tr("Days"),    "D");
  _period->append(Weekly,       tr("Weeks"),   "W");
  _period->append(Monthly,      tr("Months"),  "M");
  _period->append(Yearly,       tr("Years"),   "Y");
//_period->append(Custom,       tr("Custom"),  "C");

  _period->setCode("W");

  _dates->setStartCaption(tr("From:"));
  _dates->setEndCaption(tr("Until:"));
  _dates->setStartNull(tr("Today"), QDate::currentDate(), true);
  _dates->setStartVisible(false);

  setMaxVisible(false);

  XSqlQuery eotq("SELECT endOfTime() AS eot;");
  if (eotq.first())
    _dates->setEndNull(tr("Forever"), eotq.value("eot").toDate(), true);
  else
  {
    qWarning("RecurrenceWidget could not get endOfTime()");
    _dates->setEndNull(tr("Today"), QDate::currentDate(), true);
  }

  clear();
}

RecurrenceWidget::~RecurrenceWidget()
{
}

void RecurrenceWidget::languageChange()
{
}

void RecurrenceWidget::clear()
{
  set();
  _id             = -1;

  _prevParentId   = -1;
  _prevParentType = "";
  _parentId       = -1;
  _parentType     = "";
}

RecurrenceWidget::RecurrencePeriod RecurrenceWidget::stringToPeriod(QString p) const
{
  if (p == "m"      || p == tr("Minutes"))  return Minutely;
  else if (p == "H" || p == tr("Hours")  )  return Hourly;
  else if (p == "D" || p == tr("Days")   )  return Daily;
  else if (p == "W" || p == tr("Weeks")  )  return Weekly;
  else if (p == "M" || p == tr("Months") )  return Monthly;
  else if (p == "Y" || p == tr("Years")  )  return Yearly;
  else if (p == "C" || p == tr("Custom") )  return Custom;
  else return Never;

}

QDate RecurrenceWidget::endDate() const
{
  return _dates->endDate();
}

int RecurrenceWidget::frequency() const
{
  return _frequency->value();
}

RecurrenceWidget::RecurrenceChangePolicy RecurrenceWidget::getChangePolicy()
{
  if (! modified())
    return IgnoreFuture;

  RecurrenceChangePolicy ans;

  XSqlQuery futureq;
  futureq.prepare("SELECT openRecurringItems(:parent_id, :parent_type,"
                  "                          NULL) AS open;");
  futureq.bindValue(":parent_id",   _parentId);
  futureq.bindValue(":parent_type", _parentType);
  futureq.exec();
  if (futureq.first())
  {
    int open = futureq.value("open").toInt();
    if (open < 0)
    {
      QMessageBox::critical(this, tr("Processing Error"),
                            storedProcErrorLookup("openRecurringItems", open));
      return NoPolicy;
    }
    else if (open == 0)
      return IgnoreFuture;
  }
  else if (futureq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Database Error"),
                          futureq.lastError().text());
    return NoPolicy;
  }

  switch (QMessageBox::question(this, tr("Change Open Events?"),
                                tr("<p>This event is part of a recurring "
                                   "series. Do you want to change all open "
                                   "events in this series?"),
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                QMessageBox::Yes))
  {
    case QMessageBox::Yes:      ans = ChangeFuture; break;
    case QMessageBox::No:       ans = IgnoreFuture; break;
    case QMessageBox::Cancel:
    default:                    ans = NoPolicy;     break;
  };

  return ans;
}

bool RecurrenceWidget::isRecurring() const
{
  return _recurring->isChecked();
}

int RecurrenceWidget::max() const
{
  return _max->value();
}

bool RecurrenceWidget::maxVisible() const
{
  return _max->isVisible();
}

bool RecurrenceWidget::modified() const
{
  bool returnVal = (isRecurring()!= _prevRecurring ||
                    period()     != _prevPeriod    ||
                    frequency()  != _prevFrequency ||
                    startDate()  != _prevStartDate ||
                    endDate()    != _prevEndDate   ||
                    max()        != _prevMax       ||
                    _parentId    != _prevParentId  ||
                    _parentType  != _prevParentType);

  return returnVal;
}

int RecurrenceWidget::parentId() const
{
  return _parentId;
}

QString RecurrenceWidget::parentType() const
{
  return _parentType;
}

RecurrenceWidget::RecurrencePeriod RecurrenceWidget::period() const
{
  return (RecurrencePeriod)(_period->id());
}

QString RecurrenceWidget::periodCode() const
{
  return _period->code();
}

bool RecurrenceWidget::save(bool externaltxn, RecurrenceChangePolicy cp, QString &message)
{
  if (! modified())
    return true;

  if (_parentId < 0 || _parentType.isEmpty())
  {
    message = tr("Could not save Recurrence information. The "
                 "parent object/event has not been set.");
    if (! externaltxn)
      QMessageBox::warning(this, tr("Missing Data"), message);
    return false;
  }

  if (! externaltxn && cp == NoPolicy)
  {
    cp = getChangePolicy();
    if (cp == NoPolicy)
      return false;
  }
  else if (externaltxn && cp == NoPolicy)
  {
    message = tr("You must choose how open events are to be handled");
    return false;
  }

  XSqlQuery rollbackq;
  if (! externaltxn)
  {
    XSqlQuery beginq("BEGIN;");
    rollbackq.prepare("ROLLBACK;");
  }

  XSqlQuery recurq;
  if (isRecurring())
  {
    if (_id > 0)
    {
      if (cp == ChangeFuture)
      {
        XSqlQuery futureq;
        futureq.prepare("SELECT splitRecurrence(:parent_id, :parent_type,"
                        "                       :splitdate) AS newrecurid;");
        futureq.bindValue(":parent_id",   _parentId);
        futureq.bindValue(":parent_type", _parentType);
        futureq.bindValue(":splitdate",   startDate());
        futureq.exec();
        if (futureq.first())
        {
          int result = futureq.value("newrecurid").toInt();
          if (result > 0)
          {
            _id = result;
            futureq.prepare("SELECT recur_parent_id"
                            "  FROM recur"
                            " WHERE recur_id=:recur_id;");
            futureq.bindValue(":recur_id", _id);
            futureq.exec();
            if (futureq.first())
              _parentId = futureq.value("recur_parent_id").toInt();
          }
          else if (result < 0)
          {
            message = storedProcErrorLookup("splitRecurrence", result);
            if (! externaltxn)
            {
              rollbackq.exec();
              QMessageBox::warning(this, tr("Procesing Error"), message);
            }
            return false;
          }
        }
        // one check for potentially 2 queries
        if (futureq.lastError().type() != QSqlError::NoError)
        {
          message = futureq.lastError().text();
          if (! externaltxn)
          {
            rollbackq.exec();
            QMessageBox::warning(this, tr("Database Error"), message);
          }
          return false;
        }
      }

      recurq.prepare("UPDATE recur SET"
                     "  recur_parent_id=:recur_parent_id,"
                     "  recur_parent_type=UPPER(:recur_parent_type),"
                     "  recur_period=:recur_period,"
                     "  recur_freq=:recur_freq,"
                     "  recur_start=:recur_start,"
                     "  recur_end=:recur_end,"
                     "  recur_max=:recur_max"
                     " WHERE (recur_id=:recurid)"
                     " RETURNING recur_id;");
      recurq.bindValue(":recurid", _id);
    }
    else
    {
      recurq.prepare("INSERT INTO recur ("
                     "  recur_parent_id,  recur_parent_type,"
                     "  recur_period,     recur_freq,"
                     "  recur_start,      recur_end,"
                     "  recur_max"
                     ") VALUES ("
                     "  :recur_parent_id, UPPER(:recur_parent_type),"
                     "  :recur_period,    :recur_freq,"
                     "  :recur_start,     :recur_end,"
                     "  :recur_max"
                     ") RETURNING recur_id;");
    }

    recurq.bindValue(":recur_parent_id",   _parentId);
    recurq.bindValue(":recur_parent_type", _parentType);
    recurq.bindValue(":recur_period",      periodCode());
    recurq.bindValue(":recur_freq",        frequency());
    recurq.bindValue(":recur_start",       startDate());
    recurq.bindValue(":recur_end",         endDate());
    recurq.bindValue(":recur_max",         max());
    recurq.exec();
    if (recurq.first())
    {
      _id = recurq.value("recur_id").toInt();
      _prevParentId   = _parentId;
      _prevParentType = _parentType;
      _prevEndDate    = endDate();
      _prevFrequency  = frequency();
      _prevMax        = max();
      _prevPeriod     = period();
      _prevRecurring  = isRecurring();
      _prevStartDate  = startDate();
    }
  }
  else // ! isRecurring()
  {
    recurq.prepare("DELETE FROM recur"
                   " WHERE ((recur_parent_id=:recur_parent_id)"
                   "    AND (recur_parent_type=:recur_parent_type));");
    recurq.bindValue(":recur_parent_id",   _parentId);
    recurq.bindValue(":recur_parent_type", _parentType);
    recurq.exec();
  }

  if (recurq.lastError().type() != QSqlError::NoError)
  {
    message = recurq.lastError().text();
    if (! externaltxn)
    {
      rollbackq.exec();
      QMessageBox::warning(this, tr("Database Error"), message);
    }
    return false;
  }

  if (cp == ChangeFuture)
  {
    int procresult = -1;
    QString procname = "deleteOpenRecurringItems";
    XSqlQuery cfq;
    cfq.prepare("SELECT deleteOpenRecurringItems(:parentId, :parentType,"
                "                                :splitdate) AS result;");
    cfq.bindValue(":parentId",   _parentId);
    cfq.bindValue(":parentType", _parentType);
    cfq.bindValue(":splitdate",  startDate());
    cfq.exec();
    if (cfq.first())
    {
      procresult = cfq.value("result").toInt();
      if (procresult >= 0)
      {
        QString procname = "createOpenRecurringItems";
        cfq.prepare("SELECT createRecurringItems(:parentId, :parentType)"
                    "       AS result;");
        cfq.bindValue(":parentId",   _parentId);
        cfq.bindValue(":parentType", _parentType);
        cfq.exec();
        if (cfq.first())
          procresult = cfq.value("result").toInt();
      }
    }

    // error handling for either 1 or 2 queries so no elseif
    if (procresult < 0)
    {
      message = storedProcErrorLookup(procname, procresult);
      if (! externaltxn)
      {
        rollbackq.exec();
        QMessageBox::critical(this, tr("Processing Error"), message);
      }
      return false;
    }
    else if (cfq.lastError().type() != QSqlError::NoError)
    {
      message = cfq.lastError().text();
      if (! externaltxn)
      {
        rollbackq.exec();
        QMessageBox::critical(this, tr("Database Error"), message);
      }
      return false;
    }
  }

  if (! isRecurring())
    clear();

  return true;
}

void RecurrenceWidget::set(bool recurring, int frequency, QString period, QDate startDate, QDate endDate, int max)
{
  setRecurring(recurring);
  setPeriod(period);
  setFrequency(frequency);
  setStartDate(startDate);
  setEndDate(endDate);
  setMax(max);

  _prevEndDate    = endDate;
  _prevFrequency  = frequency;
  _prevPeriod     = stringToPeriod(period);
  _prevRecurring  = recurring;
  _prevStartDate  = startDate;
  _prevMax        = max;
}

void RecurrenceWidget::setEndDate(QDate p)
{
  _dates->setEndDate(p.isValid() ? p : _eot);
}

void RecurrenceWidget::setFrequency(int p)
{
  _frequency->setValue(p);
}

void RecurrenceWidget::setMax(int p)
{
  _max->setValue(p);
}

void RecurrenceWidget::setMaxVisible(bool p)
{
  _max->setVisible(p);
  _maxLit->setVisible(p);
}

bool RecurrenceWidget::setParent(int pid, QString ptype)
{
  _parentId       = pid;
  _parentType     = ptype;

  XSqlQuery recurq;
  recurq.prepare("SELECT *"
                 "  FROM recur"
                 " WHERE ((recur_parent_id=:parentid)"
                 "   AND  (recur_parent_type=UPPER(:parenttype)));");
  recurq.bindValue(":parentid",   pid);
  recurq.bindValue(":parenttype", ptype);
  recurq.exec();
  if (recurq.first())
  {
    set(true,
        recurq.value("recur_freq").toInt(),
        recurq.value("recur_period").toString(),
        recurq.value("recur_start").toDate(),
        recurq.value("recur_end").toDate(),
        recurq.value("recur_max").toInt());
    _id             = recurq.value("recur_id").toInt();
    _prevParentId   = _parentId;
    _prevParentType = _parentType;
    return true;
  }
  else if (recurq.lastError().type() != QSqlError::NoError)
    QMessageBox::warning(this, tr("Database Error"),
                         recurq.lastError().text());

  // TODO? clear();
  return false;
}

void RecurrenceWidget::setPeriod(RecurrencePeriod p)
{
  _period->setId((int)p);
}

void RecurrenceWidget::setPeriod(QString p)
{
  _period->setCode(p);
}

void RecurrenceWidget::setRecurring(bool p)
{
  _recurring->setChecked(p);
}

void RecurrenceWidget::setStartDate(QDate p)
{
  _dates->setStartDate(p.isValid() ? p : QDate::currentDate());
}

void RecurrenceWidget::setStartDateVisible(bool p)
{
  _dates->setStartVisible(p);
}

QDate RecurrenceWidget::startDate() const
{
  return _dates->startDate();
}

bool RecurrenceWidget::startDateVisible() const
{
  return _dates->startVisible();
}
