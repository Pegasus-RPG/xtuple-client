/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspIncidentsByCRMAccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "crmaccount.h"
#include "incident.h"
#include "todoItem.h"

dspIncidentsByCRMAccount::dspIncidentsByCRMAccount(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _crmacctGroupInt = new QButtonGroup(this);
  _crmacctGroupInt->addButton(_allAccts);
  _crmacctGroupInt->addButton(_selectedAcct);

  _createdDate->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _createdDate->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  connect(_list,  SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
				     this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _list->addColumn(tr("Account Number"),	    80, Qt::AlignLeft,  true, "crmacct_number");
  _list->addColumn(tr("Account Name"),		   100, Qt::AlignLeft,  true, "crmacct_name");
  _list->addColumn(tr("Incident"),	  _orderColumn, Qt::AlignRight, true, "incdt_number");
  _list->addColumn(tr("Summary"),	            -1, Qt::AlignLeft,  true, "summary");
  _list->addColumn(tr("Entered/Assigned"), _dateColumn, Qt::AlignLeft,  true, "startdate");
  _list->addColumn(tr("Status"),         _statusColumn, Qt::AlignCenter,true, "status");
  _list->addColumn(tr("Assigned To"),	   _userColumn, Qt::AlignLeft,  true, "assigned");
  _list->addColumn(tr("To-Do Due"),	   _dateColumn, Qt::AlignLeft,  true, "duedate");

  _list->setIndentation(10);
}

dspIncidentsByCRMAccount::~dspIncidentsByCRMAccount()
{
}

void dspIncidentsByCRMAccount::languageChange()
{
    retranslateUi(this);
}

enum SetResponse dspIncidentsByCRMAccount::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _selectedAcct->setChecked(true);
    _crmacct->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}


