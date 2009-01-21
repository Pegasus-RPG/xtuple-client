/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "whseCalendar.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a whseCalendar as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
whseCalendar::whseCalendar(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _description->setFocus();
}

whseCalendar::~whseCalendar()
{
  // no need to delete child widgets, Qt does it all for us
}

void whseCalendar::languageChange()
{
  retranslateUi(this);
}

enum SetResponse whseCalendar::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("whsecal_id", &valid);
  if (valid)
  {
    _whsecalid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(false);
      _description->setEnabled(false);
      _dates->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
      _activeGroup->setEnabled(false);

      _close->setFocus();
    }
  }

  return NoError;
}

void whseCalendar::sSave()
{
  if (!_dates->allValid())
  {
    QMessageBox::critical( this, tr("Enter Start/End Date"),
                           tr("You must enter a valid start/end date for this Site Calendar before saving it.") );
    _dates->setFocus();
    return;
  }

  q.prepare("SELECT whsecal_id"
            "  FROM whsecal"
            " WHERE((whsecal_id != :whsecal_id)"
            "   AND (whsecal_warehous_id=:warehous_id)"
            "   AND (whsecal_effective=:startDate)"
            "   AND (whsecal_expires=:endDate))");
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":whsecal_id", _whsecalid);
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Date for Site Already Set"),
      tr("The Dates specified for the Site is already in the system. Please edit that record or change you dates."));
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('whsecal_whsecal_id_seq') AS _whsecal_id;");
    if (q.first())
      _whsecalid = q.value("_whsecal_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO whsecal "
               "( whsecal_id, whsecal_warehous_id, whsecal_descrip,"
               "  whsecal_effective, whsecal_expires, whsecal_active ) "
               "VALUES "
               "( :whsecal_id, :warehous_id, :whsecal_descrip,"
               "  :startDate, :endDate, :whsecal_active );" );
  }
  else
    q.prepare( "UPDATE whsecal "
               "SET whsecal_warehous_id=:warehous_id,"
               "    whsecal_descrip=:whsecal_descrip,"
               "    whsecal_effective=:startDate,"
               "    whsecal_expires=:endDate,"
               "    whsecal_active=:whsecal_active "
               "WHERE (whsecal_id=:whsecal_id);" );

  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":whsecal_id", _whsecalid);
  q.bindValue(":whsecal_descrip", _description->text());
  q.bindValue(":whsecal_active", QVariant(_active->isChecked()));
  q.exec();

  done(_whsecalid);
}

void whseCalendar::populate()
{
  q.prepare( "SELECT whsecal_warehous_id, whsecal_descrip,"
             "       whsecal_effective, whsecal_expires,"
             "       whsecal_active "
             "FROM whsecal "
             "WHERE (whsecal_id=:whsecal_id);" );
  q.bindValue(":whsecal_id", _whsecalid);
  q.exec();
  if (q.first())
  {
    int warehousid = q.value("whsecal_warehous_id").toInt();
    if(warehousid < 1)
      _warehouse->setAll();
    else
      _warehouse->setId(warehousid);

    _description->setText(q.value("whsecal_descrip").toString());
    _dates->setStartDate(q.value("whsecal_effective").toDate());
    _dates->setEndDate(q.value("whsecal_expires").toDate());

    if(q.value("whsecal_active").toBool())
      _active->setChecked(true);
    else
      _inactive->setChecked(true);
  }
}

