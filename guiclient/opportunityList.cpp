/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityList.h"

#include "xdialog.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QToolBar>
#include <QVariant>

#include "guiclient.h"
#include "opportunity.h"
#include "parameterwidget.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

opportunityList::opportunityList(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "opportunityList", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Opportunities"));
  setReportName("OpportunityList");
  setMetaSQLOptions("opportunities", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  list()->addColumn(tr("Number"),      _orderColumn,    Qt::AlignLeft,   false, "ophead_number" );
  list()->addColumn(tr("Active"),      _orderColumn,    Qt::AlignLeft,   false, "ophead_active" );
  list()->addColumn(tr("Name"),        -1,              Qt::AlignLeft,   true, "ophead_name"  );
  list()->addColumn(tr("CRM Acct."),   _userColumn,     Qt::AlignLeft,   true, "crmacct_number" );
  list()->addColumn(tr("Owner"),       _userColumn,     Qt::AlignLeft,   true, "ophead_owner_username" );
  list()->addColumn(tr("Assigned"),    _userColumn,     Qt::AlignLeft,   false, "ophead_username" );
  list()->addColumn(tr("Stage"),       _orderColumn,    Qt::AlignLeft,   true, "opstage_name" );
  list()->addColumn(tr("Priority"),    _orderColumn,    Qt::AlignLeft,   true, "incdtpriority_name" );
  list()->addColumn(tr("Source"),      _orderColumn,    Qt::AlignLeft,   true, "opsource_name" );
  list()->addColumn(tr("Type"),        _orderColumn,    Qt::AlignLeft,   true, "optype_name" );
  list()->addColumn(tr("Prob.%"),      _prcntColumn,    Qt::AlignCenter, true, "ophead_probability_prcnt" );
  list()->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  true, "ophead_amount" );
  list()->addColumn(tr("Weighted Value"),      _moneyColumn,    Qt::AlignRight,  true, "value" );
  list()->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   false, "f_currency" );
  list()->addColumn(tr("Target Date"), _dateColumn,     Qt::AlignLeft,   true, "ophead_target_date" );
  list()->addColumn(tr("Actual Date"), _dateColumn,     Qt::AlignLeft,   false, "ophead_actual_date" );
  list()->addColumn(tr("Create Date"), _dateColumn,     Qt::AlignLeft,   false, "ophead_created" );

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sOpen()));

  if (!_privileges->check("MaintainAllOpportunities") && !_privileges->check("MaintainPersonalOpportunities"))
    newAction()->setEnabled(false);

  parameterWidget()->append(tr("User"), "username", ParameterWidget::User, omfgThis->username());
  parameterWidget()->append(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget()->append(tr("Assigned To"), "assigned_username", ParameterWidget::User);
  parameterWidget()->append(tr("Target Date on or After"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Target Date on or Before"),   "endDate",   ParameterWidget::Date);
  parameterWidget()->append(tr("Account"), "crmacct_id",  ParameterWidget::Crmacct);
  parameterWidget()->appendComboBox(tr("Type"), "optype_id", XComboBox::OpportunityTypes);
  parameterWidget()->append(tr("Type Pattern"), "optype_pattern",    ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Source"), "opsource_id", XComboBox::OpportunitySources);
  parameterWidget()->append(tr("Source Pattern"), "opsource_pattern",    ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Stage"), "opstage_id", XComboBox::OpportunityStages);
  parameterWidget()->append(tr("Stage Pattern"), "opstage_pattern",    ParameterWidget::Text);
  parameterWidget()->append(tr("Create Date on or After"), "startCreateDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Create Date on or Before"),   "endCreateDate",   ParameterWidget::Date);

  setupCharacteristics("OPP");
}

void opportunityList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  bool editPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("ophead_owner_username") && _privileges->check("MaintainPersonalOpportunities")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("ophead_username") && _privileges->check("MaintainPersonalOpportunities")) ||
      (_privileges->check("MaintainAllOpportunities"));

  bool viewPriv =
      (omfgThis->username() == list()->currentItem()->rawValue("ophead_owner_username") && _privileges->check("ViewPersonalOpportunities")) ||
      (omfgThis->username() == list()->currentItem()->rawValue("ophead_username") && _privileges->check("ViewPersonalOpportunities")) ||
      (_privileges->check("ViewAllOpportunities"));

  menuItem = pMenu->addAction(tr("New..."), this, SLOT(sNew()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(editPriv);

  pMenu->addSeparator();

  if (list()->altId() == 0)
  {
    menuItem = pMenu->addAction(tr("Deactivate"), this, SLOT(sDeactivate()));
    menuItem->setEnabled(editPriv);
  }
  else
  {
    menuItem = pMenu->addAction(tr("Activate"), this, SLOT(sActivate()));
    menuItem->setEnabled(editPriv);
  }
}

enum SetResponse opportunityList::set(const ParameterList& pParams)
{
  XWidget::set(pParams);

  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void opportunityList::sNew()
{
  ParameterList params;
  setParams(params);
  params.append("mode","new");

  opportunity newdlg(0, "", true);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityList::sEdit()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("ophead_id", item->id());

    opportunity* newdlg = new opportunity(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
}

void opportunityList::sView()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("ophead_id", item->id());

    opportunity* newdlg = new opportunity(0, "", false);
    newdlg->set(params);
    newdlg->setAttribute(Qt::WA_DeleteOnClose);
    newdlg->show();
  }
}

void opportunityList::sDelete()
{
  XSqlQuery opportunityDelete;
  opportunityDelete.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    opportunityDelete.bindValue(":ophead_id", item->id());
    opportunityDelete.exec();
    if (opportunityDelete.first())
    {
      int result = opportunityDelete.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Opportunity"),
                             storedProcErrorLookup("deleteOpportunity", result),
                             __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Opportunity"),
                                opportunityDelete, __FILE__, __LINE__))
    {
      return;
    }
  }

  sFillList();
}

void opportunityList::sDeactivate()
{
  XSqlQuery opportunityDeactivate;
  opportunityDeactivate.prepare("UPDATE ophead SET ophead_active=false WHERE ophead_id=:ophead_id;");
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    opportunityDeactivate.bindValue(":ophead_id", item->id());
    opportunityDeactivate.exec();
    if (opportunityDeactivate.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deactiving Opportunity"),
                           opportunityDeactivate, __FILE__, __LINE__);
  }

  sFillList();
}

