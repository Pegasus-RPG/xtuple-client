/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentWorkbench.h"

#include <QSqlError>

#include <openreports.h>
#include <metasql.h>

#include "guiclient.h"
#include "incident.h"

incidentWorkbench::incidentWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_query,	SIGNAL(clicked()),	this,	SLOT(sFillList()));
  connect(_reset,	SIGNAL(clicked()),	this,	SLOT(sReset()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  _incdt->addColumn(tr("Number"),      _itemColumn, Qt::AlignLeft, true, "incdt_number" );
  _incdt->addColumn(tr("Account"),     _itemColumn, Qt::AlignLeft, true, "crmacct_name" );
  _incdt->addColumn(tr("Status"),      _itemColumn, Qt::AlignLeft, true, "incdt_status" );
  _incdt->addColumn(tr("Assigned To"), _userColumn, Qt::AlignLeft, true, "incdt_assigned_username" ); 
  _incdt->addColumn(tr("Summary"),     -1,          Qt::AlignLeft, true, "incdt_summary" );

  _createdDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _createdDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _createdDates->setStartCaption(tr("First Creation Date:"));
  _createdDates->setEndCaption(tr("Last Creation Date:"));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _statusFeedback->setChecked(true);
    _statusConfirmed->setChecked(true);
    _statusNew->setChecked(true);
    _statusAssigned->setChecked(true);
  }

  _assignedTo->setType(ParameterGroup::User);
}

incidentWorkbench::~incidentWorkbench()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentWorkbench::languageChange()
{
    retranslateUi(this);
}

void incidentWorkbench::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void incidentWorkbench::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _incdt->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void incidentWorkbench::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _incdt->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incidentWorkbench::sReset()
{
  _statusNew->setChecked(true);
  _statusFeedback->setChecked(true);
  _statusConfirmed->setChecked(true);
  _statusAssigned->setChecked(true);
  _statusResolved->setChecked(false);
  _statusClosed->setChecked(false);
  _createdDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _createdDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _textPattern->clear();
}

void incidentWorkbench::setParams(ParameterList & params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  _assignedTo->appendValue(params);

  if(_statusNew->isChecked())
    params.append("isnew");
  if(_statusFeedback->isChecked())
    params.append("isfeedback");
  if(_statusConfirmed->isChecked())
    params.append("isconfirmed");
  if(_statusAssigned->isChecked())
    params.append("isassigned");
  if(_statusResolved->isChecked())
    params.append("isresolved");
  if(_statusClosed->isChecked())
    params.append("isclosed");

  if(!_textPattern->text().trimmed().isEmpty())
    params.append("pattern", _textPattern->text().trimmed());

  _createdDates->appendValue(params);
}

void incidentWorkbench::sFillList()
{
  QString sql="SELECT incdt_id,"
              "       incdt_number,"
	      "       crmacct_name,"
              "       CASE WHEN(incdt_status='N') THEN <? value(\"new\") ?>"
              "            WHEN(incdt_status='F') THEN <? value(\"feedback\") ?>"
              "            WHEN(incdt_status='C') THEN <? value(\"confirmed\") ?>"
              "            WHEN(incdt_status='A') THEN <? value(\"assigned\") ?>"
              "            WHEN(incdt_status='R') THEN <? value(\"resolved\") ?>"
              "            WHEN(incdt_status='L') THEN <? value(\"closed\") ?>"
              "            ELSE incdt_status"
              "       END,"
              "       incdt_assigned_username,"
              "       incdt_summary "
              "  FROM incdt, crmacct"
              " WHERE ((incdt_crmacct_id=crmacct_id)"
	      " <? if exists(\"username\") ?> "
	      " AND (incdt_assigned_username = <? value(\"username\") ?>) "
	      " <? elseif exists(\"usr_pattern\") ?> "
	      " AND (incdt_assigned_username ~* <? value(\"usr_pattern\") ?>) "
	      " <? endif ?>"
	      " AND (incdt_status IN ('' "
	      "   <? if exists(\"isnew\") ?>,		'N' <? endif ?> "
	      "   <? if exists(\"isfeedback\") ?>,	'F' <? endif ?> "
	      "   <? if exists(\"isconfirmed\") ?>,	'C' <? endif ?> "
	      "   <? if exists(\"isassigned\") ?>,	'A' <? endif ?> "
	      "   <? if exists(\"isresolved\") ?>,	'R' <? endif ?> "
	      "   <? if exists(\"isclosed\") ?>,	'L' <? endif ?> "
	      " ))"
	      " <? if exists(\"pattern\") ?> "
	      " AND ((incdt_summary ~* <? value(\"pattern\") ?>)"
	      "  OR  (incdt_descrip ~* <? value(\"pattern\") ?>)"
	      "  OR  (incdt_id IN (SELECT comment_source_id"
	      "             FROM comment"
	      "            WHERE((comment_source='INCDT')"
              "              AND (comment_text ~* <? value(\"pattern\") ?>)))))"
	      " <? endif ?>"
	      " AND (incdt_timestamp BETWEEN <? value(\"startDate\") ?> "
	      "				 AND <? value(\"endDate\") ?>) "
	      ") ORDER BY incdt_number; ";

  ParameterList params;
  setParams(params);

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  _incdt->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incidentWorkbench::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("IncidentWorkbenchList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void incidentWorkbench::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

