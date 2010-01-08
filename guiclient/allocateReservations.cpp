/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "allocateReservations.h"

#include <QVariant>
#include <QMessageBox>

#include "submitAction.h"

/*
 *  Constructs a allocateReservations as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
allocateReservations::allocateReservations(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_allocate, SIGNAL(clicked()), this, SLOT(sAllocate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sCustomerSelected()));

  _customerTypes->setType(XComboBox::CustomerTypes);
}

/*
 *  Destroys the object and frees any allocated resources
 */
allocateReservations::~allocateReservations()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
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

  q.prepare("SELECT reserveAllSo(:addpackinglist, :startDate, :endDate, :cust_id, :shipto_id, :custtype_id, :custtype_pattern) AS result;");
  q.bindValue(":addpackinglist", _addPackingList->isChecked());
  _dates->bindValue(q);
  if (_selectedCustomer->isChecked())
    q.bindValue(":cust_id", _cust->id());
  if (_selectedCustomerShipto->isChecked())
    q.bindValue(":shipto_id", _customerShipto->id());
  if (_selectedCustomerType->isChecked())
    q.bindValue(":custtype_id", _customerTypes->id());
  if (_customerTypePattern->isChecked())
    q.bindValue(":custtype_pattern", _customerType->text());

  q.exec();

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
  _customerShipto->clear();
  q.prepare("SELECT shipto_id, shipto_num"
            "  FROM shipto"
            " WHERE (shipto_cust_id=:cust_id); ");
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  _customerShipto->populate(q);
}

