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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include "dspCustomerInformationExport.h"

#include <QSqlError>
#include <QVariant>

dspCustomerInformationExport::dspCustomerInformationExport(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));

  _customerType->setType(ParameterGroup::CustomerType);
}

dspCustomerInformationExport::~dspCustomerInformationExport()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCustomerInformationExport::languageChange()
{
  retranslateUi(this);
}

void dspCustomerInformationExport::sQuery()
{
  _cust->setColumnCount(0);

  struct {
    QCheckBox *checkbox;
    QString title;
    QString dbcol;
  } list[] = {
    { _number,       tr("Customer Number"),   "cust_number" },
    { _name,         tr("Customer Name"),     "cust_name" },
    { _billAddress1, tr("Billing Address 1"), "cust_address1" },
    { _billAddress2, tr("Billing Address 2"), "cust_address2" },
    { _billAddress3, tr("Billing Address 3"), "cust_address3" },
    { _billCity,     tr("Billing City"),      "cust_city" },
    { _billState,    tr("Billing State"),     "cust_state" },
    { _billZipCode,  tr("Billing Zip Code"),  "cust_zipcode" },
    { _billCountry,  tr("Billing Country"),   "cust_country" },
    { _billContact,  tr("Billing Contact"),   "cust_contact" },
    { _billPhone,    tr("Billing Phone"),     "cust_phone" },
    { _billFAX,      tr("Billing FAX"),       "cust_fax" },
    { _billEmail,    tr("Billing Email"),     "cust_email" },
    { _corrAddress1, tr("Corr. Address 1"),   "cust_corraddress1" },
    { _corrAddress2, tr("Corr. Address 2"),   "cust_corraddress2" },
    { _corrAddress3, tr("Corr. Address 3"),   "cust_corraddress3" },
    { _corrCity,     tr("Corr. City"),        "cust_corrcity" },
    { _corrState,    tr("Corr. State"),       "cust_corrstate" },
    { _corrZipCode,  tr("Corr. Zip Code"),    "cust_corrzipcode" },
    { _corrCountry,  tr("Corr. Country"),     "cust_corrcountry" },
    { _corrContact,  tr("Corr. Contact"),     "cust_corrcontact" },
    { _corrPhone,    tr("Corr. Phone"),       "cust_corrphone" },
    { _corrFAX,      tr("Corr. FAX"),         "cust_corrfax" },
    { _corrEmail,    tr("Corr. Email"),       "cust_corremail" }
  };

  QString sql("SELECT cust_id ");

  for (unsigned int i = 0; i < sizeof(list) / sizeof(list[0]); i++)
  {
    if (list[i].checkbox->isChecked())
    {
      _cust->addColumn(list[i].title, 100, Qt::AlignLeft, true, list[i].dbcol);
      sql += ", " + list[i].dbcol;
    }
  }

  sql += " FROM cust, custtype "
         "WHERE ( (cust_custtype_id=custtype_id)";

  if (!_showInactive->isChecked())
    sql += " AND (cust_active)";

  if (_customerType->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";
  
  sql += " ) "
         "ORDER BY cust_number;";

  q.prepare(sql);
  _customerType->bindValue(q);
  q.exec();
  _cust->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
