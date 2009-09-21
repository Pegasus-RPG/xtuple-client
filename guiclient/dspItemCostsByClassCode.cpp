/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostsByClassCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "updateActualCostsByItem.h"
#include "postCostsByItem.h"

dspItemCostsByClassCode::dspItemCostsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);

  _itemcost->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _itemcost->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "description"   );
  _itemcost->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true,  "scost"  );
  _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true,  "acost"  );
  _itemcost->addColumn(tr("% Var."),      _costColumn,  Qt::AlignRight,  false,  "percent_variance" );
}

dspItemCostsByClassCode::~dspItemCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemCostsByClassCode::languageChange()
{
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
  _classCode->appendValue(params);

  if(_onlyShowZero->isChecked())
    params.append("onlyShowZeroCosts");

  if(_onlyShowDiff->isChecked())
    params.append("onlyShowDiffCosts");

  return true;
}

void dspItemCostsByClassCode::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ItemCostsByClassCode", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostsByClassCode::sPopulateMenu(QMenu *pMenu)
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
  params.append("item_id", _itemcost->id());
  params.append("run");

  maintainItemCosts *newdlg = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostsByClassCode::sViewItemCostingSummary()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());
  params.append("run");

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostsByClassCode::sUpdateCosts()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());

  updateActualCostsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemCostsByClassCode::sPostCosts()
{
  ParameterList params;
  params.append("item_id", _itemcost->id());

  postCostsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemCostsByClassCode::sFillList()
{

  ParameterList params;
  if (! setParams(params))
    return;
  q.exec("SELECT locale_cost_scale "
         "FROM locale, usr "
         "WHERE ((usr_locale_id=locale_id) AND (usr_username=CURRENT_USER));");
  if (q.first())
    params.append("costscale", q.value("locale_cost_scale").toInt());
  else
    params.append("costscale", decimalPlaces("cost"));

  MetaSQLQuery mql("SELECT item_id, item_number, description, "
                   "       uom_name, scost, acost, "
                   "       CASE WHEN (scost = 0) THEN NULL " 
                   "       ELSE ((acost - scost) / scost)"      
                   "       END AS percent_variance,  "
                   "       'percent' AS percent_variance_xtnumericrole, "
                   "       CASE WHEN (scost = 0) THEN NULL "
                   "       WHEN (((acost - scost) / scost) < 0) THEN 'error' "
                   "       ELSE NULL "
                   "       END AS percent_variance_qtforegroundrole, "
                   " <? if exists(\"onlyShowDiffCosts\") ?>"
                   "       CASE WHEN (scost != acost"
                   "              AND ABS(scost - acost) < 10 ^ (-1 * <? value(\"costscale\") ?>))"
                   "            THEN 'altemphasis' END AS qtforegroundrole,"
                   " <? endif ?>" 
                   "       'cost' AS scost_xtnumericrole,"
                   "       'cost' AS acost_xtnumericrole "
                   "FROM ( SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
                   "              uom_name, stdcost(item_id) AS scost, actcost(item_id) AS acost"
                   "       FROM item, classcode, uom"
                   "       WHERE ((item_classcode_id=classcode_id)"
                   "         AND  (item_inv_uom_id=uom_id)"
                   " <? if exists(\"classcode_id\") ?>"
                   "      AND (classcode_id=<? value(\"classcode_id\") ?>)"
                   " <? elseif exists(\"classcode_pattern\") ?>"
                   "      AND (classcode_code ~ <? value(\"classcode_pattern\") ?>)"
                   " <? endif ?>" 
                   ") ) AS data "
                   "WHERE ( true "
                   " <? if exists(\"onlyShowZeroCosts\") ?>"
                   "    AND ((scost=0) OR (acost=0)) "
                   " <? endif ?>" 
                   " <? if exists(\"onlyShowDiffCosts\") ?>"
                   "    AND (scost != acost) "
                   " <? endif ?>" 
                   ") ORDER BY item_number;");
  q = mql.toQuery(params);
  _itemcost->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
