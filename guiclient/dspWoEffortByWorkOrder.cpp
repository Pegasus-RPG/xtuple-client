/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoEffortByWorkOrder.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "math.h"
#include "wotc.h"
#include "wocluster.h"

dspWoEffortByWorkOrder::dspWoEffortByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wotc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_wo, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));

  //omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wotc->addColumn(tr("User"),        _userColumn, Qt::AlignLeft, true, "wotc_username");
  _wotc->addColumn(tr("Operation"),            -1, Qt::AlignLeft, true, "wooper");
  _wotc->addColumn(tr("Time In"), _timeDateColumn, Qt::AlignLeft, true, "wotc_timein");
  _wotc->addColumn(tr("Time Out"),_timeDateColumn, Qt::AlignLeft, true, "wotc_timeout");
  _wotc->addColumn(tr("Setup Time"),  _timeColumn, Qt::AlignLeft, true, "setup_time");
  _wotc->addColumn(tr("Run Time"),    _timeColumn, Qt::AlignLeft, true, "run_time");
  _wotc->addColumn(tr("Effort"),      _timeColumn, Qt::AlignLeft, true, "wo_effort");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int,bool)),  this,SLOT(sFillList()));
}

dspWoEffortByWorkOrder::~dspWoEffortByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspWoEffortByWorkOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoEffortByWorkOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspWoEffortByWorkOrder::sPrint()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  orReport report("WOEffortByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoEffortByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  if (_wotc->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("New"), this, SLOT(sNew()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWoTimeClock"));
    
    menuItem = pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWoTimeClock"));

    menuItem = pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainWoTimeClock"));
  }
}


void dspWoEffortByWorkOrder::sNew()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("mode",  "new");

  wotc *newdlg = new wotc(this, "", true);
  newdlg->set(params);
  newdlg->exec();

  sFillList();
}

void dspWoEffortByWorkOrder::sEdit()
{
  ParameterList params;
  params.append("wotc_id", _wotc->id());
  params.append("mode",    "edit");

  wotc *newdlg = new wotc(this, "", true);
  newdlg->set(params);
  newdlg->exec();

  sFillList();
}

void dspWoEffortByWorkOrder::sView()
{
  ParameterList params;
  params.append("wotc_id", _wotc->id());
  params.append("mode",    "view");
  
  wotc *newdlg = new wotc(this, "", true);
  newdlg->set(params);
  newdlg->exec();

  sFillList();
}

void dspWoEffortByWorkOrder::sDelete()
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

    sFillList();
  }
}

void dspWoEffortByWorkOrder::sFillList()
{
  _wotc->clear();

  if (_wo->isValid())
  {
    q.prepare( "SELECT wotc_id, wotc_username,"
	       "       wooper_seqnumber || ' - ' || wooper_descrip1 || ' - ' ||"
	       "                                    wooper_descrip2 AS wooper, "
	       "       wotc_timein,"
	       "       wotc_timeout,"
	       "       NULL AS setup_time, NULL AS run_time,"
	       "       wotcTime(wotc_id) AS wo_effort "
	       "  FROM wotc LEFT OUTER JOIN "
	       "     wooper ON (wotc_wooper_id=wooper_id) "
	       "WHERE ((wotc_wo_id=:wo_id)"
	       "  AND  (wotc_id NOT IN (SELECT DISTINCT wooperpost_wotc_id "
	       "                        FROM wooperpost"
	       "                        WHERE (wooperpost_wo_id=:wo_id)"
	       "                          AND (wooperpost_wotc_id IS NOT NULL)))) "
	       "UNION "
	       "SELECT wotc_id, wotc_username,"
	       "       CAST(wooperpost_seqnumber AS TEXT) AS wooper, "
	       "       wotc_timein,"
	       "       wotc_timeout,"
	       "       wooperpost_sutime AS setup_time,"
	       "       wooperpost_rntime AS run_time,"
	       "       wotcTime(wotc_id) AS wo_effort "
	       "  FROM wotc, wooperpost "
	       "WHERE ((wotc_wo_id=:wo_id)"
	       "  AND  (wooperpost_wotc_id=wotc_id)) "
	       "ORDER BY wotc_timein, wotc_timeout;" );
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    _wotc->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }

    // we have to piece together the TOTAL line because one select won't work
    XTreeWidgetItem* lastLine = new XTreeWidgetItem(_wotc,
		       _wotc->topLevelItem(_wotc->topLevelItemCount() - 1),
		       -1, "", tr("Total"));

    q.prepare("SELECT MIN(wotc_timein) AS timein,"
	      "       MAX(wotc_timeout) AS timeout,"
	      "       woTimeByWo(wotc_wo_id) AS wo_effort "
	      "FROM wotc "
	      "WHERE (wotc_wo_id=:wo_id) "
	      "GROUP BY wotc_wo_id;");
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
    {
      lastLine->setText(2, q.value("timein"));
      lastLine->setText(3, q.value("timeout"));
      lastLine->setText(6, q.value("wo_effort"));
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    q.prepare("SELECT SUM(wooperpost_sutime) AS setup_time,"
	      "       SUM(wooperpost_rntime) AS run_time,"
	      "       SUM(wooperpost_sutime) + SUM(wooperpost_rntime) -"
	      "           intervalToMinutes(woTimeByWo(wooperpost_wo_id)) AS variance "
	      "FROM wooperpost "
	      "WHERE (wooperpost_wo_id=:wo_id) "
	      "GROUP BY wooperpost_wo_id;");
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
    {
      lastLine->setText(4, q.value("setup_time"));
      lastLine->setText(5, q.value("run_time"));
      if (fabs(q.value("variance").toDouble()) > 1.5)	// rounding errors that appear <= 1 min
	lastLine->setTextColor("red");
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  }
}

void dspWoEffortByWorkOrder::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
