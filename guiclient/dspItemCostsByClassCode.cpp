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

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "updateActualCostsByItem.h"
#include "postCostsByItem.h"

/*
 *  Constructs a dspItemCostsByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemCostsByClassCode::dspItemCostsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);

  _itemcost->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _itemcost->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "description"   );
  _itemcost->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true,  "scost"  );
  _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true,  "acost"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemCostsByClassCode::~dspItemCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemCostsByClassCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemCostsByClassCode::set(const ParameterList &pParams)
{
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

void dspItemCostsByClassCode::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);

  if(_onlyShowZero->isChecked())
    params.append("onlyShowZeroCosts");

  if(_onlyShowDiff->isChecked())
    params.append("onlyShowDiffCosts");

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
  QString sql( "SELECT item_id, item_number, description,"
               "       uom_name, scost, acost,"
               "       'cost' AS scost_xtnumericrole,"
               "       'cost' AS acost_xtnumericrole "
               "FROM ( SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "              uom_name, stdcost(item_id) AS scost, actcost(item_id) AS acost"
               "       FROM item, classcode, uom"
               "       WHERE ((item_classcode_id=classcode_id)"
               "         AND  (item_inv_uom_id=uom_id)" );

  if (_classCode->isSelected())
    sql += " AND (classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (classcode_code ~ :classcode_pattern)";

  sql += ") ) AS data "
         "WHERE ( (true) ";

  if (_onlyShowZero->isChecked())
    sql += "AND ((scost=0) OR (acost=0)) ";

  if (_onlyShowDiff->isChecked())
    sql += "AND (scost != acost) ";

  sql += ") ORDER BY item_number";

  q.prepare(sql);
  _classCode->bindValue(q);
  q.exec();
  _itemcost->populate(q);
}
