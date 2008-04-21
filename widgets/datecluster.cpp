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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
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
 * Powered by PostBooks, an open source solution from xTuple
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


#include <QLabel>
#include <QDateTime>
#include <QRegExp>
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFocusEvent>

#include <xsqlquery.h>
#include <parameter.h>

#include "datecluster.h"

const int intMonthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

QString formatDate(const QDate &pDate)
{
  return QString().sprintf(_dateFormat, pDate.month(), pDate.day(), pDate.year());
}

int addCentury(int intYear)
{
  if (intYear < 50)
    return (intYear + 2000);
  else
    return (intYear + 1900);
}

void DLineEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addFieldMapping(this, _fieldName, QByteArray("date"));
}

void DLineEdit::validateDate()
{
  QString dateString = text().stripWhiteSpace();
  bool    isNumeric;

#ifdef OpenMFGGUIClient_h
  QDate today = ofmgThis->dbDate();
#else
  QDate today = QDate::currentDate();
#endif

  _valid = FALSE;
  dateString.toLong(&isNumeric);

//  If the string is "0" then it is today
  if (dateString == "0")
  {
    setDate(today, TRUE);
    _valid = TRUE;
  }

//  If the string starts with a "+" or a "-" and the rest is numeric
//  then this may be an offset
  else if ( (dateString[0] == '+') || (dateString[0] == '-') )
  {
    int offset = dateString.right(dateString.length() - 1).toInt(&isNumeric);
    if (isNumeric)
    {
      setDate(today.addDays(offset * ((dateString.left(1) == "-") ? -1 : 1)), TRUE);
      _valid = TRUE;
    }
  }

//  If the string starts with a "#" and the rest is numeric then this may be a Julian date
  else if (dateString[0] == '#')
  {
    int offset = dateString.right(dateString.length() - 1).toInt(&isNumeric);
    if (isNumeric)
    {
      setDate(QDate(today.year(), 1, 1).addDays(offset - 1), TRUE);
      _valid =  TRUE;
    }
}

//  If the string is pure numeric and 1 or 2 characters then is may be the date in month
  else if (dateString.length() <= 2)
  {
    int offset = dateString.toInt(&isNumeric);
    if (isNumeric)
    {
      if (offset > today.daysInMonth())
        offset = today.daysInMonth();
 
      setDate(QDate(today.year(), today.month(), 1).addDays(offset - 1), TRUE);
      _valid = TRUE;
    }
  }

//  If the string is pure numeric and either 5 or 6 character in length then
//  it may be a short date
  else if ((isNumeric) && ((dateString.length() == 5) || (dateString.length() == 6)))
  {
    int day;
    int month;
    int year;

    if (dateString.length() == 5)
    {
      day   = dateString.mid(1, 2).toInt();
      month = dateString.left(1).toInt();
      year  = dateString.right(2).toInt();
      year  = addCentury(year);
    }
    else
    {
      day   = dateString.mid(2, 2).toInt();
      month = dateString.left(2).toInt();
      year  = dateString.right(2).toInt();
      year  = addCentury(year);
    }

    if (fixMonthEnd(&day, month, year))
    {
      setDate(QDate(year, month, day), TRUE);
      _valid = TRUE;
    }
  }

//  If the field images to #/#/##, #/##/## , ##/#/##, or ##/##/##, it may be a 2 year conventional format
//  If the field images to #/#/####, #/##/####, ##/#/####, or ##/##/####, it may be a 4 year conventional format
  else
  {
    QRegExp dateRegExp("[0-9]{1,2}[/][0-9]{1,2}[/][0-9]{2,4}");
    QRegExpValidator dateValidator(dateRegExp, this);
    int position = 0;

    if (dateValidator.validate(dateString, position))
    {
      int sep1 = dateString.find('/');
      int sep2 = dateString.find('/', (sep1 + 1));
      int day;
      int month;
      int year;

      if (sep1 != -1)
      {
        month = dateString.left(sep1).toInt();

        if (sep2 != -1)
        {
          day = dateString.mid((sep1 + 1), (sep2 - sep1 - 1)).toInt();
          year = dateString.right((dateString.length() - (sep2 + 1))).toInt();

          if (year < 100)
            year = addCentury(year);

          if (fixMonthEnd(&day, month, year))
          {
            setDate(QDate(year, month, day), TRUE);
            _valid = TRUE;
          }
        }
      }
    }
  }

  if (!_valid)
    setText("");

  _parsed = TRUE;
}

bool DLineEdit::fixMonthEnd(int *pDay, int pMonth, int pYear)
{
//  Fixup the Date
  if (pMonth == 2)
  {
    bool leapYear = FALSE;

    if ((pYear % 4) == 0)
    {
      if ((pYear % 400) == 0)
        leapYear = TRUE;
      else if ((pYear % 100) != 0)
        leapYear = TRUE;
    }

    if (leapYear)
    {
      if (*pDay > 29)
        *pDay = 29;
    }
    else if (*pDay > 28)
      *pDay = 28;
  }
  else if (*pDay > intMonthDays[(pMonth - 1)])
    *pDay = intMonthDays[(pMonth - 1)];

  return TRUE;
}

DLineEdit::DLineEdit(QWidget *parent, const char *name) :
  XLineEdit(parent, name)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMaximumWidth(100);

  _allowNull   = FALSE;
  _parsed      = FALSE;
  _nullString  = QString::null;
  _valid       = FALSE;
  _null        = FALSE;
}

