/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apWorkBench.h"

#include <QSqlError>
 
#include <metasql.h>
#include "openVouchers.h"
#include "selectPayments.h"
#include "selectedPayments.h"
#include "viewCheckRun.h"
#include "unappliedAPCreditMemos.h"
#include "dspVendorAPHistory.h"

apWorkBench::apWorkBench(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QWidget *hideme = 0;

  _vouchers = new openVouchers(this, "openVouchers", Qt::Widget);
  _vouchersTab->layout()->addWidget(_vouchers);
  hideme = _vouchers->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _vouchers->show();
  _vendorgroup->synchronize((VendorGroup*)(_vouchers->findChild<QWidget*>("_vendorgroup")));

  _payables = new selectPayments(this, "selectPayments", Qt::Widget, false);
  _payablesTab->layout()->addWidget(_payables);
  hideme = _payables->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _payables->show();
  _vendorgroup->synchronize((VendorGroup*)(_payables->findChild<QWidget*>("_vendorgroup")));

  _credits = new unappliedAPCreditMemos(this, "creditMemos", Qt::Widget);
  _creditsTab->layout()->addWidget(_credits);
  hideme = _credits->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _credits->show();
  _vendorgroup->synchronize((VendorGroup*)(_credits->findChild<QWidget*>("_vendorgroup")));

  _selectedPayments = new selectedPayments(this, "selectedPayments", Qt::Widget);
  _selectionsTab->layout()->addWidget(_selectedPayments);
  hideme = _selectedPayments->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _selectedPayments->show();
  _vendorgroup->synchronize((VendorGroup*)(_selectedPayments->findChild<QWidget*>("_vendorgroup")));

  QWidget * _checkRun = new viewCheckRun(this, "viewCheckRun", Qt::Widget);
  _checkRunTab->layout()->addWidget(_checkRun);
  hideme = _checkRun->findChild<QWidget*>("_close");
  if (hideme)
    hideme->hide();
  _checkRun->setWindowFlags(Qt::Widget);
  _checkRun->show();
  _vendorgroup->synchronize((VendorGroup*)(_checkRun->findChild<QWidget*>("_vendorgroup")));
  if (!_privileges->check("MaintainPayments"))
    _checkRun->setEnabled(false);

  if (_privileges->check("ViewAPOpenItems"))
  {
    _history = new dspVendorAPHistory(this, "dspVendorAPHistory", Qt::Widget);
    _apHistoryTab->layout()->addWidget(_history);
    _history->setCloseVisible(false);
    _history->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
    _history->findChild<DateCluster*>("_dates")->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
    _history->show();
    _vendorgroup->synchronize(_history->findChild<VendorGroup*>("_vend"));
  }
  else
    _apHistoryTab->setEnabled(false);

  connect(_query, SIGNAL(clicked()), _vouchers, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _payables, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _credits, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _selectedPayments, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _checkRun, SLOT(sFillList()));
  connect(_query, SIGNAL(clicked()), _history,  SLOT(sFillList()));

  _payables->sFillList();
}

apWorkBench::~apWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void apWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apWorkBench::set(const ParameterList & pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool    valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendorgroup->setVendId(param.toInt());

  return NoError;
}

