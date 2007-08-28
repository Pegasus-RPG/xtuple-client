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

#include "rptSalesHistoryByParameterList.h"

#include <qvariant.h>
#include <openreports.h>
#include <qmessagebox.h>

/*
 *  Constructs a rptSalesHistoryByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptSalesHistoryByParameterList::rptSalesHistoryByParameterList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptSalesHistoryByParameterList::~rptSalesHistoryByParameterList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptSalesHistoryByParameterList::languageChange()
{
    retranslateUi(this);
}


void rptSalesHistoryByParameterList::init()
{
  _parameter->setType(ProductCategory);

  _showCosts->setEnabled(_privleges->check("ViewCosts"));
  _showPrices->setEnabled(_privleges->check("ViewCustomerPrices"));
}

enum SetResponse rptSalesHistoryByParameterList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("prodcat", &valid);
  if (valid)
    _parameter->setType(ProductCategory);

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _parameter->setType(ProductCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("prodcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ProductCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custtype", &valid);
  if (valid)
    _parameter->setType(CustomerType);

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _parameter->setType(CustomerType);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custtype_pattern", &valid);
  if (valid)
  {
    _parameter->setType(CustomerType);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("custgrp", &valid);
  if (valid)
    _parameter->setType(CustomerGroup);

  param = pParams.value("custgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(CustomerGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("custgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(CustomerGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  _showPrices->setChecked(pParams.inList("showPrices"));
  _showCosts->setChecked(pParams.inList("showCosts"));

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (_parameter->type() == ProductCategory)
    setCaption(tr("Report Parameters - Sales History by Product Category"));
  if (_parameter->type() == CustomerType)
    setCaption(tr("Report Parameters - Sales History by Customer Type"));
  if (_parameter->type() == CustomerGroup)
    setCaption(tr("Report Parameters - Sales History by Customer Group"));

  return NoError;
}

void rptSalesHistoryByParameterList::sPrint()
{
    if (!_dates->allValid())
    {
        QMessageBox::warning( this, tr("Enter Valid Dates"),
                              tr("Please enter a valid Start and End Date.") );
        _dates->setFocus();
        return;
    }

    ParameterList params;

    _parameter->appendValue(params);
    _warehouse->appendValue(params);
    _dates->appendValue(params);

    if ( (_parameter->isAll()) && (_parameter->type() == CustomerGroup) )
      params.append("custgrp");

    if(_showCosts->isChecked())
      params.append("showCosts");
    if(_showPrices->isChecked())
      params.append("showPrices");

    orReport report("SalesHistoryByParameterList", params);
    if (report.isValid())
        report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    if(_captive)
        accept();
}

