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


#include "itemDock.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include "itemcluster.h"
#include "xlistview.h"

ItemDock::ItemDock(QWidget *pParent, const char *pName, Qt::WFlags pFlags) :
 Q3DockWindow(pParent, pName, pFlags)
{
  _item = new XListView(this, "_item");
  boxLayout()->addWidget(_item);

  _item->addColumn(tr("Item Number"), 100, Qt::AlignLeft   );
  _item->addColumn(tr("Description"), -1,  Qt::AlignLeft   );
  _item->addColumn(tr("Type"),        80,  Qt::AlignCenter );
  _item->setSorting(0);
  _item->setDragString("itemid=");

  setResizeEnabled(TRUE);
  setCloseMode(Always);

  sFillList();

  resize(QSize(350, 500));
}

void ItemDock::sSearch(const QString &pTarget)
{
  Q3ListViewItem *target;

  if (_item->selectedItem())
    _item->setSelected(_item->selectedItem(), FALSE);

  _item->clearSelection();

  target = _item->firstChild();
  while ((target != NULL) && (pTarget.upper() != target->text(0).left(pTarget.length())))
    target = target->nextSibling();

  if (target != NULL)
  {
    _item->setSelected(target, TRUE);
    _item->ensureItemVisible(target);
  }
}

void ItemDock::sFillList()
{
  XSqlQuery q;
  q.prepare( "SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2),"
             "       CASE WHEN (item_type='P') THEN :purchased"
             "            WHEN (item_type='M') THEN :manufactured" 
             "            WHEN (item_type='F') THEN :phantom"
             "            WHEN (item_type='B') THEN :breeder"
             "            WHEN (item_type='C') THEN :coProduct"
             "            WHEN (item_type='Y') THEN :byProduct"
             "            WHEN (item_type='R') THEN :reference"
             "            WHEN (item_type='S') THEN :costing"
             "            WHEN (item_type='T') THEN :tooling"
             "            WHEN (item_type='A') THEN :assortment"
             "            WHEN (item_type='O') THEN :outside"
             "            ELSE :error"
             "       END "
             "FROM item "
             "WHERE (item_active) "
             "ORDER BY item_number;" );
  q.bindValue(":purchased", tr("Purch."));
  q.bindValue(":manufactured", tr("Mfg."));
  q.bindValue(":phantom", tr("Phantom"));
  q.bindValue(":breeder", tr("Breeder"));
  q.bindValue(":coProduct", tr("Co-Prod."));
  q.bindValue(":byProduct", tr("By-Prod."));
  q.bindValue(":reference", tr("Ref."));
  q.bindValue(":costing", tr("Cost"));
  q.bindValue(":tooling", tr("Tool"));
  q.bindValue(":outside", tr("Outside Proc."));
  q.bindValue(":assortment", tr("Assort."));
  q.bindValue(":error", tr("Error"));
  q.exec();
  _item->populate(q);
}

