/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQOHByItem.h"
#include "guiclient.h"

#include <QMenu>

#include <metasql.h>

#include "adjustmentTrans.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "enterMiscCount.h"
#include "inputManager.h"
#include "transferTrans.h"
#include "mqlutil.h"

dspQOHByItem::dspQOHByItem(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspQOHByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quantities on Hand by Item"));
  setListLabel(tr("Quantities on Hand"));
  setReportName("QOHByItem");
  setMetaSQLOptions("qoh", "detail");

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  list()->addColumn(tr("Site"),             -1,           Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Default Location"), _itemColumn,  Qt::AlignLeft,   true,  "defaultlocation"   );
  list()->addColumn(tr("Reorder Lvl."),     _qtyColumn,   Qt::AlignRight,  true,  "reorderlevel"  );
  list()->addColumn(tr("QOH"),              _qtyColumn,   Qt::AlignRight,  true,  "qoh"  );
  list()->addColumn(tr("Non-Netable"),      _qtyColumn,   Qt::AlignRight,  true,  "f_nnqoh"  );
  list()->addColumn(tr("Unit Cost"),        _costColumn,  Qt::AlignRight,  true,  "cost"  );
  list()->addColumn(tr("Value"),            _costColumn,  Qt::AlignRight,  true,  "value"  );
  list()->addColumn(tr("NN Value"),         _costColumn,  Qt::AlignRight,  true,  "f_nnvalue"  );
  list()->addColumn(tr("Cost Method"),      _costColumn,  Qt::AlignCenter, true,  "f_costmethod" );
  sHandleValue(_showValue->isChecked());

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));

  sHandleValue(_showValue->isChecked());

  _item->setFocus();
}

void dspQOHByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspQOHByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    int menuItem;

    if (((XTreeWidgetItem *)pSelected)->altId())
    {
      pMenu->insertItem(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewDetail()), 0);
      pMenu->insertSeparator();
    }
  
    if (_metrics->boolean("MultiWhs"))
    {
      menuItem = pMenu->insertItem(tr("Transfer to another Site..."), this, SLOT(sTransfer()), 0);
      if (!_privileges->check("CreateInterWarehouseTrans"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

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
}

void dspQOHByItem::sViewDetail()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspQOHByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspQOHByItem::sHandleValue(bool pShowValue)
{
  list()->setColumnHidden(5, !pShowValue);
  list()->setColumnHidden(6, !pShowValue);
  list()->setColumnHidden(7, !pShowValue);
  list()->setColumnHidden(8, !pShowValue);
}

void dspQOHByItem::sFillList()
{
  list()->clear();
  list()->setColumnVisible(8, _showValue->isChecked() && _usePostedCosts->isChecked());

  display::sFillList();
}

bool dspQOHByItem::setParams(ParameterList &params)
{
  params.append("byItem");

  params.append("none", tr("None"));
  params.append("na", tr("N/A"));

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");
  else if (_useActualCosts->isChecked())
    params.append("useActualCosts");
  else if (_usePostedCosts->isChecked())
    params.append("usePostedCosts");

  if (_showValue->isChecked())
    params.append("showValue");

  _warehouse->appendValue(params);
  params.append("item_id", _item->id());

  return true;
}

