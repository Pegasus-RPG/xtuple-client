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

#include "dspPlannedRevenueExpensesByPlannerCode.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <parameter.h>
#include "rptPlannedRevenueExpensesByPlannerCode.h"

/*
 *  Constructs a dspPlannedRevenueExpensesByPlannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPlannedRevenueExpensesByPlannerCode::dspPlannedRevenueExpensesByPlannerCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _costGroupInt = new QButtonGroup(this);
    _costGroupInt->addButton(_useStandardCost);
    _costGroupInt->addButton(_useActualCost);

    QButtonGroup* _salesPriceGroupInt = new QButtonGroup(this);
    _salesPriceGroupInt->addButton(_useListPrice);
    _salesPriceGroupInt->addButton(_useAveragePrice);

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_planord, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _startEvalDateLit, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _startEvalDate, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _endEvalDateLit, SLOT(setEnabled(bool)));
    connect(_useAveragePrice, SIGNAL(toggled(bool)), _endEvalDate, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPlannedRevenueExpensesByPlannerCode::~dspPlannedRevenueExpensesByPlannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPlannedRevenueExpensesByPlannerCode::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspPlannedRevenueExpensesByPlannerCode::init()
{
  statusBar()->hide();

  _plannerCode->setType(PlannerCode);

  _planord->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft   );
  _planord->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter );
  _planord->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _planord->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _planord->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter );
  _planord->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight  );
  _planord->addColumn(tr("Firm"),        _ynColumn,    Qt::AlignCenter );
  _planord->addColumn(tr("Cost"),        _moneyColumn, Qt::AlignRight  );
  _planord->addColumn(tr("Revenue"),     _moneyColumn, Qt::AlignRight  );
  _planord->addColumn(tr("Gr. Profit"),  _moneyColumn, Qt::AlignRight  );
}

void dspPlannedRevenueExpensesByPlannerCode::sPrint()
{
  ParameterList params;
  params.append("startDate", _startDate->date());
  params.append("endDate", _endDate->date());
  params.append("print");
  _warehouse->appendValue(params);
  _plannerCode->appendValue(params);

  if (_useStandardCost->isChecked())
    params.append("useStandardCost");

  if (_useActualCost->isChecked())
    params.append("useActualCost");

  if (_useListPrice->isChecked())
    params.append("useListPrice");

  if (_useAveragePrice->isChecked())
  {
    params.append("useAveragePrice");
    params.append("startEvalDate", _startEvalDate->date());
    params.append("endEvalDate", _endEvalDate->date());
  }

  rptPlannedRevenueExpensesByPlannerCode newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspPlannedRevenueExpensesByPlannerCode::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}

void dspPlannedRevenueExpensesByPlannerCode::sFillList()
{
  _planord->clear();

  if (_useAveragePrice->isChecked())
  {
    if ( (!_startEvalDate->isValid()) || (!_endEvalDate->isValid()) )
    {
      QMessageBox::critical( this, tr("Enter Start and End Evaluation Dates"),
                             tr("You must enter both a start and end date for Average Sales Price calculation") );
      return;
    }
  }

  QString sql( "SELECT planord_id, planord_itemsite_id,"
               "       plonumber, plotype, item_number, itemdescrip,"
               "       formatDate(planord_duedate) AS duedate, formatQty(planord_qty) AS qty,"
               "       plofirm,"
               "       formatMoney(plocost) AS cost, formatMoney(plorevenue) AS revenue,"
               "       formatMoney(plorevenue - plocost) AS profit,"
               "       plocost, plorevenue "
               "FROM ( SELECT planord_id, planord_itemsite_id, planord_duedate,"
               "       formatPloNumber(planord_id) AS plonumber,"
               "       CASE WHEN (planord_type='P') THEN 'P/O'"
               "            WHEN (planord_type='W') THEN 'W/O'"
               "            ELSE '?'"
               "       END AS plotype,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
               "       formatDate(planord_duedate),"
               "       planord_qty, formatBoolYN(planord_firm) AS plofirm," );

  if (_useActualCost->isChecked())
    sql += " (actcost(item_id) * planord_qty) AS plocost,";
  else if (_useStandardCost->isChecked())
    sql += " (stdcost(item_id) * planord_qty) AS plocost,";

  if (_useListPrice->isChecked())
    sql += " (item_listprice * planord_qty) AS plorevenue ";
  else if (_useAveragePrice->isChecked())
    sql += " (CASE WHEN(averageSalesPrice(itemsite_id, :startEvalDate, :endEvalDate)=0) THEN item_listprice ELSE averageSalesPrice(itemsite_id, :startEvalDate, :endEvalDate) END * planord_qty) AS plorevenue ";

  sql += "FROM planord, itemsite, item "
         "WHERE ((planord_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (item_sold)"
         " AND (planord_duedate BETWEEN :startDate AND :endDate)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") ) AS data "
         "ORDER BY planord_duedate, item_number";

  q.prepare(sql);
  q.bindValue(":startEvalDate", _startEvalDate->date());
  q.bindValue(":endEvalDate", _endEvalDate->date());
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();

  XTreeWidgetItem *last = NULL;
  double        cost = 0;
  double        revenue = 0;
  
  while (q.next())
  {
    last = new XTreeWidgetItem( _planord, last, q.value("planord_id").toInt(), q.value("planord_itemsite_id").toInt(),
                              q.value("plonumber"), q.value("plotype"),
                              q.value("item_number"), q.value("itemdescrip"),
                              q.value("duedate"), q.value("qty"),
                              q.value("plofirm"), q.value("cost"),
                              q.value("revenue"), q.value("profit") );

    cost += q.value("plocost").toDouble();
    revenue += q.value("plorevenue").toDouble();

    if (q.value("plocost").toDouble() > q.value("plorevenue").toDouble())
      last->setTextColor(9, "red");
  }

  last = new XTreeWidgetItem(_planord, last, -1, -1, tr("Totals:"));
  last->setText(7, formatMoney(cost));
  last->setText(8, formatMoney(revenue));
  last->setText(9, formatMoney(revenue - cost));

  if (cost > revenue)
    last->setTextColor(9, "red");
}

