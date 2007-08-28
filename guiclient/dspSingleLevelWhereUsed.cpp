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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "dspSingleLevelWhereUsed.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <qworkspace.h>
#include "item.h"
#include "bom.h"
#include "boo.h"
#include "dspInventoryHistoryByItem.h"
#include "rptSingleLevelWhereUsed.h"

/*
 *  Constructs a dspSingleLevelWhereUsed as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSingleLevelWhereUsed::dspSingleLevelWhereUsed(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_effective, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_bomitem, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSingleLevelWhereUsed::~dspSingleLevelWhereUsed()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSingleLevelWhereUsed::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspSingleLevelWhereUsed::init()
{
  statusBar()->hide();

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _effective->setNullString(tr("Now"));
  _effective->setNullDate(omfgThis->startOfTime());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();

  _bomitem->addColumn(tr("Seq #"),       40,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Parent Item"), _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft   );
  _bomitem->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter );
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));

  _item->setFocus();
}

enum SetResponse dspSingleLevelWhereUsed::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

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
  params.append("print");

  if(!_effective->isNull())
    params.append("effective", _effective->date());

  rptSingleLevelWhereUsed newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspSingleLevelWhereUsed::sPopulateMenu(Q3PopupMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Bill of Materials..."), this, SLOT(sEditBOM()), 0);
  if (!_privleges->check("MaintainBOMs"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Bill of Operations..."), this, SLOT(sEditBOO()), 0);
  if (!_privleges->check("MaintainBOOs"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Edit Item Master..."), this, SLOT(sEditItem()), 0);
  if (!_privleges->check("MaintainItemMasters"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
  if (!_privleges->check("ViewInventoryHistory"))
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
                 "       item_number, (item_descrip1 || ' ' || item_descrip2),"
                 "       item_invuom, formatQtyper(bomitem_qtyper),"
                 "       formatScrap(bomitem_scrap),"
                 "       formatDate(bomitem_effective, 'Always'),"
                 "       formatDate(bomitem_expires, 'Never') "
                 "FROM bomitem, item "
                 "WHERE ( (bomitem_parent_item_id=item_id)"
                 " AND (bomitem_item_id=:item_id)" );

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
