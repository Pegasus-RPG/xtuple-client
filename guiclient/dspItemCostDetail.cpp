/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
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
  XWidget::set(pParams);
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
      "       extendedcost, 'cost' AS extendedcost_xtnumericrole,"
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
