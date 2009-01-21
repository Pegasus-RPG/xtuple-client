/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSingleLevelWhereUsed.h"

#include <QMenu>
#include <QVariant>

#include <openreports.h>

#include "bom.h"
#include "boo.h"
#include "dspInventoryHistoryByItem.h"
#include "item.h"

dspSingleLevelWhereUsed::dspSingleLevelWhereUsed(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_effective, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _effective->setNullString(tr("Today"));
  _effective->setNullDate(QDate::currentDate());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();

  _bomitem->addColumn(tr("Seq #"),       40,           Qt::AlignCenter, true, "bomitem_seqnumber");
  _bomitem->addColumn(tr("Parent Item"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "descrip");
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,  true, "uom_name");
  _bomitem->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight, true, "qtyper");
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight, true, "bomitem_scrap");
  _bomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter,true, "bomitem_effective");
  _bomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter,true, "bomitem_expires");
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));

  _item->setFocus();
}

dspSingleLevelWhereUsed::~dspSingleLevelWhereUsed()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSingleLevelWhereUsed::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSingleLevelWhereUsed::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _effective->setDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspSingleLevelWhereUsed::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  if(!_effective->isNull())
    params.append("effective", _effective->date());

  orReport report("SingleLevelWhereUsed", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSingleLevelWhereUsed::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Bill of Materials..."), this, SLOT(sEditBOM()), 0);
  if (!_privileges->check("MaintainBOMs"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Bill of Operations..."), this, SLOT(sEditBOO()), 0);
  if (!_privileges->check("MaintainBOOs"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Item Master..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspSingleLevelWhereUsed::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _bomitem->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSingleLevelWhereUsed::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _bomitem->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSingleLevelWhereUsed::sEditItem()
{
  item::editItem(_bomitem->id());
}

void dspSingleLevelWhereUsed::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());
  params.append("warehous_id", -1);
  params.append("run");

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSingleLevelWhereUsed::sFillList()
{
  sFillList(-1, FALSE);
}

void dspSingleLevelWhereUsed::sFillList(int pItemid, bool pLocal)
{
  if ((_item->isValid()) && (_effective->isValid()))
  {
    QString sql( "SELECT bomitem_parent_item_id, item_id, bomitem_seqnumber,"
                 "       item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
                 "       uom_name,"
                 "       itemuomtouom(bomitem_item_id, bomitem_uom_id,"
                 "                    NULL, bomitem_qtyper) AS qtyper,"
                 "       bomitem_scrap,"
                 "       bomitem_effective, bomitem_expires,"
                 "       'qtyper' AS qtyper_xtnumericrole,"
                 "       'scrap' AS bomitem_scrap_xtnumericrole,"
                 "       CASE WHEN (COALESCE(bomitem_effective, startoftime()) = startoftime()) THEN 'Always' END AS bomitem_effective_qtdisplayrole,"
                 "       CASE WHEN (COALESCE(bomitem_expires, endoftime()) = endoftime()) THEN 'Never' END AS bomitem_expires_qtdisplayrole "
		 "FROM bomitem, item, uom "
                 "WHERE ( (bomitem_parent_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (bomitem_item_id=:item_id)"
                 " AND (bomitem_rev_id=getActiveRevId('BOM',bomitem_parent_item_id))");

    if (_effective->isNull())
      sql += "AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))";
    else
      sql += " AND (:effective BETWEEN bomitem_effective AND (bomitem_expires-1))";

    sql += ") ORDER BY item_number";

    q.prepare(sql);
    q.bindValue(":item_id", _item->id());
    q.bindValue(":effective", _effective->date());
    q.exec();

    if (pLocal)
      _bomitem->populate(q, TRUE, pItemid);
    else
      _bomitem->populate(q, TRUE);
  }
  else
    _bomitem->clear();
}
