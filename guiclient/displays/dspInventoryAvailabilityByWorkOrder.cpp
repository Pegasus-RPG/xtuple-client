/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailabilityByWorkOrder.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

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

dspInventoryAvailabilityByWorkOrder::dspInventoryAvailabilityByWorkOrder(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspInventoryAvailabilityByWorkOrder", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability by Work Order"));
  setListLabel(tr("Work Order Material Availability"));
  setReportName("WOMaterialAvailabilityByWorkOrder");
  setMetaSQLOptions("inventoryAvailabilitybyWorkorder", "detail");
  setUseAltId(true);

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  list()->addColumn(tr("WO/Item#"),    120, Qt::AlignLeft,  true, "woinvav_item_wo_number");
  list()->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "woinvav_descrip");
  list()->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "woinvav_uomname");
  list()->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "woinvav_qoh");
  list()->addColumn(tr("This Alloc."), _qtyColumn, Qt::AlignRight, true, "woinvav_balance");
  list()->addColumn(tr("Total Alloc."),_qtyColumn, Qt::AlignRight, true, "woinvav_allocated");
  list()->addColumn(tr("Orders"),      _qtyColumn, Qt::AlignRight, true, "woinvav_ordered");
  list()->addColumn(tr("This Avail."), _qtyColumn, Qt::AlignRight, true, "woinvav_woavail");
  list()->addColumn(tr("Total Avail."),_qtyColumn, Qt::AlignRight, true, "woinvav_totalavail");
  list()->addColumn(tr("Type"),                 0, Qt::AlignLeft, false, "woinvav_type");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspInventoryAvailabilityByWorkOrder::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByWorkOrder::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  _onlyShowShortages->setChecked(pParams.inList("onlyShowShortages"));

  _onlyShowInsufficientInventory->setChecked(pParams.inList("onlyShowInsufficientInventory"));

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

  if(_onlyShowInsufficientInventory->isChecked())
    params.append("onlyShowInsufficientInventory");

  if(_sumParentChild->isChecked())
      params.append("summarizedParentChild");

  if(_indentedWo->isChecked())
      params.append("IndentedParentChild");
  
  return true;
}

void dspInventoryAvailabilityByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
      XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
      int menuItem;

      menuItem = pMenu->insertItem(tr("View Inventory History..."), this, SLOT(sViewHistory()), 0);
      if (!_privileges->check("ViewInventoryHistory"))
        pMenu->setItemEnabled(menuItem, FALSE);

      pMenu->insertSeparator();

      menuItem = pMenu->insertItem("View Allocations...", this, SLOT(sViewAllocations()), 0);
      if (item->rawValue("woinvav_allocated").toDouble() == 0.0)
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("View Orders...", this, SLOT(sViewOrders()), 0);
      if (item->rawValue("woinvav_ordered").toDouble() == 0.0)
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem("Running Availability...", this, SLOT(sRunningAvailability()), 0);

      pMenu->insertSeparator();
      
	  q.prepare( "SELECT itemsite_posupply as result "
				 "FROM itemsite "
				 "WHERE (itemsite_id=:womatl_id);" );
	  q.bindValue(":womatl_id", list()->id());
	  q.exec();
	  if (q.first())
	  {
		  if (q.value("result").toBool())
		  {
			menuItem = pMenu->insertItem(tr("Create P/R..."), this, SLOT(sCreatePR()), 0);
			if (!_privileges->check("MaintainPurchaseRequests"))
			  pMenu->setItemEnabled(menuItem, FALSE);

			menuItem = pMenu->insertItem("Create P/O...", this, SLOT(sCreatePO()), 0);
			if (!_privileges->check("MaintainPurchaseOrders"))
			  pMenu->setItemEnabled(menuItem, FALSE);

			pMenu->insertSeparator();
		  }
	  }
	  q.prepare( "SELECT itemsite_wosupply as result "
				 "FROM itemsite "
				 "WHERE (itemsite_id=:womatl_id);" );
	  q.bindValue(":womatl_id", list()->id());
	  q.exec();
	  if (q.first())
	  {
		  if (q.value("result").toBool())
		  {
			if(list()->altId() != -1)
			{
			  menuItem = pMenu->insertItem("Create W/O...", this, SLOT(sCreateWO()), 0);
			  if (!_privileges->check("MaintainWorkOrders"))
				pMenu->setItemEnabled(menuItem, FALSE);
			}
			menuItem = pMenu->insertItem(tr("Post Misc. Production..."), this, SLOT(sPostMiscProduction()), 0);
			if (!_privileges->check("PostMiscProduction"))
			  pMenu->setItemEnabled(menuItem, FALSE);

			pMenu->insertSeparator();
		  }
	  }
//      }

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
  params.append("itemsite_id", list()->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewAllocations()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
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
             "WHERE (womatl_itemsite_id=:womatl_id);" );
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
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
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewSubstituteAvailability()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
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
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sCreatePO()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sCreateWO()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sPostMiscProduction()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("itemsite_id", list()->id());

  postMiscProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sEnterMiscCount()
{  
  int currentAltId = 0;
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sFillList()
{
  display::sFillList();
  if(_indentedWo->isChecked())
    list()->expandAll();
}
