/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostsByClassCode.h"

#include <QMenu>
#include <QVariant>

#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "updateActualCostsByItem.h"
#include "postCostsByItem.h"

dspItemCostsByClassCode::dspItemCostsByClassCode(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemCostsByClassCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Costs by Class Code"));
  setListLabel(tr("Items"));
  setReportName("ItemCostsByClassCode");
  setMetaSQLOptions("itemCost", "detail");

  _classCode->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "description"   );
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true,  "scost"  );
  list()->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true,  "acost"  );
  list()->addColumn(tr("% Var."),      _costColumn,  Qt::AlignRight,  false,  "percent_variance" );
}

void dspItemCostsByClassCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspItemCostsByClassCode::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("classcode_id", &valid);
  if (valid)
    _classCode->setId(param.toInt());
  
  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    _classCode->setPattern(param.toString());
  
  if (pParams.inList("run"))
    sFillList();

  return NoError;
}

bool dspItemCostsByClassCode::setParams(ParameterList &params)
{
  params.append("byClassCode");
  _classCode->appendValue(params);

  if(_onlyShowZero->isChecked())
    params.append("onlyShowZeroCosts");

  if(_onlyShowDiff->isChecked())
    params.append("onlyShowDiffCosts");

  XSqlQuery qq;
  qq.exec("SELECT locale_cost_scale "
         "FROM locale, usr "
         "WHERE ((usr_locale_id=locale_id) AND (usr_username=CURRENT_USER));");
  if (qq.first())
    params.append("costscale", qq.value("locale_cost_scale").toInt());
  else
    params.append("costscale", decimalPlaces("cost"));

  return true;
}

void dspItemCostsByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);
  pMenu->insertItem(tr("View Item Costing Summary..."), this, SLOT(sViewItemCostingSummary()), 0);
  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Update Actual Costs..."), this, SLOT(sUpdateCosts()), 0);
  if (!_privileges->check("UpdateActualCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Post Actual Costs..."), this, SLOT(sPostCosts()), 0);
  if (!_privileges->check("PostActualCosts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspItemCostsByClassCode::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", list()->id());
  params.append("run");

  maintainItemCosts *newdlg = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostsByClassCode::sViewItemCostingSummary()
{
  ParameterList params;
  params.append("item_id", list()->id());
  params.append("run");

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostsByClassCode::sUpdateCosts()
{
  ParameterList params;
  params.append("item_id", list()->id());

  updateActualCostsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemCostsByClassCode::sPostCosts()
{
  ParameterList params;
  params.append("item_id", list()->id());

  postCostsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

