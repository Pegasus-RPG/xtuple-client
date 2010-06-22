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

#include <QMessageBox>

postSubLedger::postSubLedger(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(post()));

  _buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Post"));
  _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  _subLedgerDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _subLedgerDates->setEndNull(tr("Laest"), omfgThis->endOfTime(), true);
  _distDate->setDate(omfgThis->dbDate());

  populate();
}

postSubLedger::~postSubLedger()
{
}

void postSubLedger::languageChange()
{
  retranslateUi(this);
}

bool postSubLedger::check()
{
  return false;
}

void postSubLedger::post()
{

}

void postSubLedger::populate()
{

}

void postSubLedger::selectionChanged()
{
  bool hasSelections = _sources->selectedItems().count();
  _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasSelections);
}
