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

//_period->append(Never,        tr("Never"),  "");
//_period->append(Minutely,     tr("Minutes"), "m");
//_period->append(Hourly,       tr("Hours"),   "H");
  _period->append(Daily,        tr("Days"),    "D");
  _period->append(Weekly,       tr("Weeks"),   "W");
  _period->append(Monthly,      tr("Months"),  "M");
  _period->append(Yearly,       tr("Years"),   "Y");
//_period->append(Custom,       tr("Custom"), "C");

  _period->setCode("W");

  _dates->setStartCaption(tr("From:"));
  _dates->setEndCaption(tr("Until:"));
  _dates->setStartNull(tr("Today"), QDate::currentDate(), true);
  XSqlQuery eotq("SELECT endOfTime() AS eot;");
  if (eotq.first())
    _dates->setEndNull(tr("Forever"), eotq.value("eot").toDate(), true);
  else
  {
    qWarning("RecurrenceWidget could not get endOfTime()");
    _dates->setEndNull(tr("Today"), QDate::currentDate(), true);
  }
}

RecurrenceWidget::~RecurrenceWidget()
{
}

void RecurrenceWidget::languageChange()
{
}

QDate RecurrenceWidget::endDate() const
{
  return _dates->endDate();
}

int RecurrenceWidget::frequency() const
{
  return _frequency->value();
}

bool RecurrenceWidget::isRecurring() const
{
  return _recurring->isChecked();
}

RecurrenceWidget::RecurrencePeriod RecurrenceWidget::period() const
{
  return (RecurrencePeriod)(_period->id());
}

QString RecurrenceWidget::periodCode() const
{
  return _period->code();
}

void RecurrenceWidget::set(bool recurring, int frequency, QString period, QDate startDate, QDate endDate)
{
  setRecurring(recurring);
  setPeriod(period);
  setFrequency(frequency);
  setStartDate(startDate);
  setEndDate(endDate);
}

void RecurrenceWidget::setEndDate(QDate p)
{
  _dates->setEndDate(p);
}

void RecurrenceWidget::setFrequency(int p)
{
  _frequency->setValue(p);
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
  _dates->setStartDate(p);
}

QDate RecurrenceWidget::startDate() const
{
  return _dates->startDate();
}
