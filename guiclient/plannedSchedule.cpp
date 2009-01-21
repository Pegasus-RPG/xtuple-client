/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannedSchedule.h"

#include <QVariant>
#include <QMessageBox>

#include "plannedScheduleItem.h"

plannedSchedule::plannedSchedule(QWidget * parent, const char * name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _pschheadid = -1;
  _presaved = false;

  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));  
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));  
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));  
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sNumberChanged()));

  _list->addColumn(tr("#"),           _whsColumn,  Qt::AlignRight,  true,  "pschitem_linenumber"  );
  _list->addColumn(tr("Sched. Date"), _dateColumn, Qt::AlignCenter, true,  "pschitem_scheddate" );
  _list->addColumn(tr("Item Number"), -1,          Qt::AlignLeft,   true,  "item_number"   );
  _list->addColumn(tr("Qty"),         _qtyColumn,  Qt::AlignRight,  true,  "pschitem_qty"  );
  _list->addColumn(tr("Status"),      _whsColumn,  Qt::AlignCenter, true,  "pschitem_status" );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

}

plannedSchedule::~plannedSchedule()
{
}

void plannedSchedule::languageChange()
{
  retranslateUi(this);
}

enum SetResponse plannedSchedule::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pschhead_id", &valid);
  if(valid)
  {
    _pschheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
    {
      _mode = cNew;

      q.exec("SELECT nextval('pschhead_pschhead_id_seq') AS pschhead_id;");
      if(q.first())
        _pschheadid = q.value("pschhead_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }
      _new->setEnabled(false);

      _number->setFocus();
    }
    else if("edit" == mode)
    {
      _mode = cEdit;

      _number->setEnabled(false);
      _warehouse->setEnabled(false);

      _descrip->setFocus();
    }
    else if("view" == mode)
    {
      _mode = cView;

      _number->setEnabled(false);
      _descrip->setEnabled(false);
      _dates->setEnabled(false);
      _warehouse->setEnabled(false);
      _schedtype->setEnabled(false);

      _save->hide();
      _cancel->setFocus();
    }
  }

  return NoError;
}

void plannedSchedule::sSave()
{
  // TODO: Check and rules that need to be enforced here.
  q.prepare("SELECT pschitem_id "
            "  FROM pschitem "
            " WHERE ((pschitem_scheddate NOT BETWEEN :startDate AND :endDate) "
            "   AND  (pschitem_pschhead_id=:pschhead_id)); ");
  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":startDate", _dates->startDate());
  q.bindValue(":endDate", _dates->endDate());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning( this, tr("Scheduled Items Out of Date Range"),
      tr("One or more of the Scheduled Items is outside the specified date\n"
         "range for this Planned Schedule. Please fix this before continuing.") );
    return;
  }

  if (_schedtype->currentIndex() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Schedule"),
                           tr( "You must select a schedule type for this Schedule before creating it.\n" ) );
    _schedtype->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Schedule"),
                           tr( "You must enter a Start Date for this Schedule before creating it.\n" ) );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Schedule"),
                           tr( "You must enter an End Date for this Schedule before creating it.\n" ) );
    _dates->setFocus();
    return;
  }

  if (_number->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Schedule"),
                           tr( "You must enter a Schedule Number for this Schedule before creating it.\n" ) );
    _number->setFocus();
    return;
  }

  if(cNew == _mode && _presaved != true)
  {
    q.prepare("INSERT INTO pschhead "
              "      (pschhead_id, pschhead_number, pschhead_warehous_id,"
              "       pschhead_descrip,"
              "       pschhead_start_date, pschhead_end_date, pschhead_type) "
              "VALUES(:pschhead_id, :number, :warehous_id,"
              "       :descrip,"
              "       :startDate, :endDate, :schedtype);");
  }
  else
    q.prepare("UPDATE pschhead "
              "   SET pschhead_descrip=:descrip,"
              "       pschhead_start_date=:startDate,"
              "       pschhead_end_date=:endDate,"
              "       pschhead_type=:schedtype "
              " WHERE (pschhead_id=:pschhead_id); ");
  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":number", _number->text().trimmed().toUpper());
  q.bindValue(":descrip", _descrip->text());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":startDate", _dates->startDate());
  q.bindValue(":endDate", _dates->endDate());
  if (_schedtype->currentIndex() == 0)
    q.bindValue(":schedtype", "F");
  else if (_schedtype->currentIndex() == 1)
    q.bindValue(":schedtype", "N");
  else if (_schedtype->currentIndex() == 2)
    q.bindValue(":schedtype", "P");

  q.exec();

  disconnect(_number, SIGNAL(lostFocus()), this, SLOT(sNumberChanged()));
  accept();
}

