/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoScheduleByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "bom.h"
#include "boo.h"
#include "changeWoQty.h"
#include "closeWo.h"
#include "correctOperationsPosting.h"
#include "correctProductionPosting.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspRunningAvailability.h"
#include "dspWoEffortByWorkOrder.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspWoOperationsByWorkOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "issueWoMaterialItem.h"
#include "postOperations.h"
#include "postProduction.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "storedProcErrorLookup.h"
#include "workOrder.h"

dspWoScheduleByItem::dspWoScheduleByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cJob);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cJob);
  _dates->setStartCaption(tr("Start W/O Start Date:"));
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("End W/O Start Date:"));

  _wo->addColumn(tr("W/O #"),      -1,            Qt::AlignLeft,   true,  "wonumber"   );
  _wo->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  _wo->addColumn(tr("Pri."),       _statusColumn, Qt::AlignCenter, true,  "wo_priority" );
  _wo->addColumn(tr("Site"),       _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  _wo->addColumn(tr("Ordered"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  _wo->addColumn(tr("Received"),   _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  _wo->addColumn(tr("Start Date"), _dateColumn,   Qt::AlignCenter, true,  "wo_startdate" );
  _wo->addColumn(tr("Due Date"),   _dateColumn,   Qt::AlignCenter, true,  "wo_duedate" );
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

dspWoScheduleByItem::~dspWoScheduleByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspWoScheduleByItem::languageChange()
{
  retranslateUi(this);
}

bool dspWoScheduleByItem::setParams(ParameterList &pParams)
{
  if (! _item->isValid())
  {
    QMessageBox::warning(this, tr("Missing Data"),
			 tr("You must enter a item."));
    _item->setFocus();
    return false;
  }
  if (! _dates->allValid())
  {
    QMessageBox::warning(this, tr("Missing Data"),
			 tr("You must enter both a start and end date."));
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);
  pParams.append("item_id", _item->id());

  if (_showOnlyRI->isChecked())
    pParams.append("showOnlyRI");

  if (_showOnlyTopLevel->isChecked())
    pParams.append("showOnlyTopLevel");

  if (_sortByStartDate->isChecked())
    pParams.append("sortByStartDate");
  else
    pParams.append("sortByDueDate");

  return true;
}

void dspWoScheduleByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("WOScheduleByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoScheduleByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postProduction newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctProductionPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sPostOperations()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  postOperations newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sCorrectOperationsPosting()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  correctOperationsPosting newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sReleaseWO()
{
  q.prepare("SELECT releaseWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoScheduleByItem::sRecallWO()
{
  q.prepare("SELECT recallWo(:wo_id, FALSE);");
  q.bindValue(":wo_id", _wo->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
}

void dspWoScheduleByItem::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  explodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  implodeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sDeleteWO()
{
  q.prepare( "SELECT wo_ordtype "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    QString question;
    if (q.value("wo_ordtype") == "W")
      question = tr("The Work Order that you selected to delete is a child of "
		    "another Work Order.  If you delete the selected Work "
		    "Order then the Work Order Materials Requirements for the "
		    "Component Item will remain but the Work Order to relieve "
		    "that demand will not. Are you sure that you want to "
		    "delete the selected Work Order?" );
    else if (q.value("wo_ordtype") == "S")
      question = tr("The Work Order that you selected to delete was created to "
		    "satisfy a Sales Order demand.  If you delete the selected "
		    "Work Order then the Sales Order demand will remain but "
		    "the Work Order to relieve that demand will not. Are you "
		    "sure that you want to delete the selected Work Order?" );
    else
      question = tr("Are you sure that you want to delete the selected "
		    "Work Order?");

    if (QMessageBox::question(this, tr("Delete Work Order?"), "<p>" + question,
			      QMessageBox::No | QMessageBox::Default,
			      QMessageBox::Yes) == QMessageBox::No)
      return;

    q.prepare("SELECT deleteWo(:wo_id, TRUE) AS returnVal;");
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("returnVal").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteWo", result));
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByItem::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  printWoTraveler newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  reprioritizeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  rescheduleWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  changeWoQty newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sViewWooper()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoOperationsByWorkOrder *newdlg = new dspWoOperationsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  QString status(selected->text(1));
  int     menuItem;

  menuItem = pMenu->insertItem(tr("Edit W/O"), this, SLOT(sEdit()), 0);
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

    if (_metrics->boolean("Routings"))
    {
      menuItem = pMenu->insertItem(tr("Post Operations..."), this, SLOT(sPostOperations()), 0);
      if (!_privileges->check("PostWoOperations"))
        pMenu->setItemEnabled(menuItem, FALSE);

      if (status != "E")
      {
        menuItem = pMenu->insertItem(tr("Correct Operations Posting..."), this, SLOT(sCorrectOperationsPosting()), 0);
        if (!_privileges->check("PostWoOperations"))
          pMenu->setItemEnabled(menuItem, FALSE);
      }
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
    menuItem = pMenu->insertItem(tr("Delete W/O..."), this, SLOT(sDeleteWO()), 0);
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

    if (_metrics->boolean("Routings"))
    {
      menuItem = pMenu->insertItem(tr("View W/O Operations..."), this, SLOT(sViewWooper()), 0);
      if (!_privileges->check("ViewWoOperations"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    menuItem = pMenu->insertItem(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()), 0);
    if (!_privileges->check("ViewInventoryAvailability"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Traveler..."), this, SLOT(sPrintTraveler()), 0);
    if (!_privileges->check("PrintWorkOrderPaperWork"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Issue Material Item..."), this, SLOT(sIssueWoMaterialItem()));
    pMenu->setItemEnabled(menuItem, _privileges->check("IssueWoMaterials"));
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

  pMenu->insertSeparator();
  
  menuItem = pMenu->insertItem(tr("View Bill of Materials..."), this, SLOT(sViewBOM()), 0);
  pMenu->setItemEnabled(menuItem, _privileges->check("ViewBOMs"));
  if (_metrics->boolean("Routings"))
  {
    menuItem = pMenu->insertItem(tr("View Bill of Operations..."), this, SLOT(sViewBOO()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewBOOs"));
  }

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()), 0);

  if (_metrics->boolean("Routings"))
  {
    menuItem = pMenu->insertItem(tr("Production Time Clock by Work Order..."), this, SLOT(sDspWoEffortByWorkOrder()), 0);
    pMenu->setItemEnabled(menuItem, (_privileges->check("MaintainWoTimeClock") || _privileges->check("ViewWoTimeClock")));
  }
}

void dspWoScheduleByItem::sFillList()
{
  _wo->clear();

  QString sql( "SELECT wo_id, itemsite_id,"
               "       wo.*, warehous_code,"
               "       formatWONumber(wo_id) AS wonumber,"
               "       'qty' AS wo_qtyord_xtnumericrole,"
               "       'qty' AS wo_qtyrcv_xtnumericrole,"
               "       CASE WHEN ((wo_startdate<=CURRENT_DATE) AND (wo_status IN ('O','E','S','R'))) THEN 'error' END AS wo_startdate_qtforegroundrole,"
               "       CASE WHEN (wo_duedate<=CURRENT_DATE) THEN 'error' END AS wo_duedate_qtforegroundrole "
	       "FROM wo, itemsite, warehous "
	       "WHERE ((wo_itemsite_id=itemsite_id)"
	       " AND (itemsite_warehous_id=warehous_id)"
	       " AND (itemsite_item_id=<? value(\"item_id\") ?>)"
	       " AND (wo_startdate BETWEEN <? value(\"startDate\") ?>"
	       "                       AND <? value(\"endDate\") ?>)"
	       "<? if exists(\"showOnlyRI\") ?>"
	       " AND (wo_status IN ('R','I'))"
	       "<? else ?>"
	       " AND (wo_status<>'C')"
	       "<? endif ?>"
	       "<? if exists(\"showOnlyTopLevel\") ?>"
	       " AND (wo_ordtype<>'W')"
	       "<? endif ?>"
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       ") "
	       "ORDER BY "
	       "<? if exists(\"sortByStartDate\") ?>"
	       "	wo_startdate,"
	       "<? elseif exists(\"sortByDueDate\") ?>"
	       "	wo_duedate,"
	       "<? endif ?>"
	       " wo_number, wo_subnumber" );

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _wo->populate(q, true);
}

void dspWoScheduleByItem::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void dspWoScheduleByItem::sDspWoEffortByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("run");

  dspWoEffortByWorkOrder *newdlg = new dspWoEffortByWorkOrder();
  enum SetResponse setresp = newdlg->set(params);
  if (setresp == NoError || setresp == NoError_Run)
    omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sIssueWoMaterialItem()
{
  issueWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("wo_id", _wo->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void dspWoScheduleByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _wo->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sViewBOM()
{
  q.prepare("SELECT itemsite_item_id FROM itemsite WHERE (itemsite_id=:id);");
  q.bindValue(":id", _wo->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("item_id", q.value("itemsite_item_id"));
    params.append("mode", "view");

    BOM *newdlg = new BOM();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByItem::sViewBOO()
{
  q.prepare("SELECT itemsite_item_id FROM itemsite WHERE (itemsite_id=:id);");
  q.bindValue(":id", _wo->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("item_id", q.value("itemsite_item_id"));
    params.append("mode", "view");

    boo *newdlg = new boo();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
