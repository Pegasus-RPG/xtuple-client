/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
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
#include "mqlutil.h"

incidentWorkbench::incidentWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  parameterWidget->setType("Owner", "owner_username", ParameterWidget::User);
  parameterWidget->setType("Assigned User", "assigned_username", ParameterWidget::User);
  parameterWidget->setType("Assigned Pattern", "assigned_usr_pattern", ParameterWidget::Text);
  parameterWidget->setType("Owner Pattern", "owner_usr_pattern", ParameterWidget::Text);
  parameterWidget->setType("Pattern", "pattern", ParameterWidget::Text);
  parameterWidget->setType("Start Date", "startDate", ParameterWidget::Date);
  parameterWidget->setType("End Date", "endDate", ParameterWidget::Date);
	parameterWidget->setType("CRM Account", "crmAccountId", ParameterWidget::Crmacct);
	parameterWidget->setXComboBoxType("Severity", "severity_id", XComboBox::IncidentSeverity);
	parameterWidget->setXComboBoxType("Category", "category_id", XComboBox::IncidentCategory);


  connect(parameterWidget, SIGNAL(updated()), this, SLOT(sFillList()));
  parameterWidget->applyDefaultFilterSet();

  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_query,	SIGNAL(clicked()),	this,	SLOT(sFillList()));
  connect(_reset,	SIGNAL(clicked()),	this,	SLOT(sReset()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  _incdt->addColumn(tr("Number"),      _orderColumn,Qt::AlignLeft, true, "incdt_number" );
  _incdt->addColumn(tr("Created"),     _dateColumn, Qt::AlignLeft, true, "incdt_timestamp" );
  _incdt->addColumn(tr("Account"),     _itemColumn, Qt::AlignLeft, true, "crmacct_name" );
  _incdt->addColumn(tr("Status"),      _itemColumn, Qt::AlignLeft, true, "incdt_status" );
  _incdt->addColumn(tr("Assigned To"), _userColumn, Qt::AlignLeft, true, "incdt_assigned_username" ); 
  _incdt->addColumn(tr("Owner"),       _userColumn, Qt::AlignLeft, true, "incdt_owner_username" );
  _incdt->addColumn(tr("Summary"),     -1,          Qt::AlignLeft, true, "incdt_summary" );

  //_createdDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  //_createdDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _statusFeedback->setChecked(true);
    _statusConfirmed->setChecked(true);
    _statusNew->setChecked(true);
    _statusAssigned->setChecked(true);
  }

  //_user->setType(ParameterGroup::User);
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
  //_createdDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  //_createdDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  //_textPattern->clear();
}

void incidentWorkbench::setParams(ParameterList & params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

	params.append("startDate", omfgThis->startOfTime());
	params.append("endDate", omfgThis->endOfTime());

  //if (_assignedTo->isChecked())
  //  params.append("assignedTo");
  //else
  //  params.append("ownedBy"); 
  //_user->appendValue(params);

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

  //if(!_textPattern->text().trimmed().isEmpty())
   // params.append("pattern", _textPattern->text().trimmed());

  //_createdDates->appendValue(params);
  parameterWidget->appendValue(params);
}

void incidentWorkbench::sFillList()
{
  MetaSQLQuery mql = mqlLoad("incidents", "detail");
  ParameterList params;
  setParams(params);
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

