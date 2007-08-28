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

#include "dspCostedSingleLevelBOM.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <qworkspace.h>
#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "rptCostedSingleLevelBOM.h"

/*
 *  Constructs a dspCostedSingleLevelBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCostedSingleLevelBOM::dspCostedSingleLevelBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _costsGroupInt = new QButtonGroup(this);
    _costsGroupInt->addButton(_useStandardCosts);
    _costsGroupInt->addButton(_useActualCosts);

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_costsGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
    connect(_bomitem, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*)));
    connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCostedSingleLevelBOM::~dspCostedSingleLevelBOM()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCostedSingleLevelBOM::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspCostedSingleLevelBOM::init()
{
  statusBar()->hide();

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _bomitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Unit Cost"),   _costColumn,  Qt::AlignRight  );
  _bomitem->addColumn(tr("Ext'd Cost"),  _costColumn,  Qt::AlignRight  );

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

enum SetResponse dspCostedSingleLevelBOM::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspCostedSingleLevelBOM::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  rptCostedSingleLevelBOM newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspCostedSingleLevelBOM::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected)
{
  if (((XListViewItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);

  if (((XListViewItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("View Item Costing..."), this, SLOT(sViewItemCosting()), 0);
}

void dspCostedSingleLevelBOM::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());

  maintainItemCosts *newdlg = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedSingleLevelBOM::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", _bomitem->altId() );
  params.append( "run",     TRUE              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedSingleLevelBOM::sFillList()
{
  sFillList(-1, TRUE);
}

void dspCostedSingleLevelBOM::sFillList(int pItemid, bool)
{
  if ( (pItemid == _item->id()) ||
       ( (pItemid == -1) && (_item->isValid()) ) )
  {
    _bomitem->clear();

    QString sql( "SELECT bomitem_id, bomitem_item_id AS item_id,"
                 "       bomitem_seqnumber, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription, item_invuom,"
                 "       formatQtyper(bomitem_qtyper) AS f_qtyper,"
                 "       formatScrap(bomitem_scrap) AS f_scrap,"
                 "       formatDate(bomitem_effective, 'Always') AS f_effective,"
                 "       formatDate(bomitem_expires, 'Never') AS f_expires," );

     if (_useStandardCosts->isChecked())
       sql += " formatCost(stdcost(bomitem_item_id)) AS f_unitcost,"
              " formatCost(bomitem_qtyper * (1 + bomitem_scrap) * stdcost(bomitem_item_id)) AS f_extendedcost,"
              " (bomitem_qtyper * (1 + bomitem_scrap) * stdcost(bomitem_item_id)) AS extendedcost,";
     else if (_useActualCosts->isChecked())
       sql += " formatCost(actcost(bomitem_item_id)) AS f_unitcost,"
              " formatCost(bomitem_qtyper * (1 + bomitem_scrap) * actcost(bomitem_item_id)) AS f_extendedcost,"
              " (bomitem_qtyper * (1 + bomitem_scrap) * actcost(bomitem_item_id)) AS extendedcost,";

    sql += " bomitem_effective AS effective "
           "FROM bomitem, item "
           "WHERE ( (bomitem_item_id=item_id)"
           " AND (bomitem_parent_item_id=:item_id)"
           " AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires - 1)) ) "

           "UNION SELECT -1 AS bomitem_id, -1 AS item_id,"
           "             -1 AS bomitem_seqnumber, costelem_type AS item_number,"
           "             '' AS itemdescription, '' AS item_invuom,"
           "             '' AS qtyper, '' AS scrap, '' AS f_effective, '' AS f_expires,"
           "             '' AS f_unitcost,";

     if (_useStandardCosts->isChecked())
       sql += " formatCost(itemcost_stdcost) AS f_extendedcost,"
              " itemcost_stdcost AS extendedcost,";
     else if (_useActualCosts->isChecked())
       sql += " formatCost(currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE)) AS f_extendedcost,"
              " currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS extendedcost,";

     sql += " startOfTime() AS effective "
            "FROM itemcost, costelem "
            "WHERE ( (itemcost_costelem_id=costelem_id)"
            " AND (NOT itemcost_lowlevel)"
            " AND (itemcost_item_id=:item_id) ) "

            "ORDER BY bomitem_seqnumber, effective, item_number;";

    q.prepare(sql);
    q.bindValue(":item_id", _item->id());
    q.exec();
    XListViewItem *last = NULL;
    double        totalCost = 0;

    while (q.next())
    {
      if (q.value("bomitem_id") == -1)
        last = new XListViewItem( _bomitem, last, -1, -1, "", q.value("item_number").toString());
      else
        last = new XListViewItem( _bomitem, last, q.value("bomitem_id").toInt(), q.value("item_id").toInt(),
                                  q.value("bomitem_seqnumber").toString(), q.value("item_number").toString(),
                                  q.value("itemdescription").toString(), q.value("item_invuom").toString(),
                                  q.value("f_qtyper").toString(), q.value("f_scrap").toString(),
                                  q.value("f_effective").toString(), q.value("f_expires").toString() );

      last->setText(8, q.value("f_unitcost").toString());
      last->setText(9, q.value("f_extendedcost").toString());

      totalCost += q.value("extendedcost").toDouble();
    }

    last = new XListViewItem(_bomitem, last, -1, -1, "", tr("Total Cost"));
    last->setText(9, formatCost(totalCost));

    q.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
               "       formatCost(stdcost(:item_id)) AS standard;" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      last = new XListViewItem(_bomitem, last, -1, -1, "", tr("Actual Cost"));
      last->setText(9, q.value("actual").toString());

      last = new XListViewItem(_bomitem, last, -1, -1, "", tr("Standard Cost") );
      last->setText(9, q.value("standard").toString());
    }
  }
}
