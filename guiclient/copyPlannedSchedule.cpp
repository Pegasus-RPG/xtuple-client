/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyPlannedSchedule.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a copyPlannedSchedule as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
copyPlannedSchedule::copyPlannedSchedule(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_newDate, SIGNAL(newDate(const QDate&)), this, SLOT(sHandleDates()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));

  _pschheadid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
copyPlannedSchedule::~copyPlannedSchedule()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void copyPlannedSchedule::languageChange()
{
    retranslateUi(this);
}

enum SetResponse copyPlannedSchedule::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pschhead_id", &valid);
  if (valid)
  {
    _pschheadid = param.toInt();

    q.prepare("SELECT pschhead_number, pschhead_start_date"
              "  FROM pschhead"
              " WHERE (pschhead_id=:pschhead_id);");
    q.bindValue(":pschhead_id", _pschheadid);
    q.exec();
    if(q.first())
    {
      _scheduleName->setText(q.value("pschhead_number").toString());
      _oldDate->setDate(q.value("pschhead_start_date").toDate());
    }
  }

  return NoError;
}

void copyPlannedSchedule::sHandleDates()
{
  int offset = _oldDate->date().daysTo(_newDate->date());
  _offset->setText(tr("%1 day(s)").arg(offset));
}

void copyPlannedSchedule::sCopy()
{
  QString number = _number->text().trimmed().toUpper();

  if(number.isEmpty())
  {
    QMessageBox::information( this, tr("Incomplete Data"),
      tr("You have not specified a Number for the new schedule.") );
    return;
  }

  q.prepare( "SELECT pschhead_id "
             "  FROM pschhead "
             " WHERE (pschhead_number=:pschhead_number) "
             "LIMIT 1;" );
  q.bindValue(":pschhead_number", number);
  q.exec();
  if (q.first())
  {
    _number->setText("");
    _number->setFocus();
    QMessageBox::warning( this, tr("Invalid Number"),
      tr("The Number you specified for this Planned Schedule already exists.") );
    return;
  }

  if(!_newDate->isValid())
  {
    QMessageBox::warning( this, tr("Incomplete data"),
      tr("You must specify a new Start Date for the copied schedule.") );
    return;
  }

  q.prepare("SELECT copyPlannedSchedule(:pschhead_id, :number, :newdate) AS result;");
  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":number", number);
  q.bindValue(":newdate", _newDate->date());
  q.exec();

  accept();
}

