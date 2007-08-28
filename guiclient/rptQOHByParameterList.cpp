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

#include "rptQOHByParameterList.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>

/*
 *  Constructs a rptQOHByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptQOHByParameterList::rptQOHByParameterList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    _showGroupInt = new QButtonGroup(this);
    _showGroupInt->addButton(_showAll);
    _showGroupInt->addButton(_showPositive);
    _showGroupInt->addButton(_showNegative);

    _costsGroupInt = new QButtonGroup(this);
    _costsGroupInt->addButton(_useStandardCosts);
    _costsGroupInt->addButton(_useActualCosts);

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_showValue, SIGNAL(toggled(bool)), _costsGroup, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptQOHByParameterList::~rptQOHByParameterList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptQOHByParameterList::languageChange()
{
    retranslateUi(this);
}


void rptQOHByParameterList::init()
{
  _captive = FALSE;

  _showValue->setEnabled(_privleges->check("ViewInventoryValue"));
}

enum SetResponse rptQOHByParameterList::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if(valid)
    _parameter->setType(ClassCode);

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if(valid)
    _parameter->setType(ItemGroup);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  _showPositive->setChecked(pParams.inList("onlyShowPositive"));
  _showNegative->setChecked(pParams.inList("onlyShowNegative"));
  _showValue->setChecked(pParams.inList("showValue"));
  if(pParams.inList("useStandardCosts"))
    _useStandardCosts->setChecked(TRUE);
  if(pParams.inList("useActualCosts"))
    _useActualCosts->setChecked(TRUE);

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  switch (_parameter->type())
  {
    case ClassCode:
      setCaption(tr("Report Parameters - Quantities on Hand by Class Code"));
      break;

    case ItemGroup:
      setCaption(tr("Report Parameters - Quantities on Hand by Item Group"));
      break;

    default:
      break;
  }

  return NoError;
}

void rptQOHByParameterList::sPrint()
{
  ParameterList params;

  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if(_parameter->type() == ItemGroup)
    params.append("itemgrp");

  if(_showValue->isChecked())
    params.append("showValue");

  if(_useStandardCosts->isChecked())
    params.append("useStandardCosts");
  
  if(_useActualCosts->isChecked())
    params.append("useActualCosts");
  
  if(_showPositive->isChecked())
    params.append("onlyShowPositive");
  
  if(_showNegative->isChecked())
    params.append("onlyShowNegative");

  orReport report("QOHByParameterList", params);

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

