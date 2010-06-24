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

#include <QMenu>

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "adjustmentTrans.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "enterMiscCount.h"
#include "inputManager.h"
#include "transferTrans.h"
#include "mqlutil.h"

dspQOHByItem::dspQOHByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);
  _costsGroupInt->addButton(_usePostedCosts);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_qoh, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showValue, SIGNAL(toggled(bool)), this, SLOT(sHandleValue(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _qoh->addColumn(tr("Site"),             -1,           Qt::AlignCenter, true,  "warehous_code" );
  _qoh->addColumn(tr("Default Location"), _itemColumn,  Qt::AlignLeft,   true,  "defaultlocation"   );
  _qoh->addColumn(tr("Reorder Lvl."),     _qtyColumn,   Qt::AlignRight,  true,  "reorderlevel"  );
  _qoh->addColumn(tr("QOH"),              _qtyColumn,   Qt::AlignRight,  true,  "qoh"  );
  _qoh->addColumn(tr("Non-Netable"),      _qtyColumn,   Qt::AlignRight,  true,  "f_nnqoh"  );
  _qoh->addColumn(tr("Unit Cost"),        _costColumn,  Qt::AlignRight,  true,  "cost"  );
  _qoh->addColumn(tr("Value"),            _costColumn,  Qt::AlignRight,  true,  "value"  );
  _qoh->addColumn(tr("NN Value"),         _costColumn,  Qt::AlignRight,  true,  "f_nnvalue"  );
  _qoh->addColumn(tr("Cost Method"),      _costColumn,  Qt::AlignCenter, true,  "f_costmethod" );
  sHandleValue(_showValue->isChecked());

  _showValue->setEnabled(_privileges->check("ViewInventoryValue"));

  sHandleValue(_showValue->isChecked());

  _item->setFocus();
}

dspQOHByItem::~dspQOHByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspQOHByItem::languageChange()
{
  retranslateUi(this);
}

void dspQOHByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);

  if (_showValue->isChecked())
    params.append("showValue");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  if (_usePostedCosts->isChecked())
    params.append("usePostedCosts");

  orReport report("QOHByItem", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
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
  params.append("itemsite_id", _qoh->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _qoh->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _qoh->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _qoh->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByItem::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _qoh->id());

  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspQOHByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _qoh->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspQOHByItem::sHandleValue(bool pShowValue)
{
  _qoh->setColumnHidden(5, !pShowValue);
  _qoh->setColumnHidden(6, !pShowValue);
  _qoh->setColumnHidden(7, !pShowValue);
  _qoh->setColumnHidden(8, !pShowValue);
}

void dspQOHByItem::sFillList()
{
  _qoh->clear();
  _qoh->setColumnVisible(8, _showValue->isChecked() && _usePostedCosts->isChecked());

  ParameterList params;

  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("qoh", "detail");
  q = mql.toQuery(params);
  _qoh->populate(q, true);
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

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  params.append("item_id", _item->id());

  return true;
}
