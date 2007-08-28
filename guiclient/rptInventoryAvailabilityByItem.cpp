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

#include "rptInventoryAvailabilityByItem.h"

#include <qvariant.h>
#include <openreports.h>
#include <qmessagebox.h>
#include "inputManager.h"

/*
 *  Constructs a rptInventoryAvailabilityByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptInventoryAvailabilityByItem::rptInventoryAvailabilityByItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    _showByGroupInt = new QButtonGroup(this);
    _showByGroupInt->addButton(_byLeadTime);
    _showByGroupInt->addButton(_byDays);
    _showByGroupInt->addButton(_byDate);
    _showByGroupInt->addButton(_byDates);

    // signals and slots connections
    connect(_byDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
    connect(_byDate, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_showReorder, SIGNAL(toggled(bool)), _ignoreReorderAtZero, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemSites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_byDates, SIGNAL(toggled(bool)), _startDate, SLOT(setEnabled(bool)));
    connect(_byDates, SIGNAL(toggled(bool)), _endDate, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptInventoryAvailabilityByItem::~rptInventoryAvailabilityByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptInventoryAvailabilityByItem::languageChange()
{
    retranslateUi(this);
}


void rptInventoryAvailabilityByItem::init()
{
  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _captive = FALSE;
}

enum SetResponse rptInventoryAvailabilityByItem::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

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

  param = pParams.value("byDates", &valid);
  if (valid)
    _byDates->setChecked(TRUE);

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

void rptInventoryAvailabilityByItem::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Invalid Data"),
                      tr("You must enter a valid Item Number for this report.") );
    _item->setFocus();
    return;
  }

  ParameterList params;
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

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

  orReport report("InventoryAvailabilityByItem", params);
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

