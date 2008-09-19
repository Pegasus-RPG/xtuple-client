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

#include "dspCostedIndentedBOM.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"

dspCostedIndentedBOM::dspCostedIndentedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  _revision->setType("BOM");
  _revision->setMode(RevisionLineEdit::View);

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_costsGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_item,                  SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(omfgThis,   SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(_print,                  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_revision,              SIGNAL(newId(int)), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),       _itemColumn, Qt::AlignLeft,  true, "bomdata_bomwork_seqnumber");
  _bomitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),          -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  _bomitem->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Ext. Qty. Per"),_qtyColumn, Qt::AlignRight, true, "bomdata_qtyper");
  _bomitem->addColumn(tr("Scrap %"),    _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  _bomitem->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter,true, "bomdata_effective");
  _bomitem->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter,true, "bomdata_expires");
  _bomitem->addColumn(tr("Unit Cost"),   _costColumn, Qt::AlignRight, true, "unitcost");
  _bomitem->addColumn(tr("Ext. Cost"), _priceColumn, Qt::AlignRight, true, "extendedcost");

  _bomitem->setIndentation(10);

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspCostedIndentedBOM::~dspCostedIndentedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCostedIndentedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCostedIndentedBOM::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    param = pParams.value("revision_id", &valid);
    if (valid)
      _revision->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspCostedIndentedBOM::setParams(ParameterList &params)
{
  if (! _item->isValid())
  {
    QMessageBox::critical(this, tr("Must Supply Item"),
                          tr("<p>You must supply an Item to see its BOM."));
    _item->setFocus();
    return false;
  }

  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if(_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if(_useActualCosts->isChecked())
    params.append("useActualCosts");

  params.append("always", tr("Always"));
  params.append("never",  tr("Never"));

  return true;
}

void dspCostedIndentedBOM::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CostedIndentedBOM", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
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
  if (! _item->isValid())
    return;

  MetaSQLQuery mql( "SELECT bomdata_bomwork_id AS id,"
                    "       CASE WHEN bomdata_bomwork_parent_id = -1 AND "
                    "                 bomdata_bomwork_id = -1 THEN"
                    "                     -1"
                    "            ELSE bomdata_item_id"
                    "       END AS altid,"
                    "       *,"
                    "<? if exists(\"useStandardCosts\") ?>"
                    "       bomdata_stdunitcost AS unitcost,"
                    "       bomdata_stdextendedcost AS extendedcost, "
                    "<? elseif exists(\"useActualCosts\") ?>"
                    "       bomdata_actunitcost AS unitcost,"
                    "       bomdata_actextendedcost AS extendedcost, "
                    "<? endif ?>"
                    "       'qtyper' AS bomdata_qtyper_xtnumericrole,"
                    "       'percent' AS bomdata_scrap_xtnumericrole,"
                    "       'cost' AS unitcost_xtnumericrole,"
                    "       'cost' AS extendedcost_xtnumericrole,"
                    "       CASE WHEN COALESCE(bomdata_effective, startOfTime()) <= startOfTime() THEN <? value(\"always\") ?> END AS bomdata_effective_qtdisplayrole,"
                    "       CASE WHEN COALESCE(bomdata_expires, endOfTime()) <= endOfTime() THEN <? value(\"never\") ?> END AS bomdata_expires_qtdisplayrole,"
                    "       CASE WHEN bomdata_expired THEN 'expired'"
                    "            WHEN bomdata_future  THEN 'future'"
                    "       END AS qtforegroundrole,"
                    "       bomdata_bomwork_level - 1 AS xtindentrole,"
                    "       0 as extendedcost_xttotalrole "
                    "FROM indentedbom(<? value(\"item_id\") ?>,"
                    "                 <? value(\"revision_id\") ?>,0,0);" );
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _bomitem->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
             "       formatCost(stdcost(:item_id)) AS standard;" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *last = new XTreeWidgetItem(_bomitem, -1, -1);
    last->setText(0, tr("Actual Cost"));
    last->setText(9, q.value("actual").toString());

    last = new XTreeWidgetItem( _bomitem, -1, -1);
    last->setText(0, tr("Standard Cost"));
    last->setText(9, q.value("standard").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _bomitem->expandAll();
}

void dspCostedIndentedBOM::sFillList(int p, bool)
{
  if (p == _item->id())
    sFillList();
}
