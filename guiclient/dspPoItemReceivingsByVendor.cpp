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

#include "dspPoItemReceivingsByVendor.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspPoItemReceivingsByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoItemReceivingsByVendor::dspPoItemReceivingsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_vendor, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _porecv->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Sched. Date"), _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("Recv. Date"),  _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _porecv->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _porecv->addColumn(tr("Rcvd/Rtnd"),   _qtyColumn,   Qt::AlignRight  );
  _porecv->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight  );
  if (_privleges->check("ViewCosts"))
  {
    _porecv->addColumn(tr("Purch. Cost"), _priceColumn,  Qt::AlignRight );
    _porecv->addColumn(tr("Recv. Cost"),  _priceColumn,  Qt::AlignRight );
  }

  _showVariances->setEnabled(_privleges->check("ViewCosts"));
  sHandleVariance(_showVariances->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoItemReceivingsByVendor::~dspPoItemReceivingsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoItemReceivingsByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoItemReceivingsByVendor::sPrint()
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                          tr( "Please enter a valid Item Number." ) );
    _vendor->setFocus();
    return;
  }

  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("vend_id", _vendor->id());

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    params.append("showVariances");

  orReport report("ReceiptsReturnsByVendor", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoItemReceivingsByVendor::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    _porecv->showColumn(7);
    _porecv->showColumn(8);
  }
  else
  {
    _porecv->hideColumn(7);
    _porecv->hideColumn(8);
  }
}

void dspPoItemReceivingsByVendor::sFillList()
{
  if (_dates->allValid())
  {
    QString sql( "SELECT porecv_id, porecv_ponumber,"
                 "       formatDate(porecv_duedate),"
                 "       formatDate(porecv_date),"
                 "       COALESCE(item_number, (:nonInv || porecv_vend_item_number)) AS itemnumber,"
                 "       COALESCE(item_descrip1, porecv_vend_item_descrip) AS itemdescrip,"
                 "       :received,"
                 "       formatQty(porecv_qty),"
                 "       formatCost(porecv_purchcost),"
                 "       CASE WHEN (porecv_recvcost IS NULL) THEN :na"
                 "            ELSE formatCost(porecv_recvcost)"
                 "       END,"
                 "       porecv_date AS sortdate "
                 "FROM vend, porecv LEFT OUTER JOIN"
                 "           ( itemsite JOIN item"
                 "             ON (itemsite_item_id=item_id)"
                 "           ) ON (porecv_itemsite_id=itemsite_id) "
                 "WHERE ( (porecv_vend_id=:vend_id)"
                 " AND (DATE(porecv_date) BETWEEN :startDate AND :endDate)" );

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_selectedPurchasingAgent->isChecked())
      sql += " AND (porecv_agent_username=:username)";

    sql += ") "
           "UNION "
           "SELECT poreject_id, poreject_ponumber,"
           "       '',"
           "       formatDate(poreject_date),"
           "       COALESCE(item_number, (:nonInv || poreject_vend_item_number)) AS itemnumber,"
           "       COALESCE(item_descrip1, poreject_vend_item_descrip) AS itemdescrip,"
           "       :returned,"
           "       formatQty(poreject_qty),"
           "       '',"
           "       '',"
           "       poreject_date AS sortdate "
           "FROM vend, poreject LEFT OUTER JOIN"
           "           ( itemsite JOIN item"
           "             ON (itemsite_item_id=item_id)"
           "           ) ON (poreject_itemsite_id=itemsite_id) "
           "WHERE ( (poreject_vend_id=vend_id)"
           " AND (vend_id=:vend_id)"
           " AND (DATE(poreject_date) BETWEEN :startDate AND :endDate)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_selectedPurchasingAgent->isChecked())
      sql += " AND (poreject_agent_username=:username)";

    sql += ") "
           "ORDER BY sortdate DESC";

    q.prepare(sql);
    _warehouse->bindValue(q);
    _dates->bindValue(q);
    q.bindValue(":received", tr("Received"));
    q.bindValue(":returned", tr("Returned"));
    q.bindValue(":na", tr("N/A"));
    q.bindValue(":nonInv", tr("NonInv - "));
    q.bindValue(":vend_id", _vendor->id());
    q.bindValue(":username", _agent->currentText());
    q.exec();
    _porecv->populate(q);
  }
  else
    _porecv->clear();
}
