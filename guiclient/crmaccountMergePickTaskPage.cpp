/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <metasql.h>
#include <mqlutil.h>

#include "crmaccountMerge.h"
#include "crmaccountMergePickTaskPage.h"
#include "errorReporter.h"

CrmaccountMergePickTaskPage::CrmaccountMergePickTaskPage(QWidget *parent)
  : QWizardPage(parent)
{
  setupUi(this);

  connect(_continueExisting,SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_existingMerge,     SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_revertExisting,  SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_startPurge,      SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));

  registerField("_completedMerge", _completedMerge, "text", "currentIndexChanged(QString)");
  registerField("_existingMerge",  _existingMerge,  "text", "currentIndexChanged(QString)");
}

void CrmaccountMergePickTaskPage::initializePage()
{
  qDebug("CrmaccountMergePickTaskPage::initializePage() entered");
  XSqlQuery taskq("SELECT crmacctsel_dest_crmacct_id, crmacct_number"
                  "  FROM crmacctsel"
                  "  JOIN crmacct ON (crmacctsel_dest_crmacct_id=crmacct_id)"
                  " WHERE (crmacctsel_src_crmacct_id=crmacctsel_dest_crmacct_id)"
                  " ORDER BY crmacct_name;");
  _existingMerge->populate(taskq);
  ErrorReporter::error(QtCriticalMsg, this, tr("Looking for Merges"),
                       taskq, __FILE__, __LINE__);
}

bool CrmaccountMergePickTaskPage::isComplete() const
{
  if (_continueExisting->isChecked() || _revertExisting->isChecked())
    return _existingMerge->isValid();

  return true;
}

int CrmaccountMergePickTaskPage::nextId() const
{
  if (_continueExisting->isChecked())    return crmaccountMerge::Page_PickData;
  else if (_revertExisting->isChecked()) return crmaccountMerge::Page_Result;
  else if (_startPurge->isChecked())     return crmaccountMerge::Page_Purge;

  return crmaccountMerge::Page_PickAccounts;
}

void CrmaccountMergePickTaskPage::sHandleButtons()
{
  _continueExisting->setEnabled(_existingMerge->count());
  _revertExisting->setEnabled(_existingMerge->count());
  _startPurge->setEnabled(_existingMerge->count());

  _existingMerge->setEnabled(_continueExisting->isChecked() ||
                             _revertExisting->isChecked());

  emit completeChanged();
}
