/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "rate.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

rate::rate(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _price->setValidator(omfgThis->moneyVal());
}

rate::~rate()
{
  // no need to delete child widgets, Qt does it all for us
}

void rate::languageChange()
{
  retranslateUi(this);
}

enum SetResponse rate::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("destination_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _destination->setId(param.toInt());
  }

  param = pParams.value("carrier_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _carrier->setId(param.toInt());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _destination->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _destination->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _destination->setEnabled(FALSE);
      _carrier->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _stops->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void rate::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('rate_rate_id_seq') AS _rate_id");
    if (q.first())
      _rateid = q.value("_rate_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO rate "
               "( rate_id, rate_destination_id, rate_carrier_id,"
               "  rate_rank,"
               "  rate_stops, rate_price,"
               "  rate_effective, rate_expires ) "
               "SELECT :rate_id, :rate_destination_id, :rate_carrier_id,"
               "       ( ( SELECT COALESCE(MAX(rate_rank), 0)"
               "           FROM rate"
               "           WHERE rate_destination_id=:rate_destination_id) + 1),"
               "       :rate_stops, :rate_price,"
               "       :rate_effective, :rate_expires );" );
  }

  else if (_mode == cEdit)
    q.prepare( "UPDATE rate "
               "SET rate_destination_id=:rate_destination_id, rate_carrier_id=:rate_carrier_id,"
               "    rate_stops=:rate_stops, rate_price=:rate_price,"
               "    rate_effective=:rate_effective, rate_expires=:rate_expires "
               "WHERE (rate_id=:rate_id);" );

  q.bindValue(":rate_id", _rateid);
  q.bindValue(":rate_destination_id", _destination->id());
  q.bindValue(":rate_carrier_id", _carrier->id());
  q.bindValue(":rate_stops", _stops->value());
  q.bindValue(":rate_price", _price->toDouble());
  q.bindValue(":rate_effective", _dates->startDate());
  q.bindValue(":rate_expires", _dates->endDate());
  q.exec();

  done(_rateid);
}

void rate::populate()
{
  q.prepare( "SELECT rate_destination_id, rate_carrier_id,"
             "       rate_stops, rate_price, rate_comments,"
             "       rate_effective, rate_expires "
             "FROM rate "
             "WHERE (rate_id=:rate_id);" );
  q.bindValue(":rate_id", _rateid);
  q.exec();
  if (q.first())
  {
    _destination->setId(q.value("rate_destination_id").toInt());
    _carrier->setId(q.value("rate_carrier_id").toInt());
    _stops->setValue(q.value("rate_stops").toInt());
    _price->setDouble(q.value("rate_price").toDouble());
    _dates->setStartDate(q.value("rate_effective").toDate());
    _dates->setEndDate(q.value("rate_expires").toDate());
  }
}

