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

#include "dspBriefEarnedCommissions.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspBriefEarnedCommissions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBriefEarnedCommissions::dspBriefEarnedCommissions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_selectedSalesrep, SIGNAL(toggled(bool)), _salesrep, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _salesrep->setType(XComboBox::SalesReps);

  _commission->addColumn(tr("#"),               _seqColumn,      Qt::AlignCenter, true,  "salesrep_number" );
  _commission->addColumn(tr("Sales Rep."),      _itemColumn,     Qt::AlignLeft,   true,  "salesrep_name"   );
  _commission->addColumn(tr("Cust. #"),         _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  _commission->addColumn(tr("Customer"),        -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _commission->addColumn(tr("S/O #"),           _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _commission->addColumn(tr("Invoice #"),       _orderColumn,    Qt::AlignLeft,   true,  "cohist_invcnumber"   );
  _commission->addColumn(tr("Invc. Date"),      _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _commission->addColumn(tr("Ext. Price"),      _moneyColumn,    Qt::AlignRight,  true,  "sumextprice"  );
  _commission->addColumn(tr("Commission"),      _moneyColumn,    Qt::AlignRight,  true,  "sumcommission"  );
  _commission->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _commission->addColumn(tr("Base Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "sumbaseextprice"  );
  _commission->addColumn(tr("Base Commission"), _bigMoneyColumn, Qt::AlignRight,  true,  "sumbasecommission"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBriefEarnedCommissions::~dspBriefEarnedCommissions()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBriefEarnedCommissions::languageChange()
{
  retranslateUi(this);
}

void dspBriefEarnedCommissions::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Start and End Dates"),
                          tr("You must enter a valid Start and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());

  orReport report("BriefEarnedCommissions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefEarnedCommissions::sFillList()
{
  if (_dates->allValid())
  {
    QString sql( "SELECT cohist_salesrep_id, salesrep_number, salesrep_name, cust_number, cust_name,"
                 "       cohist_ordernumber, cohist_invcnumber, cohist_invcdate, currAbbr,"
                 "       SUM(extprice) AS sumextprice,"
                 "       SUM(cohist_commission) AS sumcommission,"
                 "       SUM(baseextprice) AS sumbaseextprice,"
                 "       SUM(basecommission) AS sumbasecommission,"
                 "       'curr' AS sumextprice_xtnumericrole,"
                 "       'curr' AS sumcommission_xtnumericrole,"
                 "       'curr' AS sumbaseextprice_xtnumericrole,"
                 "       'curr' AS sumbasecommission_xtnumericrole,"
                 "       0 AS sumbaseextprice_xttotalrole,"
                 "       0 AS sumbasecommission_xttotalrole "
                 "FROM saleshistory "
                 "WHERE ( (cohist_commission <> 0) "
                 "  AND   (cohist_invcdate BETWEEN :startDate AND :endDate)" );

    if (_selectedSalesrep->isChecked())
      sql += " AND (cohist_salesrep_id=:salesrep_id)";

    sql += ") "
           "GROUP BY cohist_salesrep_id, salesrep_number, salesrep_name, cust_number, cust_name,"
           "         cohist_ordernumber, cohist_invcnumber, cohist_invcdate, currAbbr "
           "ORDER BY salesrep_number, cust_number, cohist_invcdate";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":salesrep_id", _salesrep->id());
    q.exec();
    _commission->populate(q);
  }
}
