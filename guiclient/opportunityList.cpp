/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityList.h"

#include "xdialog.h"
#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>
#include <openreports.h>

#include "opportunity.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"

opportunityList::opportunityList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  
  _crmAccount->hide();

  _targetDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _targetDates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
  _targetDates->setStartCaption(tr("First Target Date:"));
  _targetDates->setEndCaption(tr("Last Target Date:"));

  _usr->setEnabled(_privileges->check("MaintainOtherTodoLists"));
  _usr->setType(ParameterGroup::User);
  q.prepare("SELECT usr_id "
	    "FROM usr "
	    "WHERE (usr_username=CURRENT_USER);");
  q.exec();
  if (q.first())
  {
    _myUsrId = q.value("usr_id").toInt();
    _usr->setId(_myUsrId);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    close();
  }

  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
  connect(_targetDates,	SIGNAL(updated()),	this,   SLOT(sFillList()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_list,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_list,	SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this,	SLOT(sPopulateMenu(QMenu*)));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));

  if(_privileges->check("MaintainOpportunities"))
  {
    connect(_list, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_list, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

//  statusBar()->hide();

  _list->addColumn(tr("Name"),        -1,              Qt::AlignLeft,   true, "ophead_name"  );
  _list->addColumn(tr("CRM Acct."),   _userColumn,     Qt::AlignLeft,   true, "crmacct_number" );
  _list->addColumn(tr("Owner"),       _userColumn,     Qt::AlignLeft,   true, "ophead_owner_username" );
  _list->addColumn(tr("Stage"),       _orderColumn,    Qt::AlignLeft,   true, "opstage_name" );
  _list->addColumn(tr("Source"),      _orderColumn,    Qt::AlignLeft,   false, "opsource_name" );
  _list->addColumn(tr("Type"),        _orderColumn,    Qt::AlignLeft,   false, "optype_name" );
  _list->addColumn(tr("Prob.%"),      _prcntColumn,    Qt::AlignCenter, false, "ophead_probability_prcnt" );
  _list->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  false, "ophead_amount" );
  _list->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   false, "f_currency" );
  _list->addColumn(tr("Target Date"), _dateColumn,     Qt::AlignLeft,   false, "ophead_target_date" );
  _list->addColumn(tr("Actual Date"), _dateColumn,     Qt::AlignLeft,   false, "ophead_actual_date" );
}

void opportunityList::languageChange()
{
  retranslateUi(this);
}

void opportunityList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  bool editPriv = _privileges->check("MaintainOpportunities");
  bool viewPriv = _privileges->check("VeiwOpportunities") || editPriv;

  menuItem = pMenu->insertItem(tr("New..."), this, SLOT(sNew()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, viewPriv);

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  pMenu->setItemEnabled(menuItem, editPriv);
}

enum SetResponse opportunityList::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _usr->setId(param.toInt());
    sFillList();
  }
  
  param = pParams.value("run", &valid);
  if (valid)
  {
    connect(_usr, SIGNAL(updated()),	this,	SLOT(sFillList()));
    sFillList();
  }

  return NoError;
}

void opportunityList::sClose()
{
  close();
}

void opportunityList::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_usr->isSelected())
    params.append("usr_id", _usr->id());
  if (_crmAccount->isValid())
    params.append("crmacct_id",_crmAccount->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ophead_id", _list->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ophead_id", _list->id());

  opportunity newdlg(this, "", TRUE);
  newdlg.set(params);

  newdlg.exec();
}

void opportunityList::sDelete()
{
  q.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");
  q.bindValue(":ophead_id", _list->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteOpportunity", result));
      return;
    }
    else
      sFillList();
    }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}

void opportunityList::setParams(ParameterList &params)
{
  if (_crmAccount->isValid())
    params.append("crmAccountId",_crmAccount->id());
  _usr->appendValue(params);
  _targetDates->appendValue(params);
}

void opportunityList::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("OpportunityList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void opportunityList::sFillList()
{

  MetaSQLQuery mql = mqlLoad("opportunities", "detail");

  ParameterList params;
  setParams(params);

  XSqlQuery itemQ = mql.toQuery(params);

  if (itemQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _list->populate(itemQ);
}

