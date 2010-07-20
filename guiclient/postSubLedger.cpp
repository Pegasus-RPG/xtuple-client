/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its postSubLedger.
 */

#include "postSubLedger.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QMessageBox>
#include <QSqlError>

postSubLedger::postSubLedger(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _post = _buttonBox->button(QDialogButtonBox::Ok);
  _post->setText(tr("&Post"));
  _post->setEnabled(false);
  _query = _buttonBox->addButton(tr("Query"),QDialogButtonBox::ActionRole);
  _subLedgerDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _subLedgerDates->setEndNull(tr("Laest"), omfgThis->endOfTime(), true);
  _distDate->setDate(omfgThis->dbDate());

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sPost()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sources, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  _sources->addColumn(tr("Source"), -1, Qt::AlignLeft, true, "sltrans_source");
  _sources->addColumn(tr("Amount"), -1, Qt::AlignRight, true, "amount");
  _sources->addColumn(tr("Journals"), -1, Qt::AlignRight, true, "journals");

  sFillList();
}

postSubLedger::~postSubLedger()
{
}

void postSubLedger::languageChange()
{
  retranslateUi(this);
}

bool postSubLedger::sCheck()
{
  return false;
}

void postSubLedger::sPost()
{
  close();
}

void postSubLedger::sFillList()
{
  MetaSQLQuery mql = mqlLoad("postSubLedger", "detail");
  ParameterList params;
  _subLedgerDates->appendValue(params);

  XSqlQuery qry;
  qry = mql.toQuery(params);
  _sources->populate(qry, true);
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postSubLedger::sSelectionChanged()
{
  bool hasSelections = _sources->selectedItems().count();
  _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasSelections);
}
