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

#include "rptPoItemsByVendor.h"

#include <qvariant.h>
#include <openreports.h>
#include <qmessagebox.h>

/*
 *  Constructs a rptPoItemsByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptPoItemsByVendor::rptPoItemsByVendor(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_vendor, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));
    connect(_selectedPO, SIGNAL(toggled(bool)), _poNumber, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptPoItemsByVendor::~rptPoItemsByVendor()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptPoItemsByVendor::languageChange()
{
    retranslateUi(this);
}


void rptPoItemsByVendor::init()
{
  _agent->populate( "SELECT usr_id, usr_username "
                    "FROM usr "
                    "WHERE (usr_agent) "
                    "ORDER BY usr_username;" );
  _agent->setText(omfgThis->username());
}

enum SetResponse rptPoItemsByVendor::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendor->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
  else
    _warehouse->setAll();

  param = pParams.value("agentUsername", &valid);
  if (valid)
  {
    _selectedPurchasingAgent->setChecked(TRUE);
    _agent->setText(param.toString());
  }

  param = pParams.value("poNumber", &valid);
  if (valid)
  {
    _selectedPO->setChecked(TRUE);
    _poNumber->setText(param.toString());
  }

  _closedItems->setChecked(pParams.inList("closedItems"));
  _openItems->setChecked(pParams.inList("openItems"));

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void rptPoItemsByVendor::sPrint()
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Vendor Number"),
                          tr( "Please enter a valid Vendor Number." ) );
    _vendor->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  params.append("vend_id", _vendor->id());

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_selectedPO->isChecked())
    params.append("poNumber", _poNumber->currentText());

  if (_openItems->isChecked())
    params.append("openItems");
  else if (_closedItems->isChecked())
    params.append("closeItems");

  orReport report("POLineItemsByVendor", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
  }

  if (_captive)
    accept();
  else
  {
    _vendor->setId(-1);
    _vendor->setFocus();
  }
}

void rptPoItemsByVendor::sPopulatePo()
{
  _poNumber->clear();

  if ( _vendor->isValid()) 
  {
    q.prepare( "SELECT pohead_id, pohead_number "
               "FROM pohead "
               "WHERE ( (pohead_vend_id=:vend_id) ) "
               "ORDER BY pohead_number;" );
    q.bindValue(":vend_id", _vendor->id());
    q.exec();
    _poNumber->populate(q);
  }
}

