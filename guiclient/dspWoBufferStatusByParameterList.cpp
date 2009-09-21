/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoBufferStatusByParameterList.h"

#include <QMenu>
#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <openreports.h>
#include "postProduction.h"
#include "correctProductionPosting.h"
#include "postOperations.h"
#include "correctOperationsPosting.h"
#include "implodeWo.h"
#include "explodeWo.h"
#include "closeWo.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "workOrder.h"
#include "salesOrderInformation.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspWoOperationsByWorkOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"

/*
 *  Constructs a dspWoBufferStatusByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoBufferStatusByParameterList::dspWoBufferStatusByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _wo->addColumn(tr("parentType"),    0,             Qt::AlignCenter, true,  "wo_ordtype" );
  _wo->addColumn(tr("W/O #"),         _orderColumn,  Qt::AlignLeft,   true,  "wonumber"   );
  _wo->addColumn(tr("Status"),        _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  _wo->addColumn(tr("Pri."),          _statusColumn, Qt::AlignCenter, true,  "wo_priority" );
  _wo->addColumn(tr("Site"),          _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  _wo->addColumn(tr("Item Number"),   _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  _wo->addColumn(tr("Description"),   -1,            Qt::AlignLeft,   true,  "description"   );
  _wo->addColumn(tr("UOM"),           _uomColumn,    Qt::AlignCenter, true,  "uom_name" );
  _wo->addColumn(tr("Ordered"),       _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  _wo->addColumn(tr("Received"),      _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  _wo->addColumn(tr("Buffer Type"),   _uomColumn,    Qt::AlignCenter, true,  "bufrststype"  );
  _wo->addColumn(tr("Buffer Status"), _uomColumn,    Qt::AlignRight,  true,  "bufrsts_status"  );
  
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoBufferStatusByParameterList::~dspWoBufferStatusByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoBufferStatusByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoBufferStatusByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    setWindowTitle(tr("W/O Buffer Status by ClassCode"));
  }

  param = pParams.value("plancode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    setWindowTitle(tr("W/O Buffer Status by Planner Code"));
  }

  param = pParams.value("plancode_id", &valid);
  if (valid)
    _parameter->setId(param.toInt());

  param = pParams.value("itemgrp", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    setWindowTitle(tr("W/O Buffer Status by Item Group"));
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());
    
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspWoBufferStatusByParameterList::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _parameter->appendValue(params);

  if (_parameter->isAll())
  {
    if (_parameter->type() == ParameterGroup::ItemGroup)
      params.append("itemgrp");
    else if(_parameter->type() == ParameterGroup::PlannerCode)
      params.append("plancode");
    else if (_parameter->type() == ParameterGroup::ClassCode)
      params.append("classcode");
  }

  if (_showOnlyRI->isChecked())
    params.append("showOnlyRI");

  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

//  if(_sortByItemNumber->isChecked())
//    params.append("sortByItemNumber");

  orReport report("WOBufferStatusByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoBufferStatusByParameterList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoBufferStatusByParameterList::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctProductionPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sPostOperations()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postOperations newdlg(this, "", TRUE);
  if(newdlg.set(params) != UndefinedError)
    newdlg.exec();
}

void dspWoBufferStatusByParameterList::sCorrectOperationsPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctOperationsPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sReleaseWO()
{
  q.prepare("SELECT releaseWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoBufferStatusByParameterList::sRecallWO()
{
  q.prepare("SELECT recallWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoBufferStatusByParameterList::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  explodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  implodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sDeleteWO()
{
  q.prepare( "SELECT wo_ordtype, wo_ordid "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    bool toDelete = FALSE;

    if (q.value("wo_ordtype").toString() == "W")
    {
      if (QMessageBox::warning( this, tr("Delete Work Order"),
                                tr( "The Work Order that you selected to delete is a child\n"
                                    "of another Work Order.  If you delete the selected\n"
                                    "Work Order then the Work Order Materials Requirements for\n"
                                    "the Component Item will remain but the Work Order to\n"
                                    "relieve that demand will not.\n"
                                    "Are you sure that you want to delete the selected Work Order?" ),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        toDelete = TRUE;
    }
    else if (q.value("wo_ordtype").toString() == "S")
    {
      if (QMessageBox::warning( this, tr("Delete Work Order"),
                                tr( "The Work Order that you selected to delete was created\n"
                                    "to satisfy a Sales Order demand.  If you delete the selected\n"
                                    "Work Order then the Sales Order demand will remain but the\n"
                                    "Work Order to relieve that demand will not.\n"
                                    "Are you sure that you want to delete the selected Work Order?" ),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        toDelete = TRUE;
    }
    else
    {
      if (QMessageBox::warning( this, tr("Delete Work Order"),
                                tr( "Are you sure that you want to delete the selected Work Order?" ),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        toDelete = TRUE;
    }

    if (toDelete)
    {
      q.prepare("SELECT deleteWo(:wo_id, TRUE);");
      q.bindValue(":wo_id", _wo->id());
      q.exec();

      omfgThis->sWorkOrdersUpdated(-1, TRUE);
    }
  }
}

void dspWoBufferStatusByParameterList::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoBufferStatusByParameterList::sViewWooper()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoOperationsByWorkOrder *newdlg = new dspWoOperationsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoBufferStatusByParameterList::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoBufferStatusByParameterList::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  reprioritizeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  rescheduleWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sViewParentSO()
{
  ParameterList params;
  params.append("soitem_id", _wo->altId());

  salesOrderInformation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoBufferStatusByParameterList::sViewParentWO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->altId());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoBufferStatusByParameterList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  QString status(selected->text(2));
  int     menuItem;

  menuItem = pMenu->insertItem(tr("View W/O"), this, SLOT(sView()), 0);

  pMenu->insertSeparator();

  if (status == "E")
  {
    menuItem = pMenu->insertItem(tr("Release W/O"), this, SLOT(sReleaseWO()), 0);
    if (!_privileges->check("ReleaseWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (status == "R")
  {
    menuItem = pMenu->insertItem(tr("Recall W/O"), this, SLOT(sRecallWO()), 0);
    if (!_privileges->check("RecallWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->insertItem(tr("Post Production..."), this, SLOT(sPostProduction()), 0);
    if (!_privileges->check("PostProduction"))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (status != "E")
    {
      menuItem = pMenu->insertItem(tr("Correct Production Posting..."), this, SLOT(sCorrectProductionPosting()), 0);
      if (!_privileges->check("PostProduction"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    menuItem = pMenu->insertItem(tr("Post Operations..."), this, SLOT(sPostOperations()), 0);
    if (!_privileges->check("PostWoOperations"))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (status != "E")
    {
      menuItem = pMenu->insertItem(tr("Correct Operations Posting..."), this, SLOT(sCorrectOperationsPosting()), 0);
      if (!_privileges->check("PostWoOperations"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    pMenu->insertSeparator();
  }

  if (status == "O")
  {
    menuItem = pMenu->insertItem(tr("Explode W/O..."), this, SLOT(sExplodeWO()), 0);
    if (!_privileges->check("ExplodeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else if (status == "E")
  {
    menuItem = pMenu->insertItem(tr("Implode W/O..."), this, SLOT(sImplodeWO()), 0);
    if (!_privileges->check("ImplodeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((status == "O") || (status == "E"))
  {
    pMenu->insertItem(tr("Delete W/O..."), this, SLOT(sDeleteWO()), 0);
    if (!_privileges->check("DeleteWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Close W/O..."), this, SLOT(sCloseWO()), 0);
    if (!_privileges->check("CloseWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  pMenu->insertSeparator();

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->insertItem(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatl()), 0);
    if (!_privileges->check("ViewWoMaterials"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("View W/O Operations..."), this, SLOT(sViewWooper()), 0);
    if (!_privileges->check("ViewWoOperations"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()), 0);
    if (!_privileges->check("ViewInventoryAvailability"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Traveler..."), this, SLOT(sPrintTraveler()), 0);
    if (!_privileges->check("PrintWorkOrderPaperWork"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if ((status == "O") || (status == "E"))
  {
    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()), 0);
    if (!_privileges->check("ReprioritizeWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()), 0);
    if (!_privileges->check("RescheduleWorkOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()), 0);
    if (!_privileges->check("ChangeWorkOrderQty"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  if (_wo->altId() != -1)
  {
    if (selected->text(0) == "S")
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("View Parent Sales Order Information..."), this, SLOT(sViewParentSO()), 0);
    }
    else if (selected->text(0) == "W")
    {
      pMenu->insertSeparator();
      menuItem = pMenu->insertItem(tr("View Parent Work Order Information..."), this, SLOT(sViewParentWO()), 0);
    }
  }
}

void dspWoBufferStatusByParameterList::sFillList()
{
  _wo->clear();

  QString sql( "SELECT wo_id, COALESCE(wo_ordid, -1) AS orderid, wo_ordtype,"
               "       formatWONumber(wo_id) as wonumber,"
               "       wo_status, wo_priority, warehous_code,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       uom_name,"
               "       wo_qtyord, wo_qtyrcv,"
               "       CASE WHEN (bufrsts_type='T') THEN :time"
               "            ELSE :stock"
               "       END AS bufrststype,"
               "       bufrsts_status,"
               "       CASE WHEN (bufrsts_status>=66) THEN 'error' END AS bufrsts_status_qtforegroundrole,"
               "       'qty' AS wo_qtyord_xtnumericrole,"
               "       'qty' AS wo_qtyrcv_xtnumericrole "
               "  FROM wo, itemsite, warehous, item, uom, bufrsts "
               " WHERE ( (wo_itemsite_id=itemsite_id)"
               "   AND   (itemsite_item_id=item_id)"
               "   AND   (item_inv_uom_id=uom_id)"
               "   AND   (itemsite_warehous_id=warehous_id)"
	       "   AND   (bufrsts_target_type='W')"
	       "   AND   (bufrsts_target_id=wo_id)"
	       "   AND   (bufrsts_date=current_date)");

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_parameter->isSelected())
  {
    if (_parameter->type() == ParameterGroup::ClassCode)
      sql += " AND (item_classcode_id=:classcode_id)";
    else if (_parameter->type() == ParameterGroup::ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem WHERE (itemgrpitem_itemgrp_id=:itemgrp_id)))";
    else if (_parameter->type() == ParameterGroup::PlannerCode)
      sql += " AND (itemsite_plancode_id=:plancode_id)";
  }
  else if (_parameter->isPattern())
  {
    if (_parameter->type() == ParameterGroup::ClassCode)
      sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    else if (_parameter->type() == ParameterGroup::ItemGroup)
      sql += " AND (item_id IN (SELECT itemgrpitem_item_id FROM itemgrpitem, itemgrp WHERE ( (itemgrpitem_itemgrp_id=itemgrp_id) AND (itemgrp_name ~ :itemgrp_pattern) ) ))";
    else if (_parameter->type() == ParameterGroup::PlannerCode)
      sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
  }
  else if (_parameter->type() == ParameterGroup::ItemGroup)
    sql += " AND (item_id IN (SELECT DISTINCT itemgrpitem_item_id FROM itemgrpitem))";

  if (_showOnlyRI->isChecked())
    sql += " AND (wo_status IN ('R','I'))";
  else
    sql += " AND (wo_status<>'C')";

  if (_showOnlyTopLevel->isChecked())
    sql += " AND (wo_ordtype<>'W')";

  sql += ") "
         " ORDER BY bufrsts_status DESC, wo_number, wo_subnumber";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _parameter->bindValue(q);
  q.bindValue(":stock", tr("Stock"));
  q.bindValue(":time", tr("Time"));
  q.exec();
  _wo->populate(q, true);
}

void dspWoBufferStatusByParameterList::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