void DLineEdit::setNull()
{
  if (_allowNull)
  {
    _valid  = TRUE;
    _null   = TRUE;
    _parsed = TRUE;
    setText(_nullString);

    _currentDate = _nullDate;
    _dateString.sprintf(_dateFormat, _nullDate.month(), _nullDate.day(), _nullDate.year());
  }
  else
  {
    _valid = FALSE;
    _null = FALSE;
    _parsed = TRUE;
    clear();
  }
}

void DLineEdit::setDate(const QDate &pDate, bool pAnnounce)
{
  setDate(pDate);

  if (pAnnounce)
    emit newDate(_currentDate);
}

void DLineEdit::setDate(const QDate &pDate)
{
  if (!pDate.isNull())
  {
    _currentDate = pDate;
    _dateString = formatDate(pDate);

    if ((_allowNull) && (_currentDate == _nullDate))
    {
      setText(_nullString);
      _null  = TRUE;
    }
    else
    {
      setText(_dateString);
      _null  = FALSE;
    }

    _valid = TRUE;
  }

  else if (_allowNull)
    setNull();
}

QDate DLineEdit::date()
{
  if (!_parsed)
    parseDate();

  if (!_valid)
    return _nullDate;

  return _currentDate;
}

QString DLineEdit::dateString()
{
  if (!_parsed)
    parseDate();

  return _dateString;
}

bool DLineEdit::isNull()
{
  if (!_parsed)
    parseDate();

  return _null;
}

bool DLineEdit::isValid()
{
  if (!_parsed)
    parseDate();

  return _valid;
}

void DLineEdit::parseDate()
{
  _parsed = TRUE;

  QString string = text().stripWhiteSpace();

  if ((_allowNull) && (string == _nullString))
    setNull();
  else if (string.length() == 0)
  {
    if (_allowNull)
    {
      setNull();

      emit newDate(_nullDate);
    }
    else
    {
      setText("");
      _valid = FALSE;
      _null  = FALSE;
    }
  }
  else
  {
    _null = FALSE;

    validateDate();
  }
}

void DLineEdit::focusOutEvent(QFocusEvent *event)
{
  if (_parsed == FALSE)
    parseDate();

  QLineEdit::focusOutEvent(event);
}


DateCluster::DateCluster(QWidget *pParent, const char *pName) : QWidget(pParent)
{
  if(pName)
    setObjectName(pName);

  QSizePolicy tsizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
  tsizePolicy.setHorizontalStretch(0);
  tsizePolicy.setVerticalStretch(0);
  tsizePolicy.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(tsizePolicy);

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(5);
  mainLayout->setMargin(0);
  mainLayout->setObjectName(QString::fromUtf8("mainLayout"));

  QVBoxLayout *literalLayout = new QVBoxLayout();
  literalLayout->setSpacing(5);
  literalLayout->setMargin(0);
  literalLayout->setObjectName(QString::fromUtf8("literalLayout"));

  _startDateLit = new QLabel(tr("Start Date:"), this, "_startDateLit");
  _startDateLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  literalLayout->addWidget(_startDateLit);

  _endDateLit = new QLabel(tr("End Date:"), this, "_endDateLit");
  _endDateLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  literalLayout->addWidget(_endDateLit);

  mainLayout->addLayout(literalLayout);

  QVBoxLayout *dataLayout = new QVBoxLayout();
  dataLayout->setSpacing(5);
  dataLayout->setMargin(0);
  dataLayout->setObjectName(QString::fromUtf8("dataLayout"));

  _startDate = new DLineEdit(this, "_startDate");
  _startDate->setMaximumWidth(100);
  dataLayout->addWidget(_startDate);

  _endDate = new DLineEdit(this, "_endDate");
  _endDate->setMaximumWidth(100);
  dataLayout->addWidget(_endDate);

  mainLayout->addLayout(dataLayout);

  _startDateLit->setBuddy(_startDate);
  _endDateLit->setBuddy(_endDate);

  connect(_startDate, SIGNAL(newDate(const QDate &)), this, SIGNAL(updated()));
  connect(_endDate, SIGNAL(newDate(const QDate &)), this, SIGNAL(updated()));

  //setTabOrder(_startDate, _endDate);
  //setTabOrder(_endDate, _startDate);
  setFocusProxy(_startDate);
}

void DateCluster::setStartNull(const QString &pString, const QDate &pDate, bool pSetNull)
{
  _startDate->setAllowNullDate(TRUE);
  _startDate->setNullString(pString);
  _startDate->setNullDate(pDate);

  if (pSetNull)
    _startDate->setNull();
}

void DateCluster::setEndNull(const QString &pString, const QDate &pDate, bool pSetNull)
{
  _endDate->setAllowNullDate(TRUE);
  _endDate->setNullString(pString);
  _endDate->setNullDate(pDate);

  if (pSetNull)
    _endDate->setNull();
}

void DateCluster::setStartCaption(const QString &pString)
{
  _startDateLit->setText(pString);
}

void DateCluster::setEndCaption(const QString &pString)
{
  _endDateLit->setText(pString);
}

void DateCluster::appendValue(ParameterList &pParams)
{
  pParams.append("startDate", _startDate->date());
  pParams.append("endDate", _endDate->date());
}

void DateCluster::bindValue(XSqlQuery &pQuery)
{
  pQuery.bindValue(":startDate", _startDate->date());
  pQuery.bindValue(":endDate", _endDate->date());
}

