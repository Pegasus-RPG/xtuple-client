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

#include "dspExpiredInventoryByClassCode.h"

#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"
#include "guiclient.h"

#define COST_COL	6
#define VALUE_COL	7
#define METHOD_COL 8

dspExpiredInventoryByClassCode::dspExpiredInventoryByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  _orderByGroupInt = new QButtonGroup(this);
  _orderByGroupInt->addButton(_itemNumber);
  _orderByGroupInt->addButton(_expirationDate);
  _orderByGroupInt->addButton(_inventoryValue);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_expired, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);
  _inventoryValue->setEnabled(_showValue->isChecked());

  _expired->addColumn(tr("Site"),         _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _expired->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _expired->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _expired->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft,   true,  "ls_number"   );
  _expired->addColumn(tr("Expiration"),   _dateColumn, Qt::AlignCenter, true,  "itemloc_expiration" );
  _expired->addColumn(tr("Qty."),         _qtyColumn,  Qt::AlignRight,  true,  "itemloc_qty"  );
  _expired->addColumn(tr("Unit Cost"),    _costColumn, Qt::AlignRight,  true,  "cost" );
  _expired->addColumn(tr("Value"),        _costColumn, Qt::AlignRight,  true,  "value" );
  _expired->addColumn(tr("Cost Method"),  _costColumn, Qt::AlignCenter, true,  "costmethod" );

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));
  if (! _privileges->check("ViewInventoryValue") || ! _showValue->isChecked())
  {
    _expired->hideColumn(COST_COL);
    _expired->hideColumn(VALUE_COL);
    _expired->hideColumn(METHOD_COL);
  }
}

dspExpiredInventoryByClassCode::~dspExpiredInventoryByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspExpiredInventoryByClassCode::languageChange()
{
  retranslateUi(this);
}

void dspExpiredInventoryByClassCode::sPrint()
{
  ParameterList params;
  params.append("thresholdDays", _thresholdDays->value());

  _warehouse->appendValue(params);
  _classCode->appendValue(params);
  
  if(_perishability->isChecked())
    params.append("perishability");
  else
    params.append("warranty");

  if(_showValue->isChecked())
    params.append("showValue");

  if (_useActualCosts->isChecked())
    params.append("useActualCost");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCost");

  if (_usePostedCosts->isChecked())
    params.append("usePostedCosts");

  if (_inventoryValue->isChecked())
    params.append("orderByInventoryValue");
  else if(_expirationDate->isChecked())
    params.append("orderByExpirationDate");
  else
    params.append("orderByItemNumber");

  orReport report("ExpiredInventoryByClassCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspExpiredInventoryByClassCode::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
#if 0
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Transfer to another Site..."), this, SLOT(sTransfer()), 0);
    if (!_privileges->check("CreateInterWarehouseTrans"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Adjust this QOH..."), this, SLOT(sAdjust()), 0);
    if (!_privileges->check("CreateAdjustmentTrans"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reset this QOH to 0..."), this, SLOT(sReset()), 0);
    if (!_privileges->check("CreateAdjustmentTrans"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Enter Misc. Count..."), this, SLOT(sMiscCount()), 0);
    if (!_privileges->check("EnterMiscCounts"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
    if (!_privileges->check("IssueCountTags"))
      pMenu->setItemEnabled(menuItem, FALSE);
  } 
#endif
}

void dspExpiredInventoryByClassCode::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _expired->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspExpiredInventoryByClassCode::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _expired->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspExpiredInventoryByClassCode::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _expired->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspExpiredInventoryByClassCode::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _expired->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspExpiredInventoryByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _expired->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspExpiredInventoryByClassCode::sHandleValue(bool pShowValue)
{
  if (pShowValue)
  {
    _expired->showColumn(COST_COL);
    _expired->showColumn(VALUE_COL);
    _expired->showColumn(METHOD_COL);
  }
  else
  {
    _expired->hideColumn(COST_COL);
    _expired->hideColumn(VALUE_COL);
    _expired->hideColumn(METHOD_COL);
  }
}

void dspExpiredInventoryByClassCode::sFillList()
{
  _expired->clear();

  _expired->setColumnVisible(METHOD_COL, _showValue->isChecked() && _usePostedCosts->isChecked());

  QString sql( "SELECT itemsite_id, itemloc_id, warehous_code, item_number, uom_name,"
               "       ls_number, itemloc_expiration, "
               "       itemloc_qty, costmethod, cost,"
               "       noNeg(cost * itemloc_qty) AS value,"
               "       'qty' AS itemloc_qty_xtnumericrole,"
               "       'cost' AS cost_xtnumericrole,"
               "       'curr' AS value_xtnumericrole,"
               "       'red' AS qtforegroundrole "
               "FROM ( SELECT itemsite_id, itemloc_id, warehous_code, item_number,"
               "              uom_name, ls_number, "
               "       CASE WHEN :expiretype='Perishability' THEN "
               "         itemloc_expiration "
               "       ELSE itemloc_warrpurc "
               "       END AS itemloc_expiration, "
               "              itemloc_qty,"
               "       CASE WHEN(itemsite_costmethod='A') THEN 'Average'"
               "            WHEN(itemsite_costmethod='S') THEN 'Standard'"
               "            WHEN(itemsite_costmethod='J') THEN 'Job'"
               "            WHEN(itemsite_costmethod='N') THEN 'None'"
               "            ELSE 'UNKNOWN'"
               "       END AS costmethod," );

  if (_useStandardCosts->isChecked())
    sql += " stdcost(itemsite_item_id) AS cost ";
  else if (_useActualCosts->isChecked())
    sql += " actcost(itemsite_item_id) AS cost ";
  else
    sql += " (itemsite_value / CASE WHEN(itemsite_qtyonhand=0) THEN 1 ELSE itemsite_qtyonhand END) AS cost ";

  sql += "FROM itemloc, itemsite, item, warehous, uom, ls "
         "WHERE ( (itemloc_itemsite_id=itemsite_id)"
         " AND (itemsite_item_id=item_id)"
         " AND (item_inv_uom_id=uom_id)"
         " AND (itemsite_warehous_id=warehous_id)"
         " AND (itemloc_ls_id=ls_id)";
         
  if (_perishability->isChecked())
    sql += " AND (itemsite_perishable)"
           " AND (itemloc_expiration < (CURRENT_DATE + :thresholdDays))";
  else
    sql += " AND (itemsite_warrpurc)"
           " AND (itemloc_warrpurc < (CURRENT_DATE + :thresholdDays))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE classcode_code ~ :classcode_pattern))";

  sql += ") ) AS data ";

  if (_itemNumber->isChecked())
    sql += "ORDER BY warehous_code, item_number;";
  else if (_expirationDate->isChecked())
    sql += "ORDER BY warehous_code, itemloc_expiration;";
  else
    sql += "ORDER BY warehous_code, noNeg(cost * itemloc_qty) DESC;";

  q.prepare(sql);
  if (_perishability->isChecked())
    q.bindValue(":expiretype", QString("Perishability"));
  else
    q.bindValue(":expiretype", QString("Warranty"));
    
  q.bindValue(":thresholdDays", _thresholdDays->value());
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  q.exec();
  _expired->populate(q, true);
}
