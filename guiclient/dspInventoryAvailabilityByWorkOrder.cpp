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

#include "dspInventoryAvailabilityByWorkOrder.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistoryByItem.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "postMiscProduction.h"
#include "inputManager.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspInventoryAvailabilityByWorkOrder::dspInventoryAvailabilityByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_womatl, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_onlyShowShortages, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_includeChildren, SIGNAL(clicked()), this, SLOT(sFillList()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn("itemType",                 0, Qt::AlignCenter,false,"type");
  _womatl->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _womatl->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "descrip");
  _womatl->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "uom_name");
  _womatl->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "qoh");
  _womatl->addColumn(tr("This Alloc."), _qtyColumn, Qt::AlignRight, true, "wobalance");
  _womatl->addColumn(tr("Total Alloc."),_qtyColumn, Qt::AlignRight, true, "allocated");
  _womatl->addColumn(tr("Orders"),      _qtyColumn, Qt::AlignRight, true, "ordered");
  _womatl->addColumn(tr("This Avail."), _qtyColumn, Qt::AlignRight, true, "woavail");
  _womatl->addColumn(tr("Total Avail."),_qtyColumn, Qt::AlignRight, true, "totalavail");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

dspInventoryAvailabilityByWorkOrder::~dspInventoryAvailabilityByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryAvailabilityByWorkOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByWorkOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  _onlyShowShortages->setChecked(pParams.inList("onlyShowShortages"));

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspInventoryAvailabilityByWorkOrder::setParams(ParameterList &params)
{
  if(!_wo->isValid())
  {
    QMessageBox::warning(this, tr("Invalid W/O Selected"),
                         tr("<p>You must specify a valid Work Order Number.") );
    _wo->setFocus();
    return false;
  }

  params.append("wo_id", _wo->id());

  if(_onlyShowShortages->isChecked())
    params.append("onlyShowShortages");

  if(_includeChildren->isChecked())
  {
    q.prepare("SELECT wo_number FROM wo WHERE wo_id=:wo_id;");
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
      params.append("wo_number", q.value("wo_number").toInt());
  }
  
  return true;
}

void dspInventoryAvailabilityByWorkOrder::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("WOMaterialAvailabilityByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryAvailabilityByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem("View Allocations...", this, SLOT(sViewAllocations()), 0);
  if (selected->text(6).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);
    
  menuItem = pMenu->insertItem("View Orders...", this, SLOT(sViewOrders()), 0);
  if (selected->text(7).toDouble() == 0.0)
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Running Availability...", this, SLOT(sRunningAvailability()), 0);

  pMenu->insertSeparator();

  if (selected->text(0) == "P")
  {
    menuItem = pMenu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
    if (!_privileges->check("MaintainPurchaseRequests"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem("Create P/O...", this, SLOT(sCreatePO()), 0);
    if (!_privileges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }
  else if (selected->text(0) == "M")
  {
    menuItem = pMenu->insertItem("Create W/O...", this, SLOT(sCreateWO()), 0);
    if (!_privileges->check("MaintainWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Post Misc. Production..."), this, SLOT(sPostMiscProduction()), 0);
    if (!_privileges->check("PostMiscProduction"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();
  }

  menuItem = pMenu->insertItem("View Substitute Availability...", this, SLOT(sViewSubstituteAvailability()), 0);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem("Issue Count Tag...", this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()), 0);
  if (!_privileges->check("EnterMiscCounts"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspInventoryAvailabilityByWorkOrder::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewAllocations()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _womatl->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspAllocations *newdlg = new dspAllocations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByWorkOrder::sViewOrders()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _womatl->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspOrders *newdlg = new dspOrders();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByWorkOrder::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewSubstituteAvailability()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _womatl->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
//  ToDo
}

void dspInventoryAvailabilityByWorkOrder::sCreatePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sCreatePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sCreateWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sPostMiscProduction()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());

  postMiscProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());

  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sEnterMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", _womatl->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql("SELECT itemsite_id, womatl_id, type,"
               "       item_number, item_description, uom_name, item_picklist,"
               "       qoh," 
               "       wobalance,"
               "       allocated,"
               "       ordered,"
               "       (qoh + ordered - wobalance) AS woavail,"
               "       (qoh + ordered - allocated) AS totalavail,"
               "       reorderlevel,"
               "       'qty' AS qoh_xtnumericrole,"
               "       'qty' AS wobalance_xtnumericrole,"
               "       'qty' AS allocated_xtnumericrole,"
               "       'qty' AS ordered_xtnumericrole,"
               "       'qty' AS woavail_xtnumericrole,"
               "       'qty' AS totalavail_xtnumericrole,"
               "       CASE WHEN (qoh < 0) THEN 'error'"
               "            WHEN (qoh < reorderlevel) THEN 'warning'"
               "       END AS qoh_qtforegroundrole,"
               "       CASE WHEN ((qoh + ordered - wobalance) < 0) THEN 'error'"
               "            WHEN ((qoh + ordered - wobalance) < reorderlevel) THEN 'warning'"
               "       END AS woavail_qtforegroundrole,"
               "       CASE WHEN ((qoh + ordered - allocated) < 0) THEN 'error'"
               "            WHEN ((qoh + ordered - allocated) < reorderlevel) THEN 'warning'"
               "       END AS totalavail_qtforegroundrole "
               "FROM ( SELECT itemsite_id, womatl_id,"
               "              CASE WHEN itemsite_supply THEN item_type"
               "                   ELSE ''"
               "              END AS type,"
               "              item_number, (item_descrip1 || ' ' || item_descrip2) AS item_description,"
               "              uom_name, item_picklist,"
               "              noNeg(itemsite_qtyonhand) AS qoh,"
               "              noNeg(itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyreq - womatl_qtyiss)) AS wobalance,"
               "              qtyAllocated(itemsite_id, womatl_duedate) AS allocated,"
               "              qtyOrdered(itemsite_id, womatl_duedate) AS ordered,"
               "              CASE WHEN(itemsite_useparams) THEN itemsite_reorderlevel ELSE 0.0 END AS reorderlevel"
               "       FROM wo, womatl, itemsite, item, uom "
               "       WHERE ( (womatl_wo_id=wo_id)"
               "        AND (womatl_itemsite_id=itemsite_id)"
               "        AND (itemsite_item_id=item_id)"
               "        AND (item_inv_uom_id=uom_id)"
               "<? if exists(\"wo_number\") ?>"
               "        AND (wo_number=<? value(\"wo_number\") ?>)) ) AS data "
               "<? else ?>"
               "        AND (womatl_wo_id=<? value(\"wo_id\") ?>)) ) AS data "
               "<? endif ?>"
               "<? if exists(\"onlyShowShortages\") ?>"
               "WHERE ( ((qoh + ordered - allocated) < 0)"
               " OR ((qoh + ordered - wobalance) < 0) ) "
               "<? endif ?>"
               "ORDER BY item_number;");
  q = mql.toQuery(params);
  _womatl->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
