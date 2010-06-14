/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspExpiredInventoryByClassCode.h"

#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include <metasql.h>
#include "mqlutil.h"

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

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_expired, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);

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

  MetaSQLQuery mql = mqlLoad("expiredInventory", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _expired->populate(q);

}

bool dspExpiredInventoryByClassCode::setParams(ParameterList &params)
{
  if (_perishability->isChecked())
  {
    params.append("perishability");
    params.append("expiretype", tr("Perishability"));
  }
  else
    params.append("expiretype", tr("Warranty"));

  params.append("thresholdDays", _thresholdDays->value());

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  if (_classCode->isSelected())
    _classCode->appendValue(params);

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");
  else if (_useActualCosts->isChecked())
    params.append("useActualCosts"); 

  return TRUE;
}
