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

#include "dspPoItemReceivingsByItem.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspPoItemReceivingsByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoItemReceivingsByItem::dspPoItemReceivingsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));

  _item->setType(ItemLineEdit::cGeneralPurchased);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _showVariances->setEnabled(_privleges->check("ViewCosts"));

  _porecv->addColumn(tr("P/O #"),        _orderColumn, Qt::AlignRight  );
  _porecv->addColumn(tr("Vendor"),       120,          Qt::AlignLeft   );
  _porecv->addColumn(tr("Due Date"),     _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("Recv. Date"),   _dateColumn,  Qt::AlignCenter );
  _porecv->addColumn(tr("Vend. Item #"), _itemColumn,  Qt::AlignLeft   );
  _porecv->addColumn(tr("Description"),  -1,           Qt::AlignLeft   );
  _porecv->addColumn(tr("Rcvd/Rtnd"),    _qtyColumn,   Qt::AlignRight  );
  _porecv->addColumn(tr("Qty."),         _qtyColumn,   Qt::AlignRight  );
  if (_privleges->check("ViewCosts"))
  {
    _porecv->addColumn(tr("Purch. Cost"), _priceColumn,  Qt::AlignRight );
    _porecv->addColumn(tr("Recv. Cost"),  _priceColumn,  Qt::AlignRight );
  }

  sHandleVariance(_showVariances->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoItemReceivingsByItem::~dspPoItemReceivingsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoItemReceivingsByItem::languageChange()
{
  retranslateUi(this);
}

void dspPoItemReceivingsByItem::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                          tr( "Please enter a valid Item Number." ) );
    _item->setFocus();
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
  params.append("item_id", _item->id() );

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    params.append("showVariances");

  orReport report("ReceiptsReturnsByItem", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspPoItemReceivingsByItem::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    _porecv->showColumn(8);
    _porecv->showColumn(9);
  }
  else
  {
    _porecv->hideColumn(8);
    _porecv->hideColumn(9);
  }
}

void dspPoItemReceivingsByItem::sFillList()
{
  if (_item->isValid())
  {
    QString sql( "SELECT porecv_id, porecv_ponumber, vend_name,"
                 "       formatDate(porecv_duedate),"
                 "       formatDate(porecv_date),"
                 "       porecv_vend_item_number, porecv_vend_item_descrip, :received,"
                 "       formatQty(porecv_qty),"
                 "       formatCost(porecv_purchcost),"
                 "       CASE WHEN (porecv_recvcost IS NULL) THEN :na"
                 "            ELSE formatCost(porecv_recvcost)"
                 "       END,"
                 "       porecv_date AS sortdate "
                 "FROM porecv, vend, itemsite "
                 "WHERE ( (porecv_vend_id=vend_id)"
                 " AND (porecv_itemsite_id=itemsite_id)"
                 " AND (DATE(porecv_date) BETWEEN :startDate AND :endDate)"
                 " AND (itemsite_item_id=:item_id)" );

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_selectedPurchasingAgent->isChecked())
      sql += " AND (porecv_agent_username=:username)";

    sql += ") "
           "UNION SELECT poreject_id, poreject_ponumber, vend_name,"
           "             '',"
           "             formatDate(poreject_date),"
           "             poreject_vend_item_number, poreject_vend_item_descrip, :returned,"
           "             formatQty(poreject_qty),"
           "             '',"
           "             '',"
           "             poreject_date AS sortdate "
           "FROM poreject, vend, itemsite "
           "WHERE ( (poreject_vend_id=vend_id)"
           " AND (poreject_itemsite_id=itemsite_id)"
           " AND (DATE(poreject_date) BETWEEN :startDate AND :endDate)"
           " AND (itemsite_item_id=:item_id)";

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
    q.bindValue(":item_id", _item->id());
    q.bindValue(":username", _agent->currentText());
    q.exec();
    _porecv->populate(q);
  }
  else
    _porecv->clear();
}
