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

#include "dspCapacityUOMsByClassCode.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <parameter.h>
#include <qworkspace.h>
#include "item.h"
#include "rptCapacityUOMsByClassCode.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspCapacityUOMsByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCapacityUOMsByClassCode::dspCapacityUOMsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCapacityUOMsByClassCode::~dspCapacityUOMsByClassCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCapacityUOMsByClassCode::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspCapacityUOMsByClassCode::init()
{
  statusBar()->hide();

  _classCode->setType(ClassCode);

  _item->addColumn(tr("Class Code"),     _itemColumn, Qt::AlignCenter );
  _item->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft   );
  _item->addColumn(tr("Description"),    -1,          Qt::AlignLeft   );
  _item->addColumn(tr("Inv. UOM"),       _uomColumn,  Qt::AlignCenter );
  _item->addColumn(tr("Cap. UOM"),       _uomColumn,  Qt::AlignCenter );
  _item->addColumn(tr("Cap./Inv. Rat."), _qtyColumn,  Qt::AlignRight  );
  _item->addColumn(tr("Alt. UOM"),       _uomColumn,  Qt::AlignCenter );
  _item->addColumn(tr("Alt/Inv Ratio"),  _qtyColumn,  Qt::AlignRight  );
}

void dspCapacityUOMsByClassCode::sPrint()
{
  ParameterList params;
  params.append("print");
  _classCode->appendValue(params);

  rptCapacityUOMsByClassCode newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspCapacityUOMsByClassCode::sPopulateMenu(Q3PopupMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privleges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspCapacityUOMsByClassCode::sEditItem()
{
  item::editItem(_item->id());
}

void dspCapacityUOMsByClassCode::sFillList()
{
  sFillList(-1, FALSE);
}

void dspCapacityUOMsByClassCode::sFillList(int pItemid, bool pLocalUpdate)
{
  QString sql( "SELECT item_id, classcode_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2), item_invuom,"
               "       item_capuom, formatQty(item_capinvrat), item_altcapuom, formatQty(item_altcapinvrat) "
               "FROM item, classcode "
               "WHERE ( (item_classcode_id=classcode_id)" );

  if (_classCode->isSelected())
    sql += " AND (classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (classcode_code ~ :classcode_pattern)";

  sql += " ) "
         "ORDER BY classcode_code, item_number";

  q.prepare(sql);
  _classCode->bindValue(q);
  q.exec();

  if ((pItemid != -1) && (pLocalUpdate))
    _item->populate(q, pItemid);
  else
    _item->populate(q);
}

