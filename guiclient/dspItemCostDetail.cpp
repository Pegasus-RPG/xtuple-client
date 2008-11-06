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

#include "dspItemCostDetail.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "bomItem.h"

dspItemCostDetail::dspItemCostDetail(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _costTypeGroupInt = new QButtonGroup(this);
  _costTypeGroupInt->addButton(_standardCosts);
  _costTypeGroupInt->addButton(_actualCosts);

  connect(_costTypeGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_costType, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _bom->addColumn(tr("#"),               _seqColumn,   Qt::AlignCenter,true, "seqnumber");
  _bom->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _bom->addColumn(tr("Description"),     -1,           Qt::AlignLeft,  true, "itemdescrip");
  _bom->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter,true, "uom_name");
  _bom->addColumn(tr("Qty. Per"),        _qtyColumn,   Qt::AlignRight, true, "qtyper");
  _bom->addColumn(tr("Scrap/Absorb. %"), _itemColumn,  Qt::AlignRight, true, "scrap" );
  _bom->addColumn(tr("Unit Cost"),       _costColumn,  Qt::AlignRight, true, "cost");
  _bom->addColumn(tr("Ext'd Cost"),      _moneyColumn, Qt::AlignRight, true, "extendedcost");

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

dspItemCostDetail::~dspItemCostDetail()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemCostDetail::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemCostDetail::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("costtype", &valid);
  if (valid)
  {
    for (int cursor = 0; cursor < _costType->count(); cursor++)
    {
      if (_costType->text(cursor) == param.toString())
      {
        _costType->setCurrentIndex(cursor);
        break;
      }
    }
  }

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT itemcost_item_id, itemcost_costelem_id "
               "FROM itemcost "
               "WHERE (itemcost_id=:itemcost_id)" );
    q.bindValue(":itemcost_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("itemcost_item_id").toInt());
      _item->setReadOnly(TRUE);
      _costType->setId(q.value("itemcost_costelem_id").toInt());
      _costType->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostDetail::sPopulate()
{
  q.prepare( "SELECT costelem_id, costelem_type "
             "FROM costelem, itemcost "
             "WHERE ( (itemcost_costelem_id=costelem_id)"
             " AND (itemcost_lowlevel)"
             " AND (itemcost_item_id=:item_id) );" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  _costType->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspItemCostDetail::setParams(ParameterList &params)
{
  params.append("item_id", _item->id());
  params.append("costelem_id", _costType->id());

  if (_standardCosts->isChecked())
    params.append("standardCost");
  else
    params.append("actualCost");

  if ( (_item->itemType() == "M") ||
       (_item->itemType() == "F") ||
       (_item->itemType() == "B") ||
       (_item->itemType() == "T") ||
       (_item->itemType() == "Y") ||
       (_item->itemType() == "R") ||
       (_item->itemType() == "O") ||
       (_item->itemType() == "P") )
    params.append("useBOM");
  else if (_item->itemType() == "C")
    params.append("useBBOM");

  return true;
}

void dspItemCostDetail::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ItemCostDetail", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostDetail::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View BOM Item..."),                  this, SLOT(sViewBomitem()),        0 );
  menuThis->insertItem(tr("View Material Costing Detail..."),   this, SLOT(sViewMaterialCosting()), 0 );
}

