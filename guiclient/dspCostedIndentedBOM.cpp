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

#include "dspCostedIndentedBOM.h"

#include <QVariant>
#include <QWorkspace>
#include <QStatusBar>
#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"
#include "rptCostedIndentedBOM.h"

/*
 *  Constructs a dspCostedIndentedBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCostedIndentedBOM::dspCostedIndentedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _costsGroupInt = new QButtonGroup(this);
    _costsGroupInt->addButton(_useStandardCosts);
    _costsGroupInt->addButton(_useActualCosts);

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_costsGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
    connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCostedIndentedBOM::~dspCostedIndentedBOM()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCostedIndentedBOM::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspCostedIndentedBOM::init()
{
  _item->setType(ItemLineEdit::cGeneralManufactured);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),         80,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext. Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),     _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),       _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Unit Cost"),     _costColumn,  Qt::AlignRight  );
  _bomitem->addColumn(tr("Ext'd Cost"),    _priceColumn, Qt::AlignRight  );
  _bomitem->setIndentation(10);
}

enum SetResponse dspCostedIndentedBOM::set(ParameterList &pParams)
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

void dspCostedIndentedBOM::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  rptCostedIndentedBOM newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspCostedIndentedBOM::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("View Item Costing..."), this, SLOT(sViewItemCosting()), 0);
}

void dspCostedIndentedBOM::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());

  maintainItemCosts *newdlg  = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedIndentedBOM::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", _bomitem->altId() );
  params.append( "run",     TRUE              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedIndentedBOM::sFillList()
{
  double totalCosts = 0;

  _bomitem->clear();

  if (_item->isValid())
  {
    q.prepare("SELECT indentedBOM(:item_id) AS bomwork_set_id;");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      int _worksetid = q.value("bomwork_set_id").toInt();

      QString sql( "SELECT bomwork_id, item_id, bomwork_parent_id,"
                   "       bomwork_seqnumber, item_number, item_invuom,"
                   "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
                   "       formatQtyPer(bomwork_qtyper) AS qtyper,"
                   "       formatScrap(bomwork_scrap) AS scrap,"
                   "       formatDate(bomwork_effective, 'Always') AS effective,"
                   "       formatDate(bomwork_expires, 'Never') AS expires," );

     if (_useStandardCosts->isChecked())
       sql += " formatCost(bomwork_stdunitcost) AS f_unitcost,"
              " formatCost(bomwork_qtyper * (1 + bomwork_scrap) * bomwork_stdunitcost) AS f_extendedcost,"
              " (bomwork_qtyper * (1 + bomwork_scrap) * bomwork_stdunitcost) AS extendedcost,";
     else if (_useActualCosts->isChecked())
       sql += " formatCost(bomwork_actunitcost) AS f_unitcost,"
              " formatCost(bomwork_qtyper * (1 + bomwork_scrap) * bomwork_actunitcost) AS f_extendedcost,"
              " (bomwork_qtyper * (1 + bomwork_scrap) * bomwork_actunitcost) AS extendedcost,";

      sql += " bomwork_level "
             "FROM bomwork, item "
             "WHERE ((bomwork_item_id=item_id)"
             " AND (bomwork_set_id=:bomwork_set_id)"
             " AND (CURRENT_DATE BETWEEN bomwork_effective AND (bomwork_expires - 1))) "

             "UNION SELECT -1 AS bomwork_id, -1 AS item_id, -1 AS bomwork_parent_id,"
             "             99999 AS bomwork_seqnumber, costelem_type AS item_number, '' AS item_invuom,"
             "             '' AS itemdescription,"
             "             '' AS qtyper, '' AS scrap, '' AS effective, '' AS expires,";

     if (_useStandardCosts->isChecked())
       sql += " formatCost(itemcost_stdcost) AS f_unitcost,"
              " formatCost(itemcost_stdcost) AS f_extendedcost,"
              " itemcost_stdcost AS extendedcost,";
     else if (_useActualCosts->isChecked())
       sql += " formatCost(currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE)) AS f_unitcost,"
              " formatCost(currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE)) AS f_extendedcost,"
              " currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS extendedcost,";

     sql += " -1 AS bomwork_level "
            "FROM itemcost, costelem "
            "WHERE ( (itemcost_costelem_id=costelem_id)"
            " AND (NOT itemcost_lowlevel)"
            " AND (itemcost_item_id=:item_id) ) "

            "ORDER BY bomwork_level, bomwork_seqnumber, item_number;";

      q.prepare(sql);
      q.bindValue(":bomwork_set_id", _worksetid);
      q.bindValue(":item_id", _item->id());
      q.exec();
      XTreeWidgetItem *last = NULL;

      while (q.next())
      {
        if (q.value("bomwork_parent_id").toInt() == -1)
        {
          if (q.value("bomwork_id").toInt() == -1)
          {
            last = new XTreeWidgetItem( _bomitem, last, -1, -1,
                                      "", q.value("item_number"),
                                      "", "", "", "", "" );
            last->setText(7, "");
            last->setText(8, q.value("f_unitcost").toString());
            last->setText(9, q.value("f_extendedcost").toString());
          }
          else
          {
            last = new XTreeWidgetItem( _bomitem, last, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                                      q.value("bomwork_seqnumber"), q.value("item_number"),
                                      q.value("itemdescription"), q.value("item_invuom"),
                                      q.value("qtyper"), q.value("scrap"),
                                      q.value("effective"), q.value("expires"),
                                      q.value("f_unitcost"), q.value("f_extendedcost") );
          }

          totalCosts += q.value("extendedcost").toDouble();
        }
        else
        {
          XTreeWidgetItem *cursor = 0;
	  int i;
	  for (i = 0; i < _bomitem->topLevelItemCount(); i++)
	  {
	    cursor = (XTreeWidgetItem*)(_bomitem->topLevelItem(i));
	    if (cursor->id() == q.value("bomwork_parent_id").toInt())
	      break;
	  }

          if (cursor && i < _bomitem->topLevelItemCount())
          {
	    cursor->addChild(new XTreeWidgetItem(cursor,
						q.value("bomwork_id").toInt(),
						q.value("item_id").toInt(),
						q.value("bomwork_seqnumber"),
						q.value("item_number"),
						q.value("itemdescription"),
						q.value("item_invuom"),
						q.value("qtyper"),
						q.value("scrap"),
						q.value("effective"),
						q.value("expires"),
						q.value("f_unitcost"),
						q.value("f_extendedcost") ));
	    cursor->setExpanded(TRUE);
          }
        }
      }
 
      last = new XTreeWidgetItem(_bomitem, last, -1, -1);
      last->setText(1, tr("Total Cost"));
      last->setText(9, formatCost(totalCosts));

      q.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
                 "       formatCost(stdcost(:item_id)) AS standard;" );
      q.bindValue(":item_id", _item->id());
      q.exec();
      if (q.first())
      {
        last = new XTreeWidgetItem(_bomitem, last, -1, -1);
        last->setText(1, tr("Actual Cost"));
        last->setText(9, q.value("actual").toString());

        last = new XTreeWidgetItem( _bomitem, last, -1, -1);
        last->setText(1, tr("Standard Cost"));
        last->setText(9, q.value("standard").toString());
      }

      q.prepare( "DELETE FROM bomwork "
                 "WHERE (bomwork_set_id=:bomwork_set_id);" );
      q.bindValue(":bomwork_set_id", _worksetid);
      q.exec();
    }
  }
}
