/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspVoucherRegister.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "glTransactionDetail.h"
#include "invoice.h"
#include "miscVoucher.h"
#include "mqlutil.h"
#include "purchaseOrder.h"
#include "voucher.h"

dspVoucherRegister::dspVoucherRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedAccount, SIGNAL(toggled(bool)), _account, SLOT(setEnabled(bool)));
  connect(_showUsername, SIGNAL(toggled(bool)), this, SLOT(sShowUsername(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _gltrans->addColumn(tr("Date"),        _dateColumn,    Qt::AlignCenter, true, "gltrans_date" );
  _gltrans->addColumn(tr("Vend. #"),     _orderColumn,   Qt::AlignRight,  true, "vend_number"  );
  _gltrans->addColumn(tr("Vend. Name"),  _itemColumn,    Qt::AlignLeft,   true, "vend_name"   );
  _gltrans->addColumn(tr("Doc. Type"),   _docTypeColumn, Qt::AlignCenter, true, "gltrans_doctype" );
  _gltrans->addColumn(tr("Doc. #"),      _orderColumn,   Qt::AlignCenter, true, "gltrans_docnumber" );
  _gltrans->addColumn(tr("Reference"),   -1,             Qt::AlignLeft,   true, "reference"   );
  _gltrans->addColumn(tr("Account"),     _itemColumn,    Qt::AlignLeft,   true, "account"   );
  _gltrans->addColumn(tr("Debit"),       _moneyColumn,   Qt::AlignRight,  true, "debit"  );
  _gltrans->addColumn(tr("Credit"),      _moneyColumn,   Qt::AlignRight,  true, "credit"  );
  _gltrans->addColumn(tr("Username"),    _userColumn,    Qt::AlignLeft,   true, "gltrans_username" );

  sShowUsername(_showUsername->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspVoucherRegister::~dspVoucherRegister()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspVoucherRegister::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspVoucherRegister::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _selectedAccount->setChecked(TRUE);
    _account->setId(param.toInt());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspVoucherRegister::sPopulateMenu(QMenu * menuThis)
{
  menuThis->addAction(tr("View..."), this, SLOT(sViewTrans()));

  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(_gltrans->currentItem());
  if(0 == item)
    return;
  QString doctype = item->rawValue("gltrans_doctype").toString();

  if (doctype == "VO")
    menuThis->addAction(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if (doctype == "IN")
    menuThis->addAction(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if (doctype == "PO")
    menuThis->addAction(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
}

void dspVoucherRegister::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);

  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());

  if (_showUsername->isChecked())
    params.append("showUsernames");

  orReport report("VoucherRegister", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspVoucherRegister::sFillList()
{
  MetaSQLQuery mql = mqlLoad("voucherRegister", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _gltrans->populate(q);
}

bool dspVoucherRegister::setParams(ParameterList & params)
{
  if (! _dates->startDate().isValid())
  {
    QMessageBox::warning(this, tr("Invalid Date"),
                          tr("Enter a valid Start Date."));
	_dates->setFocus();
    return false;
  }

  if (! _dates->endDate().isValid())
  {
    QMessageBox::warning(this, tr("Invalid Date"),
                          tr("Enter a valid End Date."));
    _dates->setFocus();
    return false;
  }

  if (_selectedAccount->isChecked())
    if (! _account->isValid())
    {
      QMessageBox::warning(this, tr("Invalid Account"),
                           tr("Enter a valid Account."));
      _account->setFocus();
      return false;
    }

  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());
  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());
  return true;
}

void dspVoucherRegister::sShowUsername( bool yes )
{
  if(yes)
    _gltrans->showColumn(_gltrans->column("gltrans_username"));
  else
    _gltrans->hideColumn(_gltrans->column("gltrans_username"));
}

void dspVoucherRegister::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->id());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspVoucherRegister::sViewDocument()
{
  QTreeWidgetItem * item = _gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->text(2) == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc"
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->text(3));
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");

    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else if(item->text(2) == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->text(3));
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->text(2) == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->text(3));
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

