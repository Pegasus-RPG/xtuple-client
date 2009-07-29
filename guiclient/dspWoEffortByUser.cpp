/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoEffortByUser.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include "implodeWo.h"
#include "explodeWo.h"
#include "closeWo.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "workOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspWoOperationsByWorkOrder.h"
#include "dspWoEffortByUser.h"
#include "wotc.h"

/*
 *  Constructs a dspWoEffortByUser as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoEffortByUser::dspWoEffortByUser(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wotc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_user, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  _dates->setStartCaption(tr("Start W/O Start Date:"));
  _dates->setEndCaption(tr("End W/O Start Date:"));
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setStartDate(QDate().currentDate());
  _dates->setEndDate(QDate().currentDate());

  _wotc->addColumn(tr("W/O #"),       _orderColumn, Qt::AlignLeft, true, "wonumber");
  _wotc->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter,true, "wo_status");
  _wotc->addColumn(tr("Pri."),       _statusColumn, Qt::AlignCenter,true, "wo_priority");
  _wotc->addColumn(tr("Site"),          _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _wotc->addColumn(tr("Operation"),             -1, Qt::AlignLeft,  true, "wooper");
  _wotc->addColumn(tr("Clock In"), _timeDateColumn, Qt::AlignLeft,  true, "wotc_timein");
  _wotc->addColumn(tr("Clock Out"),_timeDateColumn, Qt::AlignLeft,  true, "wotc_timeout");
  _wotc->addColumn(tr("Effort"),       _timeColumn, Qt::AlignLeft,  true, "wotcTime");
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _user->setFocus();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoEffortByUser::~dspWoEffortByUser()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoEffortByUser::languageChange()
{
  retranslateUi(this);
}

void dspWoEffortByUser::sPrint()
{
  ParameterList params;
  params.append("username", _user->username());
  params.append("includeFormatted", true);
  _dates->appendValue(params);

  orReport report("WOEffortByUser", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

int dspWoEffortByUser::getWoId()
{
  q.prepare("SELECT wotc_wo_id "
	    "FROM wotc "
	    "WHERE (wotc_id=:wotc_id);");
  q.bindValue(":wotc_id", _wotc->id());
  q.exec();
  if (q.first())
    return q.value("wotc_wo_id").isNull() ? -1 : q.value("wotc_wo_id").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  return -1;
}

void dspWoEffortByUser::sViewWO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", getWoId());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoEffortByUser::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", getWoId());

  closeWo newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoEffortByUser::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", getWoId());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoEffortByUser::sViewWooper()
{
  ParameterList params;
  params.append("wo_id", getWoId());
  params.append("run");

  dspWoOperationsByWorkOrder *newdlg = new dspWoOperationsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoEffortByUser::sNew()
{
  ParameterList params;
  params.append("wo_id", _wotc->altId());
  params.append("usr_id", _user->id());
  params.append("mode",  "new");

  wotc *newdlg = new wotc(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() == XDialog::Accepted)
    sFillList();
}

void dspWoEffortByUser::sEdit()
{
  ParameterList params;
  params.append("wotc_id", _wotc->id());
  params.append("mode",    "edit");

  wotc *newdlg = new wotc(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() == XDialog::Accepted)
    sFillList();
}

void dspWoEffortByUser::sView()
{
  ParameterList params;
  params.append("wotc_id", _wotc->id());
  params.append("mode",    "view");
  
  wotc *newdlg = new wotc(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void dspWoEffortByUser::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Entry?"),
			    tr("Are you sure that you want to delete the "
			       "selected time clock entry?"),
			    tr("&Yes"), tr("&No"), 0, 0, 1) == 0)
  {
    q.prepare("DELETE FROM wotc "
	      "WHERE (wotc_id=:wotc_id);");
    q.bindValue(":wotc_id", _wotc->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    else
      sFillList();
  }
}

void dspWoEffortByUser::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  QString status(selected->text(1));
  int     menuItem;

  if (_wotc->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("New"), this, SLOT(sNew()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWoTimeClock"));
    
    menuItem = pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWoTimeClock"));

    menuItem = pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWoTimeClock"));

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("View W/O"), this, SLOT(sViewWO()), 0);

    if ((status == "R") || (status == "I"))
    {
      menuItem = pMenu->insertItem(tr("Close W/O..."), this, SLOT(sCloseWO()), 0);
      if (!_privileges->check("CloseWorkOrders"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }


    if ((status == "E") || (status == "R") || (status == "I"))
    {
      pMenu->insertSeparator();

      menuItem = pMenu->insertItem(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatl()), 0);
      if (!_privileges->check("ViewWoMaterials"))
        pMenu->setItemEnabled(menuItem, FALSE);

      menuItem = pMenu->insertItem(tr("View W/O Operations..."), this, SLOT(sViewWooper()), 0);
      if (!_privileges->check("ViewWoOperations"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }
  }
}

void dspWoEffortByUser::sFillList()
{
  _wotc->clear();

  if ((_user->isValid()) && (_dates->allValid()))
  {
    MetaSQLQuery mql = mqlLoad("woEffortByUser", "detail");
    ParameterList params;
    _dates->appendValue(params);
    params.append("username", _user->username());
    q = mql.toQuery(params);
    _wotc->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    } 
  }
}

void dspWoEffortByUser::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