void dspIncidentsByCRMAccount::sPrint()
{
  if (_selectedAcct->isChecked() && _crmacct->id() <= 0)
  {
    QMessageBox::critical(this, tr("No CRM Account"),
			  tr("Please select a CRM Account before Printing."),
			  QMessageBox::Ok, QMessageBox::NoButton);
    _crmacct->setFocus();
    return;
  }

  ParameterList params;
  setParams(params);

  orReport report("IncidentsByCRMAccount", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspIncidentsByCRMAccount::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if (_list->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Edit CRM Account..."), this, SLOT(sEditCRMAccount()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainCRMAccounts"));
    menuItem = pMenu->insertItem(tr("View CRM Account..."), this, SLOT(sViewCRMAccount()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewCRMAccounts") ||
				    _privileges->check("MaintainCRMAccounts"));
  }
  else if (_list->altId() == 2)
  {
    menuItem = pMenu->insertItem(tr("Edit Incident..."), this, SLOT(sEditIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainIncidents"));
    menuItem = pMenu->insertItem(tr("View Incident..."), this, SLOT(sViewIncident()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewIncidents") ||
				    _privileges->check("MaintainIncidents"));
  }
  else if (_list->altId() == 3)
  {
    menuItem = pMenu->insertItem(tr("Edit To-Do Item..."), this, SLOT(sEditTodoItem()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainOtherTodoLists"));

    menuItem = pMenu->insertItem(tr("View To-Do Item..."), this, SLOT(sViewTodoItem()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewOtherTodoLists"));
  }
}

void dspIncidentsByCRMAccount::sEditCRMAccount()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id", _list->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void dspIncidentsByCRMAccount::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _list->id());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspIncidentsByCRMAccount::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _list->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspIncidentsByCRMAccount::sViewCRMAccount()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id", _list->id());

  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void dspIncidentsByCRMAccount::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _list->id());

  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void dspIncidentsByCRMAccount::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _list->id());

  todoItem newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

bool dspIncidentsByCRMAccount::setParams(ParameterList &params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  if (_selectedAcct->isChecked() && _crmacct->id() <= 0)
  {
    QMessageBox::critical(this, tr("No CRM Account"),
			  tr("Please select a CRM Account before Querying or Printing."),
			  QMessageBox::Ok, QMessageBox::NoButton);
    _crmacct->setFocus();
    return false;
  }

  if (! _allAccts->isChecked())
    params.append("crmacct_id", _crmacct->id());

  if (_showClosed->isChecked())
    params.append("showClosed");

  if (_showAcctsWOIncdts->isChecked())
    params.append("showAcctsWOIncdts");

  _createdDate->appendValue(params);

  return true;
}

void dspIncidentsByCRMAccount::sFillList()
{
  QString sql = "SELECT crmacct_id, 1 AS alt_id,"
		"	crmacct_number, crmacct_name, "
		"	CAST(NULL AS INTEGER) AS incdt_number, '' AS summary, "
                "       CAST(NULL AS DATE) AS startdate,"
		"       '' AS status,"
		"       '' AS assigned, CAST(NULL AS DATE) AS duedate,"
                "       MIN(incdt_timestamp) AS incdt_timestamp,"
                "       0 AS xtindentrole,"
                "       NULL AS crmacct_number_qtdisplayrole,"
                "       NULL AS crmacct_name_qtdisplayrole,"
                "       NULL AS incdt_number_qtdisplayrole "
		"  FROM crmacct "
		"      <? if exists(\"showAcctsWOIncdts\") ?> "
		"	LEFT OUTER "
		"      <? endif ?> "
		"      JOIN incdt ON (incdt_crmacct_id=crmacct_id "
		"                     AND (incdt_timestamp BETWEEN <? value(\"startDate\") ?> "
		"                                              AND <? value(\"endDate\") ?>) "
		"                     <? if not exists(\"showClosed\") ?> "
		"                       AND incdt_status != 'L' "
		"                     <? endif ?>) "
		"<? if exists(\"crmacct_id\") ?> "
                " WHERE (crmacct_id=<? value(\"crmacct_id\") ?>) "
		"<? endif ?> "
                "GROUP BY crmacct_id, crmacct_number, crmacct_name "
                "UNION "
		"SELECT incdt_id, 2, "
		"	crmacct_number, crmacct_name, "
		"	incdt_number, incdt_summary,"
                "       DATE(incdt_timestamp) AS startdate, "
		"       CASE WHEN(incdt_status='N') THEN <? value(\"new\") ?>"
		"            WHEN(incdt_status='F') THEN <? value(\"feedback\") ?>"
		"            WHEN(incdt_status='C') THEN <? value(\"confirmed\") ?>"
		"            WHEN(incdt_status='A') THEN <? value(\"assigned\") ?>"
		"            WHEN(incdt_status='R') THEN <? value(\"resolved\") ?>"
		"            WHEN(incdt_status='L') THEN <? value(\"closed\") ?>"
		"            ELSE incdt_status"
		"       END AS status,"
		"       incdt_assigned_username AS assigned, NULL,"
                "       incdt_timestamp,"
                "       1 AS xtindentrole,"
                "       '' AS crmacct_number_qtdisplayrole,"
                "       '' AS crmacct_name_qtdisplayrole,"
                "       NULL AS incdt_number_qtdisplayrole "
		"  FROM crmacct, incdt "
		"WHERE ((incdt_crmacct_id=crmacct_id)"
		"   AND (incdt_timestamp BETWEEN <? value(\"startDate\") ?> "
		"                            AND <? value(\"endDate\") ?>) "
		"<? if not exists(\"showClosed\") ?> "
		"   AND (incdt_status != 'L')"
		"<? endif ?> "
		" <? if exists(\"crmacct_id\") ?> "
                "   AND (crmacct_id=<? value(\"crmacct_id\") ?>) "
                "<? endif ?>"
                ") "
                "UNION "
		"SELECT todoitem_id, 3,"
		"	crmacct_number, crmacct_name, "
		"	incdt_number, todoitem_name,"
		"	todoitem_assigned_date,"
                "       todoitem_status, "
		"	todoitem_username, "
                "	todoitem_due_date,"
                "       incdt_timestamp,"
                "       2 AS xtindentrole,"
                "       '' AS crmacct_number_qtdisplayrole,"
                "       '' AS crmacct_name_qtdisplayrole,"
                "       '' AS incdt_number_qtdisplayrole "
		"  FROM crmacct "
	        "      JOIN incdt ON (incdt_crmacct_id=crmacct_id "
		"                AND (incdt_timestamp BETWEEN <? value(\"startDate\") ?> "
		"                                         AND <? value(\"endDate\") ?>) "
		"                     <? if not exists(\"showClosed\") ?> "
		"                       AND incdt_status != 'L' "
		"                     <? endif ?>) "
		"      JOIN todoitem ON (todoitem_incdt_id=incdt_id) "
		"  WHERE ((todoitem_status IS NULL OR todoitem_status != 'C') "
		"    <? if exists(\"crmacct_id\") ?> "
		"      AND (crmacct_id=<? value(\"crmacct_id\") ?>) "
		"    <? endif ?> "
		"    )  "
		"ORDER BY crmacct_name, incdt_timestamp, xtindentrole, duedate; "
		;
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _list->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__,__LINE__);
    return;
  }
}
