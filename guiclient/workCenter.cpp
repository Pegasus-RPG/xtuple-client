/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "workCenter.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

workCenter::workCenter(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  QButtonGroup* _runRateGroupInt = new QButtonGroup(this);
  _runRateGroupInt->addButton(_runSpecifyRate);
  _runRateGroupInt->addButton(_runUseSelectedRate);

  QButtonGroup* _setupRateGroupInt = new QButtonGroup(this);
  _setupRateGroupInt->addButton(_setupSpecifyRate);
  _setupRateGroupInt->addButton(_setupUseSelectedRate);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_numOfMachines, SIGNAL(valueChanged(const int&)), this, SLOT(sPopulateOverheadRate()));
  connect(_numOfPeople, SIGNAL(valueChanged(const int&)), this, SLOT(sPopulateOverheadRate()));
  connect(_overheadPerLaborHour, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
  connect(_overheadPerMachHour, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
  connect(_overheadPrcntOfLabor, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
  connect(_runRate, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
  connect(_runSpecifyRate, SIGNAL(toggled(bool)), _runRate, SLOT(setEnabled(bool)));
  connect(_runUseSelectedRate, SIGNAL(toggled(bool)), _stdRunRate, SLOT(setEnabled(bool)));
  connect(_runUseSelectedRate, SIGNAL(clicked()), this, SLOT(sPopulateRunRate()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_setupRate, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
  connect(_setupSpecifyRate, SIGNAL(toggled(bool)), _setupRate, SLOT(setEnabled(bool)));
  connect(_setupUseSelectedRate, SIGNAL(toggled(bool)), _stdSetupRate, SLOT(setEnabled(bool)));
  connect(_setupUseSelectedRate, SIGNAL(clicked()), this, SLOT(sPopulateSetupRate()));
  connect(_stdRunRate, SIGNAL(newID(int)), this, SLOT(sPopulateRunRate()));
  connect(_stdSetupRate, SIGNAL(newID(int)), this, SLOT(sPopulateSetupRate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
  
  _runRate->setValidator(omfgThis->runTimeVal());
  _setupRate->setValidator(omfgThis->runTimeVal());
  _overheadPrcntOfLabor->setValidator(omfgThis->percentVal());
  _overheadPerLaborHour->setValidator(omfgThis->costVal());
  _overheadPerMachHour->setValidator(omfgThis->costVal());
  _overheadRate->setPrecision(omfgThis->costVal());
  _overheadPerUnit->setValidator(omfgThis->costVal());
  _avgSetup->setValidator(omfgThis->runTimeVal());
  _dailyCapacity->setValidator(omfgThis->runTimeVal());
  _efficiencyFactor->setValidator(omfgThis->percentVal());
    
  _lbrrate.exec( "SELECT lbrrate_id, lbrrate_code, lbrrate_rate "
                 "FROM lbrrate "
                 "ORDER BY lbrrate_code;" );

  _stdSetupRate->populate(_lbrrate);
  if (!_stdSetupRate->count())
    _setupUseSelectedRate->setEnabled(FALSE);

  _stdRunRate->populate(_lbrrate);
  if (!_stdRunRate->count())
    _runUseSelectedRate->setEnabled(FALSE);

  _setupRate->setValidator(omfgThis->moneyVal());
  _runRate->setValidator(omfgThis->moneyVal());
  _overheadPrcntOfLabor->setValidator(omfgThis->percentVal());
  _overheadPerLaborHour->setValidator(omfgThis->moneyVal());
  _overheadPerMachHour->setValidator(omfgThis->moneyVal());
  _overheadPerUnit->setValidator(omfgThis->moneyVal());

  _setupType->setCurrentIndex(-1);

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  sPopulateLocations();
}

workCenter::~workCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void workCenter::languageChange()
{
  retranslateUi(this);
}

enum SetResponse workCenter::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wrkcnt_id", &valid);
  if (valid)
  {
    _wrkcntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cNew;
      _code->clear();
      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _department->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _wipLocation->setEnabled(FALSE);
      _numOfMachines->setEnabled(FALSE);
      _numOfPeople->setEnabled(FALSE);
      _setupSpecifyRate->setEnabled(FALSE);
      _setupUseSelectedRate->setEnabled(FALSE);
      _setupRate->setEnabled(FALSE);
      _stdSetupRate->setEnabled(FALSE);
      _setupType->setEnabled(FALSE);
      _runSpecifyRate->setEnabled(FALSE);
      _runUseSelectedRate->setEnabled(FALSE);
      _runRate->setEnabled(FALSE);
      _stdRunRate->setEnabled(FALSE);
      _overheadPrcntOfLabor->setEnabled(FALSE);
      _overheadPerLaborHour->setEnabled(FALSE);
      _overheadPerMachHour->setEnabled(FALSE);
      _overheadPerUnit->setEnabled(FALSE);
      _avgQueueDays->setEnabled(FALSE);
      _avgSetup->setEnabled(FALSE);
      _dailyCapacity->setEnabled(FALSE);
      _efficiencyFactor->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void workCenter::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT wrkcnt_id "
               "FROM wrkcnt "
               "WHERE (UPPER(wrkcnt_code)=UPPER(:wrkcnt_code));" );
    q.bindValue(":wrkcnt_code", _code->text());
    q.exec();
    if (q.first())
    {
      _wrkcntid = q.value("wrkcnt_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void workCenter::sSave()
{
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Work Center"),
                           tr("You must select a Site for this Work Center before you may save it.") );
    _warehouse->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('wrkcnt_wrkcnt_id_seq') AS _wrkcnt_id;");
    if (q.first())
      _wrkcntid = q.value("_wrkcnt_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO wrkcnt "
               "( wrkcnt_id, wrkcnt_code, wrkcnt_descrip,"
               "  wrkcnt_dept_id, wrkcnt_warehous_id,"
               "  wrkcnt_nummachs, wrkcnt_numpeople,"
               "  wrkcnt_setup_lbrrate_id, wrkcnt_setuprate,"
               "  wrkcnt_run_lbrrate_id, wrkcnt_runrate,"
               "  wrkcnt_brd_prcntlbr, wrkcnt_brd_rateperlbrhr,"
               "  wrkcnt_brd_ratepermachhr, wrkcnt_brd_rateperunitprod,"
               "  wrkcnt_avgqueuedays, wrkcnt_avgsutime,"
               "  wrkcnt_dailycap, wrkcnt_caploaduom, wrkcnt_efficfactor,"
               "  wrkcnt_comments, wrkcnt_wip_location_id) "
               "VALUES "
               "( :wrkcnt_id, :wrkcnt_code, :wrkcnt_descrip,"
               "  :wrkcnt_dept_id, :wrkcnt_warehous_id,"
               "  :wrkcnt_nummachs, :wrkcnt_numpeople,"
               "  :wrkcnt_setup_lbrrate_id, :wrkcnt_setuprate,"
               "  :wrkcnt_run_lbrrate_id, :wrkcnt_runrate,"
               "  :wrkcnt_brd_prcntlbr, :wrkcnt_brd_rateperlbrhr,"
               "  :wrkcnt_brd_ratepermachhr, :wrkcnt_brd_rateperunitprod,"
               "  :wrkcnt_avgqueuedays, :wrkcnt_avgsutime,"
               "  :wrkcnt_dailycap, :wrkcnt_caploaduom, :wrkcnt_efficfactor,"
               "  :wrkcnt_comments, :wrkcnt_wip_location_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE wrkcnt "
               "SET wrkcnt_code=:wrkcnt_code, wrkcnt_descrip=:wrkcnt_descrip,"
               "    wrkcnt_dept_id=:wrkcnt_dept_id, wrkcnt_warehous_id=:wrkcnt_warehous_id,"
               "    wrkcnt_nummachs=:wrkcnt_nummachs, wrkcnt_numpeople=:wrkcnt_numpeople,"
               "    wrkcnt_setup_lbrrate_id=:wrkcnt_setup_lbrrate_id, wrkcnt_setuprate=:wrkcnt_setuprate,"
               "    wrkcnt_run_lbrrate_id=:wrkcnt_run_lbrrate_id, wrkcnt_runrate=:wrkcnt_runrate,"
               "    wrkcnt_brd_prcntlbr=:wrkcnt_brd_prcntlbr, wrkcnt_brd_rateperlbrhr=:wrkcnt_brd_rateperlbrhr,"
               "    wrkcnt_brd_ratepermachhr=:wrkcnt_brd_ratepermachhr, wrkcnt_brd_rateperunitprod=:wrkcnt_brd_rateperunitprod,"
               "    wrkcnt_avgqueuedays=:wrkcnt_avgqueuedays, wrkcnt_avgsutime=:wrkcnt_avgsutime,"
               "    wrkcnt_dailycap=:wrkcnt_dailycap, wrkcnt_caploaduom=:wrkcnt_caploaduom, wrkcnt_efficfactor=:wrkcnt_efficfactor,"
               "    wrkcnt_comments=:wrkcnt_comments, wrkcnt_wip_location_id=:wrkcnt_wip_location_id "
               "WHERE (wrkcnt_id=:wrkcnt_id);" );

  q.bindValue(":wrkcnt_id", _wrkcntid);
  q.bindValue(":wrkcnt_code", _code->text().trimmed());
  q.bindValue(":wrkcnt_descrip", _description->text().trimmed());
  if(_department->id() != -1)
    q.bindValue(":wrkcnt_dept_id", _department->id());
  q.bindValue(":wrkcnt_warehous_id", _warehouse->id());
  q.bindValue(":wrkcnt_wip_location_id", _wipLocation->id());
  q.bindValue(":wrkcnt_nummachs", _numOfMachines->value());
  q.bindValue(":wrkcnt_numpeople", _numOfPeople->value());
  q.bindValue(":wrkcnt_setup_lbrrate_id", ((_setupUseSelectedRate->isChecked()) ? _stdSetupRate->id() : -1));
  q.bindValue(":wrkcnt_setuprate", _setupRate->toDouble());
  q.bindValue(":wrkcnt_run_lbrrate_id", ((_runUseSelectedRate->isChecked()) ? _stdRunRate->id() : -1));
  q.bindValue(":wrkcnt_runrate", _runRate->toDouble());
  q.bindValue(":wrkcnt_brd_prcntlbr", (_overheadPrcntOfLabor->toDouble() / 100.0));
  q.bindValue(":wrkcnt_brd_rateperlbrhr", _overheadPerLaborHour->toDouble());
  q.bindValue(":wrkcnt_brd_ratepermachhr", _overheadPerMachHour->toDouble());
  q.bindValue(":wrkcnt_brd_rateperunitprod", _overheadPerUnit->toDouble());
  q.bindValue(":wrkcnt_avgqueuedays", _avgQueueDays->value());
  q.bindValue(":wrkcnt_avgsutime", _avgSetup->toDouble());
  q.bindValue(":wrkcnt_dailycap", _dailyCapacity->toDouble());
  q.bindValue(":wrkcnt_efficfactor", (_efficiencyFactor->toDouble() / 100.0));
  q.bindValue(":wrkcnt_comments", _comments->toPlainText());

  if (_setupType->currentIndex() == 0)
    q.bindValue(":wrkcnt_caploaduom", "M");
  else
    q.bindValue(":wrkcnt_caploaduom", "L");

  q.exec();

  omfgThis->sWorkCentersUpdated();
  close();
}

void workCenter::sPopulateOverheadRate()
{
  double runRate;

  if (_runUseSelectedRate->isChecked())
  {
    _lbrrate.findFirst("lbrrate_id", _stdRunRate->id());
    runRate = _lbrrate.value("lbrrate_rate").toDouble();
  }
  else
    runRate = _runRate->toDouble();

  _overheadRate->setText(formatCost( ((_numOfPeople->value() * runRate) * _overheadPrcntOfLabor->toDouble() / 100) +
                                     (_numOfPeople->value() * _overheadPerLaborHour->toDouble()) +
                                     (_numOfMachines->value() * _overheadPerMachHour->toDouble()) ) );
}

void workCenter::sPopulateSetupRate()
{
  XSqlQuery rate;
  rate.prepare( "SELECT formatMoney(lbrrate_rate) AS rate "
                "FROM lbrrate "
                "WHERE (lbrrate_id=:lbrrate_id);" );
  rate.bindValue(":lbrrate_id", _stdSetupRate->id());
  rate.exec();
  if (rate.first())
    _setupRate->setText(rate.value("rate"));
}

void workCenter::sPopulateRunRate()
{
  XSqlQuery rate;
  rate.prepare( "SELECT lbrrate_rate AS rate "
                "FROM lbrrate "
                "WHERE (lbrrate_id=:lbrrate_id);" );
  rate.bindValue(":lbrrate_id", _stdRunRate->id());
  rate.exec();
  if (rate.first())
    _runRate->setText(rate.value("rate").toDouble());
}

void workCenter::populate()
{
  q.prepare( "SELECT wrkcnt_code, wrkcnt_descrip, COALESCE(wrkcnt_dept_id, -1) AS wrkcnt_dept_id,"
             "       wrkcnt_warehous_id, wrkcnt_caploaduom, wrkcnt_comments,"
             "       wrkcnt_nummachs, wrkcnt_numpeople,"
             "       wrkcnt_brd_prcntlbr * 100 AS f_brd_prcntlbr,"
             "       wrkcnt_brd_rateperlbrhr,"
             "       wrkcnt_brd_ratepermachhr,"
             "       wrkcnt_brd_rateperunitprod,"
             "       wrkcnt_avgqueuedays,"
             "       wrkcnt_avgsutime,"
             "       wrkcnt_dailycap,"
             "       wrkcnt_efficfactor * 100 AS f_efficfactor,"
             "       wrkcnt_setuprate,"
             "       wrkcnt_runrate,"
             "       wrkcnt_setup_lbrrate_id, wrkcnt_run_lbrrate_id,"
             "       wrkcnt_wip_location_id "
             "FROM wrkcnt "
             "WHERE (wrkcnt_id=:wrkcnt_id);" );
  q.bindValue(":wrkcnt_id", _wrkcntid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("wrkcnt_code"));
    _description->setText(q.value("wrkcnt_descrip"));
    _department->setId(q.value("wrkcnt_dept_id").toInt());
    _numOfMachines->setValue(q.value("wrkcnt_nummachs").toInt());
    _numOfPeople->setValue(q.value("wrkcnt_numpeople").toInt());
    _overheadPrcntOfLabor->setText(q.value("f_brd_prcntlbr").toDouble());
    _overheadPerLaborHour->setText(q.value("wrkcnt_brd_rateperlbrhr").toDouble());
    _overheadPerMachHour->setText(q.value("wrkcnt_brd_ratepermachhr").toDouble());
    _overheadPerUnit->setText(q.value("wrkcnt_brd_rateperunitprod").toDouble());
    _avgQueueDays->setValue(q.value("wrkcnt_avgqueuedays").toInt());
    _avgSetup->setText(q.value("wrkcnt_avgsutime").toDouble());
    _dailyCapacity->setText(q.value("wrkcnt_dailycap").toDouble());
    _efficiencyFactor->setText(q.value("f_efficfactor").toDouble());
    _warehouse->setId(q.value("wrkcnt_warehous_id").toInt());
    _wipLocation->setId(q.value("wrkcnt_wip_location_id").toInt());

    if (q.value("wrkcnt_setup_lbrrate_id").toInt() != -1)
    {
      _setupUseSelectedRate->setChecked(TRUE);
      _stdSetupRate->setId(q.value("wrkcnt_setup_lbrrate_id").toInt());
    }
    else
      _setupSpecifyRate->setChecked(TRUE);

    _setupRate->setText(q.value("wrkcnt_setuprate").toDouble());
    sPopulateOverheadRate();

    if (q.value("wrkcnt_run_lbrrate_id").toInt() != -1)
    {
      _runUseSelectedRate->setChecked(TRUE);
      _stdRunRate->setId(q.value("wrkcnt_run_lbrrate_id").toInt());
    }
    else
      _runSpecifyRate->setChecked(TRUE);

    _runRate->setText(q.value("wrkcnt_runrate").toDouble());
    sPopulateOverheadRate();

    if (q.value("wrkcnt_caploaduom").toString() == "M")
      _setupType->setCurrentIndex(0);
    else if (q.value("wrkcnt_caploaduom").toString() == "L")
      _setupType->setCurrentIndex(1);
    else _setupType->setCurrentIndex(-1);

    _comments->setText(q.value("wrkcnt_comments").toString());
  }
}

void workCenter::sPopulateLocations()
{
  XSqlQuery loc;
  loc.prepare("SELECT location_id, formatLocationName(location_id) AS locationname"
              "  FROM location"
              " WHERE ( (NOT location_restrict)"
              "   AND   (location_warehous_id=:warehous_id) ) "
              "ORDER BY locationname;");
  loc.bindValue(":warehous_id", _warehouse->id());
  loc.exec();
  _wipLocation->populate(loc);
}

