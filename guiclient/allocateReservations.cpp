/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "allocateReservations.h"

#include <QVariant>
#include <QMessageBox>

#include "submitAction.h"

allocateReservations::allocateReservations(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _submit = _buttonBox->addButton(tr("Schedule"), QDialogButtonBox::ActionRole);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sAllocate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sCustomerSelected()));

  _customerTypes->setType(XComboBox::CustomerTypes);
}

allocateReservations::~allocateReservations()
{
  // no need to delete child widgets, Qt does it all for us
}

void allocateReservations::languageChange()
{
  retranslateUi(this);
}

void allocateReservations::sAllocate()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"), tr("You must specify a valid date range."));
    return;
  }

  XSqlQuery allocateRes;
  allocateRes.prepare("SELECT reserveAllSo(:addpackinglist, :startDate, :endDate, :cust_id, :shipto_id, :custtype_id, :custtype_pattern) AS result;");
  allocateRes.bindValue(":addpackinglist", _addPackingList->isChecked());
  _dates->bindValue(allocateRes);
  if (_selectedCustomer->isChecked())
    allocateRes.bindValue(":cust_id", _cust->id());
  if (_selectedCustomerShipto->isChecked())
    allocateRes.bindValue(":shipto_id", _customerShipto->id());
  if (_selectedCustomerType->isChecked())
    allocateRes.bindValue(":custtype_id", _customerTypes->id());
  if (_customerTypePattern->isChecked())
    allocateRes.bindValue(":custtype_pattern", _customerType->text());

  allocateRes.exec();

  accept();
}

void allocateReservations::sSubmit()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"), tr("You must specify a valid date range."));
    return;
  }

  ParameterList params;

  params.append("action_name", "AllocateReservations");
  params.append("addPackingList", _addPackingList->isChecked());
  params.append("startDateOffset", QDate::currentDate().daysTo(_dates->startDate()));
  params.append("endDateOffset", QDate::currentDate().daysTo(_dates->endDate()));
  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  if (_selectedCustomerShipto->isChecked())
    params.append("shipto_id", _customerShipto->id());
  if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
    accept();
}

void allocateReservations::sCustomerSelected()
{
  XSqlQuery selectedRes;
  _customerShipto->clear();
  selectedRes.prepare("SELECT shipto_id, shipto_num"
            "  FROM shiptoinfo"
            " WHERE (shipto_cust_id=:cust_id); ");
  selectedRes.bindValue(":cust_id", _cust->id());
  selectedRes.exec();
  _customerShipto->populate(selectedRes);
}

