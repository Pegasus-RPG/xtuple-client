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

#include "dspSummarizedTaxableSales.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspSummarizedTaxableSales as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedTaxableSales::dspSummarizedTaxableSales(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_selectedTaxCode, SIGNAL(toggled(bool)), _taxCode, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _dates->setStartCaption(tr("Start Ship Date:"));
  _dates->setEndCaption(tr("End Ship Date:"));

  _taxCode->populate( "SELECT tax_id, (tax_code || '-' || tax_descrip) "
                      "FROM tax "
                      "ORDER by tax_code;" );

  _invchead->addColumn(tr("Tax Code"),      _itemColumn,  Qt::AlignLeft   );
  _invchead->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _invchead->addColumn(tr("Sales $"),       _itemColumn,  Qt::AlignRight  );
  _invchead->addColumn(tr("Freight $"),     _moneyColumn, Qt::AlignRight  );
  _invchead->addColumn(tr("Freight Taxed"), _itemColumn,  Qt::AlignCenter ); 
  _invchead->addColumn(tr("Tax $"),         _moneyColumn, Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedTaxableSales::~dspSummarizedTaxableSales()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedTaxableSales::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedTaxableSales::sPrint()
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Start Date"),
                          tr("You must enter a valid Start Date to print this report.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Valid End Date"),
                          tr("You must enter a valid End Date to print this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedTaxCode->isChecked())
    params.append("tax_id", _taxCode->id());

  orReport report("SummarizedTaxableSales", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedTaxableSales::sFillList()
{
  if (_dates->allValid())
  {
    QString sql( "SELECT tax_id, tax_code, tax_descrip,"
                 "       formatMoney( ( SELECT COALESCE(SUM(round(cohist_qtyshipped * cohist_unitprice,2)), 0)"
                 "                      FROM cohist"
                 "                      WHERE ( (cohist_tax_id=tax_id)"
                 "                       AND (cohist_itemsite_id<>-1)"
                 "                       AND (cohist_invcdate BETWEEN :startDate AND :endDate) ) ) ),"
                 "       formatMoney( ( SELECT COALESCE(SUM(cohist_unitprice), 0)"
                 "                         FROM cohist"
                 "                         WHERE ( (cohist_tax_id=tax_id)"
                 "                          AND (cohist_misc_type='F')"
                 "                          AND (cohist_invcdate BETWEEN :startDate AND :endDate) ) ) ),"
                 "       formatBoolYN(tax_freight),"
                 "       formatMoney( ( SELECT COALESCE(SUM(COALESCE(cohist_tax_ratea,0) + COALESCE(cohist_tax_rateb,0) + COALESCE(cohist_tax_ratec, 0)), 0)"
                 "                         FROM cohist"
                 "                         WHERE ( (cohist_tax_id=tax_id)"
                // "                          AND (cohist_misc_type='T')"
                 "                          AND (cohist_invcdate BETWEEN :startDate AND :endDate) ) ) ) "
                 "FROM tax " );

    if (_selectedTaxCode->isChecked())
      sql += "WHERE (tax_id=:tax_id) ";

    sql += "GROUP BY tax_id, tax_code, tax_descrip, tax_freight;";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":tax_id", _taxCode->id());
    q.exec();
    _invchead->populate(q);
  }
  else
    _invchead->clear();
}