void plannedSchedule::reject()
{
  if(cNew == _mode)
  {
    q.prepare("DELETE FROM pschitem WHERE pschitem_pschhead_id=:pschhead_id;"
              "DELETE FROM pschhead WHERE pschhead_id=:pschhead_id;");
    q.bindValue(":pschhead_id", _pschheadid);
    q.exec();
  }

  XDialog::reject();
}

void plannedSchedule::sNumberChanged()
{
  if(cNew != _mode)
    return;

  QString str = _number->text().trimmed().toUpper();

  if(str.isEmpty())
    return;

  q.prepare("SELECT pschhead_id "
            "  FROM pschhead "
            " WHERE (pschhead_number=:pschhead_number);");
  q.bindValue(":pschhead_number", str);
  q.exec();
  if(q.first())
  {
    _number->setText("");
    _number->setFocus();
    QMessageBox::warning( this, tr("Invalid Number"),
      tr("The Number you specified for this Planned Schedule already exists.") );
    
    return;
  }

  _new->setEnabled(true);
}

void plannedSchedule::sNew()
{
  if(cNew == _mode && _presaved != true)
  {
    q.prepare("INSERT INTO pschhead "
              "      (pschhead_id, pschhead_number, pschhead_warehous_id,"
              "       pschhead_start_date, pschhead_end_date) "
              "VALUES(:pschhead_id, :number, :warehous_id, startOfTime(), startOfTime());");
    q.bindValue(":pschhead_id", _pschheadid);
    q.bindValue(":number", _number->text().trimmed().toUpper());
    q.bindValue(":warehous_id", _warehouse->id());
    if(!q.exec())
    {
      QMessageBox::warning( this, tr("Error"),
        tr("There was an error updating the database.") );
      return;
    }

    _number->setEnabled(false);
    _warehouse->setEnabled(false);
    _presaved = true;
   
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("pschhead_id", _pschheadid);

  plannedScheduleItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedule::sEdit()
{
  ParameterList params;
  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");
  params.append("pschitem_id", _list->id());

  plannedScheduleItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedule::sCopy()
{
  if(cView == _mode)
    return;

  ParameterList params;
  params.append("mode", "copy");
  params.append("pschitem_id", _list->id());

  plannedScheduleItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedule::sDelete()
{
  q.prepare("UPDATE pschitem SET pschitem_status = 'X' WHERE (pschitem_id=:pschitem_id);");
  q.bindValue(":pschitem_id", _list->id());
  q.exec();
  sFillList();
}

void plannedSchedule::populate()
{
  q.prepare("SELECT pschhead_number,"
            "       pschhead_descrip,"
            "       pschhead_warehous_id,"
            "       pschhead_start_date,"
            "       pschhead_end_date,"
            "       pschhead_status,"
            "       pschhead_type "
            "  FROM pschhead "
            " WHERE (pschhead_id=:pschhead_id);");
  q.bindValue(":pschhead_id", _pschheadid);
  q.exec();
  if(q.first())
  {
    _number->setText(q.value("pschhead_number").toString());
    _descrip->setText(q.value("pschhead_descrip").toString());
    _warehouse->setId(q.value("pschhead_warehous_id").toInt());
    _dates->setStartDate(q.value("pschhead_start_date").toDate());
    _dates->setEndDate(q.value("pschhead_end_date").toDate());
    if (q.value("pschhead_status").toString() == "U")
      _status->setText("Unreleased");
    else if (q.value("pschhead_status").toString() == "R")
      _status->setText("Released");
    else
      _status->setText(q.value("pschhead_status").toString());
    if (q.value("pschhead_type").toString() == "F")
      _schedtype->setCurrentIndex(0);
    else if (q.value("pschhead_type").toString() == "N")
      _schedtype->setCurrentIndex(1);
    else if (q.value("pschhead_type").toString() == "P")
      _schedtype->setCurrentIndex(2);
    else
      _schedtype->setCurrentIndex(-1);


    _number->setEnabled(false);
    _warehouse->setEnabled(false);
    _new->setEnabled(true);

    sFillList();
  }
}

void plannedSchedule::sFillList()
{
  q.prepare("SELECT pschitem_id,"
            "       pschitem_linenumber,"
            "       pschitem_scheddate, "
            "       item_number,"
            "       pschitem_qty,"
            "       pschitem_status,"
            "       'qty' AS pschitem_qty_xtnumericrole "
            "  FROM pschitem JOIN "
            "       (itemsite JOIN item "
            "         ON (itemsite_item_id=item_id)) "
            "       ON (pschitem_itemsite_id=itemsite_id) "
            " WHERE (pschitem_pschhead_id=:pschhead_id) "
            " ORDER BY pschitem_linenumber;");
  q.bindValue(":pschhead_id", _pschheadid);
  q.exec();
  _list->populate(q);
}

