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
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
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

#include "rptInventoryAvailabilityBySourceVendor.h"

#include <qvariant.h>
#include <openreports.h>
#include <qmessagebox.h>

/*
 *  Constructs a rptInventoryAvailabilityBySourceVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptInventoryAvailabilityBySourceVendor::rptInventoryAvailabilityBySourceVendor(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    _vendorGroupInt = new QButtonGroup(this);
    _vendorGroupInt->addButton(_allVendors);
    _vendorGroupInt->addButton(_selectedVendor);
    _vendorGroupInt->addButton(_selectedVendorType);
    _vendorGroupInt->addButton(_vendorTypePattern);

    _showByGroupInt = new QButtonGroup(this);
    _showByGroupInt->addButton(_byLeadTime);
    _showByGroupInt->addButton(_byDays);
    _showByGroupInt->addButton(_byDate);
    _showByGroupInt->addButton(_byDates);

    // signals and slots connections
    connect(_byDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
    connect(_byDate, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_showReorder, SIGNAL(toggled(bool)), _ignoreReorderAtZero, SLOT(setEnabled(bool)));
    connect(_byDates, SIGNAL(toggled(bool)), _startDate, SLOT(setEnabled(bool)));
    connect(_byDates, SIGNAL(toggled(bool)), _endDate, SLOT(setEnabled(bool)));
    connect(_selectedVendor, SIGNAL(toggled(bool)), _vend, SLOT(setEnabled(bool)));
    connect(_selectedVendorType, SIGNAL(toggled(bool)), _vendorTypes, SLOT(setEnabled(bool)));
    connect(_vendorTypePattern, SIGNAL(toggled(bool)), _vendorType, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptInventoryAvailabilityBySourceVendor::~rptInventoryAvailabilityBySourceVendor()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptInventoryAvailabilityBySourceVendor::languageChange()
{
    retranslateUi(this);
}


void rptInventoryAvailabilityBySourceVendor::init()
{
  _captive = FALSE;

  _vendorTypes->setType(XComboBox::VendorTypes);
}

enum SetResponse rptInventoryAvailabilityBySourceVendor::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _selectedVendor->setChecked(TRUE);
    _vend->setId(param.toInt());
  }

  param = pParams.value("vendtype_id", &valid);
  if (valid)
  {
    _selectedVendorType->setChecked(TRUE);
    _vendorTypes->setId(param.toInt());
  }

  param = pParams.value("vendtype_pattern", &valid);
  if (valid)
  {
    _vendorTypePattern->setChecked(TRUE);
    _vendorType->setText(param.toString());
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  _byLeadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
    _byDays->setChecked(TRUE);
    _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
    _byDate->setChecked(TRUE);
    _date->setDate(param.toDate());
  }

  _byDates->setChecked(pParams.inList("byDates"));

  param = pParams.value("startDate", &valid);
  if (valid)
    _startDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endDate->setDate(param.toDate());

  _showReorder->setChecked(pParams.inList("showReorder"));
  _ignoreReorderAtZero->setChecked(pParams.inList("ignoreReorderAtZero"));
  _showShortages->setChecked(pParams.inList("showShortages"));

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void rptInventoryAvailabilityBySourceVendor::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);

  if (_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());
  else if (_selectedVendorType->isChecked())
    params.append("vendtype_id", _vendorTypes->id());
  else if (_vendorTypePattern->isChecked())
    params.append("vendtype_pattern", _vendorType->text());

  if (_byLeadTime->isChecked())
    params.append("byLeadTime");
  else if (_byDays->isChecked())
    params.append("byDays", _days->text().toInt());
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());
  else if (_byDates->isChecked())
  {
    params.append("byDates");
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }

  if(_showReorder->isChecked())
    params.append("showReorder");

  if(_ignoreReorderAtZero->isChecked())
    params.append("ignoreReorderAtZero");

  if(_showShortages->isChecked())
    params.append("showShortages");

  orReport report("InventoryAvailabilityBySourceVendor", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
  }

  if (_captive)
    accept();
}

