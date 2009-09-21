/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAPOpenItemsByVendor.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "apOpenItem.h"

/*
 *  Constructs a dspAPOpenItemsByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspAPOpenItemsByVendor::dspAPOpenItemsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_apopen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_vend, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _apopen->addColumn(tr("Doc. Type"),    -1,              Qt::AlignCenter, true,  "f_doctype" );
  _apopen->addColumn(tr("Doc. #"),       _orderColumn,    Qt::AlignRight,  true,  "apopen_docnumber"  );
  _apopen->addColumn(tr("P/O #"),        _orderColumn,    Qt::AlignRight,  true,  "apopen_ponumber"  );
  _apopen->addColumn(tr("Invoice #"),    _orderColumn,    Qt::AlignRight,  true,  "invoicenumber"  );
  _apopen->addColumn(tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter, true,  "apopen_docdate" );
  _apopen->addColumn(tr("Due Date"),     _dateColumn,     Qt::AlignCenter, true,  "apopen_duedate" );
  _apopen->addColumn(tr("Amount"),       _bigMoneyColumn, Qt::AlignRight,  true,  "apopen_amount"  );
  _apopen->addColumn(tr("Paid"),         _bigMoneyColumn, Qt::AlignRight,  true,  "paid"  );
  _apopen->addColumn(tr("Balance"),      _bigMoneyColumn, Qt::AlignRight,  true,  "balance"  );
  _apopen->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  _apopen->addColumn(tr("Base Balance"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_balance"  );

  if (omfgThis->singleCurrency())
  {
    _apopen->hideColumn(8);
    _apopen->hideColumn(9);
  }
  else
  {
    q.prepare("SELECT currConcat(baseCurrId()) AS currConcat;");
    q.exec();
    QString currConcat;
    if (q.first())
      currConcat = q.value("currConcat").toString();
    else
      currConcat = tr("?????");
    _apopen->headerItem()->setText(10, tr("Balance\n(in %1)").arg(currConcat));
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _vend->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspAPOpenItemsByVendor::~dspAPOpenItemsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspAPOpenItemsByVendor::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAPOpenItemsByVendor::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("asofDate", &valid);
  if (valid)
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(FALSE);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAPOpenItemsByVendor::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("EditAPOpenItem"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspAPOpenItemsByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", _apopen->id());
  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apopen->id());
  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sPrint()
{
  ParameterList params;
  params.append("vend_id", _vend->id());
  params.append("asofDate", _asOf->date());
  _dates->appendValue(params);
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("voucher", tr("Voucher"));

  orReport report("APOpenItemsByVendor", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAPOpenItemsByVendor::sFillList()
{
  _apopen->clear();

  q.prepare( "SELECT apopen_id, apopen_ponumber, apopen_docnumber,"
             "       CASE WHEN (apopen_doctype='C') THEN :creditMemo"
             "            WHEN (apopen_doctype='D') THEN :debitMemo"
             "            WHEN (apopen_doctype='V') THEN :voucher"
             "            ELSE :other"
             "       END AS f_doctype,"
             "       apopen_invcnumber AS invoicenumber,"
             "       apopen_docdate, apopen_duedate, apopen_amount,"
             "       apopen_paid - COALESCE(SUM(apapply_target_paid),0) AS paid,"
             "       (apopen_amount - apopen_paid + COALESCE(SUM(apapply_target_paid),0)) * "
             "       CASE WHEN apopen_doctype IN ('D', 'V') THEN 1 ELSE -1 "
             "       END AS balance,"
             "       currConcat(apopen_curr_id) AS currAbbr,"
             "       (apopen_amount - apopen_paid + COALESCE(SUM(apapply_target_paid),0)) "
             "       / apopen_curr_rate * (CASE WHEN apopen_doctype IN ('D', 'V') THEN 1 ELSE -1 "
             "            END) AS base_balance,"
             "       'curr' AS apopen_amount_xtnumericrole,"
             "       'curr' AS paid_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole,"
             "       'curr' AS base_balance_xtnumericrole,"
             "       0 AS base_balance_xttotalrole "
             "FROM apopen "
             "  LEFT OUTER JOIN apapply ON (((apopen_id=apapply_target_apopen_id) "
             "                          OR (apopen_id=apapply_source_apopen_id)) "
             "                          AND (apapply_postdate > :asofdate)) "
             " WHERE ( (COALESCE(apopen_closedate,date :asofdate + integer '1')>:asofdate) "
             "   AND   (apopen_docdate<=:asofdate)"
             "   AND   (apopen_vend_id=:vend_id) "
             "   AND   (apopen_duedate BETWEEN :startDate AND :endDate) ) "
             " GROUP BY apopen_id, apopen_ponumber, apopen_docnumber,apopen_doctype, apopen_invcnumber, apopen_docdate, "
             "   apopen_duedate, apopen_docdate, apopen_amount, apopen_paid, apopen_curr_id, apopen_curr_rate "
             " ORDER BY apopen_docdate;" );
  _dates->bindValue(q);
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":creditMemo", tr("Credit Memo"));
  q.bindValue(":debitMemo", tr("Debit Memo"));
  q.bindValue(":voucher", tr("Voucher"));
  q.bindValue(":asofdate", _asOf->date());
  q.exec();
  if (q.first())
    _apopen->populate(q);
}

