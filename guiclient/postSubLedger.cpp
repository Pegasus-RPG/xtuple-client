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
  _selectAll = _buttonBox->addButton(tr("Select All"), QDialogButtonBox::ActionRole);
  _subLedgerDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _subLedgerDates->setEndNull(tr("Laest"), omfgThis->endOfTime(), true);
  _distDate->setDate(omfgThis->dbDate());

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sPost()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectAll, SIGNAL(clicked()), _sources, SLOT(selectAll()));
  connect(_preview, SIGNAL(toggled(bool)), this, SLOT(sHandlePreview()));
  connect(_sources, SIGNAL(valid(bool)), this, SLOT(sHandleSelection()));

  _sources->addColumn(tr("Source"), _docTypeColumn, Qt::AlignLeft, true, "sltrans_source");
  _sources->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "description");
  sHandlePreview();

  sFillList();
}

postSubLedger::~postSubLedger()
{
}

void postSubLedger::languageChange()
{
  retranslateUi(this);
}

void postSubLedger::sPost()
{
  XSqlQuery qry;
  MetaSQLQuery mql = mqlLoad("postSubLedger", "post");
  QList<XTreeWidgetItem*> selected = _sources->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    _subLedgerDates->appendValue(params);
    params.append("distDate", _distDate->date());
    params.append("source", selected.at(i)->rawValue("sltrans_source").toString());

    mql.toQuery(params);
    if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  close();
}

void postSubLedger::sFillList()
{
  MetaSQLQuery mql = mqlLoad("postSubLedger", "detail");
  ParameterList params;
  _subLedgerDates->appendValue(params);
  params.append("AP", tr("Accounts Payable"));
  params.append("AR", tr("Accounts Receivable"));
  params.append("GL", tr("General Ledger"));
  params.append("IM", tr("Inventory Management"));
  params.append("PD", tr("Products"));
  params.append("PO", tr("Purchase Order"));
  params.append("SO", tr("Sales Order"));
  params.append("SR", tr("Shipping and Receiving"));
  params.append("WO", tr("Work Order"));
  params.append("Other", tr("Other"));
  if (_preview->isChecked())
    params.append("preview");

  XSqlQuery qry;
  qry = mql.toQuery(params);
  _sources->populate(qry, true);
  _sources->expandAll();
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postSubLedger::sHandlePreview()
{
  _sources->clear();
  _sources->setColumnCount(2);
  if (_preview->isChecked())
  {
    _sources->addColumn(tr("Debit"), _bigMoneyColumn, Qt::AlignRight, true, "debit");
    _sources->addColumn(tr("Credit"),_bigMoneyColumn, Qt::AlignRight, true, "credit");
    _sources->setSelectionMode(QAbstractItemView::SingleSelection);
    _selectAll->setEnabled(false);
  }
  else
  {
    _sources->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _selectAll->setEnabled(true);
  }

  _sources->addColumn(tr("Journals"), _qtyColumn, Qt::AlignRight, true, "journals");
}

void postSubLedger::sHandleSelection()
{
  _post->setEnabled(_sources->id() == 0);
}

