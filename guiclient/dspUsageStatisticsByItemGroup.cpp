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

#include "dspUsageStatisticsByItemGroup.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <parameter.h>
#include <qworkspace.h>
#include <dbtools.h>
#include "dspInventoryHistoryByItem.h"
#include "rptUsageStatisticsByItemGroup.h"

/*
 *  Constructs a dspUsageStatisticsByItemGroup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUsageStatisticsByItemGroup::dspUsageStatisticsByItemGroup(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_usage, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*,int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUsageStatisticsByItemGroup::~dspUsageStatisticsByItemGroup()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUsageStatisticsByItemGroup::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspUsageStatisticsByItemGroup::init()
{
  statusBar()->hide();

  _itemGroup->setType(ItemGroup);

  _usage->addColumn(tr("Whs."),        _whsColumn,  Qt::AlignCenter );
  _usage->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _usage->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _usage->addColumn(tr("Received"),    _qtyColumn,  Qt::AlignRight  );
  _usage->addColumn(tr("Issued"),      _qtyColumn,  Qt::AlignRight  );
  _usage->addColumn(tr("Sold"),        _qtyColumn,  Qt::AlignRight  );
  _usage->addColumn(tr("Scrap"),       _qtyColumn,  Qt::AlignRight  );
  _usage->addColumn(tr("Adjustments"), _qtyColumn,  Qt::AlignRight  );
}

void dspUsageStatisticsByItemGroup::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("print");
  _warehouse->appendValue(params);
  _itemGroup->appendValue(params);

  rptUsageStatisticsByItemGroup newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspUsageStatisticsByItemGroup::sViewAll()
{
  viewTransactions(NULL);
}

void dspUsageStatisticsByItemGroup::sViewReceipt()
{
  viewTransactions("R");
}

void dspUsageStatisticsByItemGroup::sViewIssue()
{
  viewTransactions("I");
}

void dspUsageStatisticsByItemGroup::sViewSold()
{
  viewTransactions("S");
}

void dspUsageStatisticsByItemGroup::sViewScrap()
{
  viewTransactions("SC");
}

void dspUsageStatisticsByItemGroup::sViewAdjustment()
{
  viewTransactions("A");
}

void dspUsageStatisticsByItemGroup::viewTransactions(QString pType)
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("itemsite_id", _usage->id());
  params.append("run");

  if (!pType.isNull())
    params.append("transtype", pType);

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUsageStatisticsByItemGroup::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *, int pColumn)
{
  int menuItem;

  menuItem = pMenu->insertItem("View All Transactions...", this, SLOT(sViewAll()), 0);
  if (!_privleges->check("ViewInventoryHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  switch (pColumn)
  {
    case 3:
      menuItem = pMenu->insertItem("View Receipt Transactions...", this, SLOT(sViewReceipt()), 0);
      if (!_privleges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 4:
      menuItem = pMenu->insertItem("View Issue Transactions...", this, SLOT(sViewIssue()), 0);
      if (!_privleges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 5:
      menuItem = pMenu->insertItem("View Sold Transactions...", this, SLOT(sViewSold()), 0);
      if (!_privleges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 6:
      menuItem = pMenu->insertItem("View Scrap Transactions...", this, SLOT(sViewScrap()), 0);
      if (!_privleges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;

    case 7:
      menuItem = pMenu->insertItem("View Adjustment Transactions...", this, SLOT(sViewAdjustment()), 0);
      if (!_privleges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);
      break;
  }
}

void dspUsageStatisticsByItemGroup::sFillList()
{
  _usage->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  QString sql( "SELECT itemsite_id, warehous_code,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2),"
               "       formatQty(summTransR(itemsite_id, :startDate, :endDate)),"
               "       formatQty(summTransI(itemsite_id, :startDate, :endDate)),"
               "       formatQty(summTransS(itemsite_id, :startDate, :endDate)),"
               "       formatQty(summTransC(itemsite_id, :startDate, :endDate)),"
               "       formatQty(summTransA(itemsite_id, :startDate, :endDate)) "
               "FROM item, itemsite, warehous, itemgrp, itemgrpitem "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemgrpitem_itemgrp_id=itemgrp_id)"
               " AND (itemgrpitem_item_id=item_id)" );

  if (_itemGroup->isSelected())
    sql += " AND (itemgrp_id=:itemgrp_id)";
  else if (_itemGroup->isPattern())
    sql += " AND (itemgrp_name ~ :itemgrp_pattern)";

  if (_warehouse->isSelected())
    sql += "AND (itemsite_warehous_id=:warehous_id)";

  sql += " ) "
         "ORDER BY warehous_code, item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _itemGroup->bindValue(q);
  _dates->bindValue(q);
  q.exec();
  _usage->populate(q);
}