void dspItemCostDetail::sViewBomitem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", _bom->id());

  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemCostDetail::sViewMaterialCosting()
{
  ParameterList params;
  params.append("item_id", _bom->altId());
  params.append("costtype", "Material");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostDetail::sFillList()
{
  sFillList(-1, TRUE);
}

void dspItemCostDetail::sFillList(int pItemid, bool pLocale)
{
  if (! pLocale && (pItemid != _item->id()))
    return;

  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(
      "SELECT id, item.item_id, seqnumber, item_number,"
      "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
      "       qtyper,       'qtyper' AS qtyper_xtnumericrole,"
      "       scrap,        'scrap' AS scrap_xtnumericrole,"
      "       cost,         'cost' AS cost_xtnumericrole,"
      "       extendedcost, 'curr' AS extendedcost_xtnumericrole,"
      "       0 AS extendedcost_xttotalrole "
      "FROM uom, item, ("
      "<? if exists(\"useBOM\") ?>"
      "     SELECT bomitem_id AS id, bomitem_seqnumber AS seqnumber,"
      "            itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
      "            bomitem_scrap AS scrap,"
      "            bomitem_item_id AS item_id,"
      "  <? if exists(\"standardCost\") ?>"
      "            itemcost_stdcost AS cost,"
      "            (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL,"
      "                          bomitem_qtyper * (1 + bomitem_scrap)) *"
      "             itemcost_stdcost) AS extendedcost "
      "  <? elseif exists(\"actualCost\") ?>"
      "            itemcost_actcost AS cost,"
      "            (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL,"
      "                          bomitem_qtyper * (1 + bomitem_scrap)) *"
      "             itemcost_actcost) AS extendedcost "
      "  <? endif ?>"
      "    FROM bomitem LEFT OUTER JOIN rev ON (bomitem_rev_id=rev_id),"
      "         itemcost, costelem "
      "    WHERE ((CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))"
      "     AND (COALESCE(rev_status, 'A')='A')"
      "     AND (itemcost_item_id=bomitem_item_id)"
      "     AND (itemcost_costelem_id=costelem_id)"
      "     AND (bomitem_parent_item_id=<? value(\"item_id\") ?>)"
      "     AND (costelem_id=<? value(\"costelem_id\") ?>) ) "
      "<? elseif exists(\"useBBOM\") ?>"
      "     SELECT bbomitem_id AS id, bbomitem_seqnumber AS seqnumber,"
      "            bbomitem_qtyper AS qtyper,"
      "            bbomitem_costabsorb AS scrap,"
      "            bbomparent_parent_item_id AS item_id,"
      "  <? if exists(\"standardCost\") ?>"
      "            itemcost_stdcost AS cost,"
      "            (itemcost_stdcost / bbomitem_qtyper *"
      "             bbomitem_costabsorb) AS extendedcost "
      "  <? elseif exists(\"actualCost\") ?>"
      "            itemcost_actcost AS cost,"
      "            (itemcost_actcost / bbomitem_qtyper *"
      "             bbomitem_costabsorb) AS extendedcost "
      "  <? endif ?>"
      "    FROM bbomitem, itemcost "
      "    WHERE ((CURRENT_DATE BETWEEN bbomitem_effective AND (bbomitem_expires-1))"
      "     AND (itemcost_item_id=bbomitem_parent_item_id)"
      "     AND (itemcost_costelem_id=<? value(\"costelem_id\") ?>)"
      "     AND (bbomitem_item_id=<? value(\"item_id\") ?>) )"
      "    UNION"
      "    SELECT source.bbomitem_id, source.bbomitem_seqnumber AS seqnumber,"
      "           source.bbomitem_qtyper, target.bbomitem_costabsorb,"
      "           item_id,"
      "  <? if exists(\"standardCost\") ?>"
      "           itemcost_stdcost AS cost,"
      "           (itemcost_stdcost * source.bbomitem_qtyper /"
      "            target.bbomitem_qtyper * target.bbomitem_costabsorb) AS extendedcost "
      "  <? elseif exists(\"actualCost\") ?>"
      "           itemcost_actcost AS cost,"
      "           (itemcost_actcost * source.bbomitem_qtyper /"
      "            target.bbomitem_qtyper * target.bbomitem_costabsorb) AS extendedcost "
      "  <? endif ?>"
      "    FROM item, itemcost, bbomitem AS target, bbomitem AS source "
      "    WHERE ( (source.bbomitem_parent_item_id=target.bbomitem_parent_item_id)"
      "     AND (CURRENT_DATE BETWEEN source.bbomitem_effective AND (source.bbomitem_expires-1))"
      "     AND (CURRENT_DATE BETWEEN target.bbomitem_effective AND (target.bbomitem_expires-1))"
      "     AND (source.bbomitem_item_id=itemcost_item_id)"
      "     AND (source.bbomitem_item_id=item_id)"
      "     AND (item_type='Y')"
      "     AND (target.bbomitem_item_id=<? value(\"item_id\") ?>)"
      "     AND (itemcost_costelem_id=<? value(\"costelem_id\") ?>) )"
      "<? endif ?>"
      ") AS data "
      "WHERE ((data.item_id=item.item_id)"
      "   AND (item_inv_uom_id=uom_id)) "
      "ORDER BY seqnumber;");

  q = mql.toQuery(params);
  _bom->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
