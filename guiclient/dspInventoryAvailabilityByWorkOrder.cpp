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

#include <metasql.h>
#include "mqlutil.h"

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
  connect(_showAll, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_onlyShowShortages, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_onlyShowInsufficientInventory, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_parentOnly, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sumParentChild, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_indentedWo, SIGNAL(clicked()), this, SLOT(sFillList()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("WO/Item#"),    120, Qt::AlignLeft,  true, "woinvav_item_wo_number");
  _womatl->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "woinvav_descrip");
  _womatl->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "woinvav_uomname");
  _womatl->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "woinvav_qoh");
  _womatl->addColumn(tr("This Alloc."), _qtyColumn, Qt::AlignRight, true, "woinvav_balance");
  _womatl->addColumn(tr("Total Alloc."),_qtyColumn, Qt::AlignRight, true, "woinvav_allocated");
  _womatl->addColumn(tr("Orders"),      _qtyColumn, Qt::AlignRight, true, "woinvav_ordered");
  _womatl->addColumn(tr("This Avail."), _qtyColumn, Qt::AlignRight, true, "woinvav_woavail");
  _womatl->addColumn(tr("Total Avail."),_qtyColumn, Qt::AlignRight, true, "woinvav_totalavail");
  _womatl->addColumn(tr("Type"),                 0, Qt::AlignLeft, false, "woinvav_type");

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
	  q.bindValue(":womatl_id", _womatl->id());
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
	  q.bindValue(":womatl_id", _womatl->id());
	  q.exec();
	  if (q.first())
	  {
		  if (q.value("result").toBool())
		  {
			if(_womatl->altId() != -1)
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
  params.append("itemsite_id", _womatl->id());

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewAllocations()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", _womatl->id());
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
             "WHERE (womatl_itemsite_id=:womatl_id);" );
  q.bindValue(":womatl_itemsite_id", _womatl->id());
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
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", _womatl->id());
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
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", _womatl->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = _womatl->id();
  sFillList();
  _womatl->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sCreatePO()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", _womatl->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = _womatl->id();
  sFillList();
  _womatl->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sCreateWO()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _womatl->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", _womatl->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = _womatl->id();
  sFillList();
  _womatl->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sPostMiscProduction()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("itemsite_id", _womatl->id());

  postMiscProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", _womatl->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = _womatl->id();
  sFillList();
  _womatl->setId(currentId,currentAltId);
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
  int currentAltId = 0;
  ParameterList params;
  params.append("itemsite_id", _womatl->id());
  
  enterMiscCount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", _womatl->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = _womatl->id();
  sFillList();
  _womatl->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("inventoryAvailabilitybyWorkorder", "detail");
  q = mql.toQuery(params);
  _womatl->populate(q, true);
  if(_indentedWo->isChecked())
    _womatl->expandAll();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
