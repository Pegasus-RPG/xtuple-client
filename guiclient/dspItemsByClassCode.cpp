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

#include "dspItemsByClassCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "bom.h"
#include "boo.h"
#include "guiclient.h"
#include "item.h"
#include "mqlutil.h"

dspItemsByClassCode::dspItemsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);

  _item->addColumn( tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _item->addColumn( tr("Description"), -1,          Qt::AlignLeft,  true, "descrip");
  _item->addColumn( tr("Class Code"),  _itemColumn, Qt::AlignLeft,  true, "classcode_code");
  _item->addColumn( tr("Type"),        _itemColumn, Qt::AlignCenter,true, "type");
  _item->addColumn( tr("UOM"),         _uomColumn,  Qt::AlignCenter,true, "uom_name");
  _item->setDragString("itemid=");
}

dspItemsByClassCode::~dspItemsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemsByClassCode::languageChange()
{
  retranslateUi(this);
}

bool dspItemsByClassCode::setParams(ParameterList &params)
{
  _classCode->appendValue(params);

  if(_showInactive->isChecked())
    params.append("showInactive");

  params.append("purchased",    tr("Purchased"));
  params.append("manufactured", tr("Manufactured"));
  params.append("job",          tr("Job"));
  params.append("phantom",      tr("Phantom"));
  params.append("breeder",      tr("Breeder"));
  params.append("coProduct",    tr("Co-Product"));
  params.append("byProduct",    tr("By-Product"));
  params.append("reference",    tr("Reference"));
  params.append("costing",      tr("Costing"));
  params.append("tooling",      tr("Tooling"));
  params.append("outside",      tr("Outside Process"));
  params.append("kit",          tr("Kit"));
  params.append("error",        tr("Error"));

  params.append("byClassCode");

  return true;
}

void dspItemsByClassCode::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("ItemsByClassCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemsByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item Master..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (selected->text(2) == "M")
  {
    menuItem = pMenu->insertItem(tr("Edit Bill of Material..."), this, SLOT(sEditBOM()), 0);
    if (!_privileges->check("MaintainBOMs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Bill of Material..."), this, SLOT(sViewBOM()), 0);
    if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Edit Item Bill of Operations..."), this, SLOT(sEditBOO()), 0);
    if (!_privileges->check("MaintainBOOs"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View Item Bill of Operations..."), this, SLOT(sViewBOO()), 0);
    if ( (!_privileges->check("MaintainBOOs")) && (!_privileges->check("ViewBOOs")) )
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspItemsByClassCode::sEdit()
{
  item::editItem(_item->id());
}

void dspItemsByClassCode::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByClassCode::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByClassCode::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _item->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByClassCode::sViewBOO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _item->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByClassCode::sFillList()
{
  sFillList(-1, TRUE);
}

void dspItemsByClassCode::sFillList(int pItemid, bool pLocal)
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("products", "items");
  q = mql.toQuery(params);
  if ((pItemid != -1) && (pLocal))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