void opportunityList::sActivate()
{
  XSqlQuery opportunityActivate;
  opportunityActivate.prepare("UPDATE ophead SET ophead_active=true WHERE ophead_id=:ophead_id;");
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    opportunityActivate.bindValue(":ophead_id", item->id());
    opportunityActivate.exec();
    if (opportunityActivate.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Activating Opportunity"),
                           opportunityActivate, __FILE__, __LINE__);
  }

  sFillList();
}

bool opportunityList::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (!findChild<QCheckBox*>("_showInactive")->isChecked())
    params.append("activeOnly");

  return true;
}

void opportunityList::sOpen()
{
  if (list()->id() > 0)
  {
    bool editPriv =
    (omfgThis->username() == list()->currentItem()->rawValue("ophead_owner_username") && _privileges->check("MaintainPersonalOpportunities")) ||
    (omfgThis->username() == list()->currentItem()->rawValue("ophead_username") && _privileges->check("MaintainPersonalOpportunities")) ||
    (_privileges->check("MaintainAllOpportunities"));
    
    bool viewPriv =
    (omfgThis->username() == list()->currentItem()->rawValue("ophead_owner_username") && _privileges->check("ViewPersonalOpportunities")) ||
    (omfgThis->username() == list()->currentItem()->rawValue("ophead_username") && _privileges->check("ViewPersonalOpportunities")) ||
    (_privileges->check("ViewAllOpportunities"));
    
    if (editPriv)
      sEdit();
    else if (viewPriv)
      sView();
  }
}


