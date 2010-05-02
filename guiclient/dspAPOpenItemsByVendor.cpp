/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAPOpenItemsByVendor.h"

#include <QVariant>
#include <QSqlError>
#include <QMessageBox>
#include <QMenu>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
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
  connect(_apopen, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _apopen->addColumn(tr("Doc. Type"),    _orderColumn,    Qt::AlignLeft,   true,  "f_doctype" );
  _apopen->addColumn(tr("Doc. #"),       _orderColumn,    Qt::AlignLeft,   true,  "apopen_docnumber"  );
  _apopen->addColumn(tr("Vendor#"),      _orderColumn,    Qt::AlignLeft,   true,  "vend_number"  );
  _apopen->addColumn(tr("Name"),         -1,              Qt::AlignLeft,   true,  "vend_name"  );
  _apopen->addColumn(tr("P/O #"),        _orderColumn,    Qt::AlignLeft,   true,  "apopen_ponumber"  );
  _apopen->addColumn(tr("Invoice #"),    _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"  );
  _apopen->addColumn(tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter, false, "apopen_docdate" );
  _apopen->addColumn(tr("Due Date"),     _dateColumn,     Qt::AlignCenter, true,  "apopen_duedate" );
  _apopen->addColumn(tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "apopen_amount"  );
  _apopen->addColumn(tr("Paid"),         _moneyColumn,    Qt::AlignRight,  true,  "paid"  );
  _apopen->addColumn(tr("Balance"),      -1,              Qt::AlignRight,  true,  "balance"  );
  _apopen->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  _apopen->addColumn(tr("Base Balance"), _moneyColumn,    Qt::AlignRight,  true,  "base_balance"  );
  _apopen->addColumn(tr("Status"),       _moneyColumn,    Qt::AlignCenter, false, "apopen_status"  );

  if (omfgThis->singleCurrency())
  {
    _apopen->hideColumn(10);
    _apopen->hideColumn(11);
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
    _apopen->headerItem()->setText(12, tr("Balance\n(in %1)").arg(currConcat));
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _vendorGroup->setFocus();
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
    _vendorGroup->setVendId(param.toInt());

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

void dspAPOpenItemsByVendor::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected)
{
  QString status(selected->text(1));
   
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("EditAPOpenItem"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  XSqlQuery menu;
  menu.prepare("SELECT apopen_status FROM apopen WHERE apopen_id=:apopen_id;");
  menu.bindValue(":apopen_id", _apopen->id());
  menu.exec();
  if (menu.first())
  {
    if(menu.value("apopen_status").toString() == "O")
	{
    menuItem = pMenu->insertItem(tr("On Hold"), this, SLOT(sOnHold()), 0);
	  if (_privileges->check("EditAPOpenItem"))
	  	pMenu->setItemEnabled(menuItem, TRUE);
	  else
      pMenu->setItemEnabled(menuItem, FALSE);
    }
    if(menu.value("apopen_status").toString() == "H") 
	{
    menuItem = pMenu->insertItem(tr("Open"), this, SLOT(sOpen()), 0);
	  if (_privileges->check("EditAPOpenItem"))
	  	pMenu->setItemEnabled(menuItem, TRUE);
	  else
	  pMenu->setItemEnabled(menuItem, FALSE);
	}	
  }
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
  _vendorGroup->appendValue(params);
  params.append("asofDate", _asOf->date());
  _dates->appendValue(params);
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("voucher", tr("Voucher"));
  params.append("includeFormatted", true);

  orReport report("APOpenItemsByVendor", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAPOpenItemsByVendor::sFillList()
{
  _apopen->clear();

  MetaSQLQuery mql = mqlLoad("apOpenItems", "detail");
  ParameterList params;
  _vendorGroup->appendValue(params);
  params.append("asofDate", _asOf->date());
  _dates->appendValue(params);
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("voucher", tr("Voucher"));
  XSqlQuery qry;
  qry = mql.toQuery(params);
  _apopen->populate(qry, true);
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspAPOpenItemsByVendor::sOpen()
{
  XSqlQuery open;
  open.prepare("UPDATE apopen SET apopen_status = 'O' WHERE apopen_id=:apopen_id;");
  open.bindValue(":apopen_id", _apopen->id());
  open.exec();
  sFillList();
}

void dspAPOpenItemsByVendor::sOnHold()
{
  XSqlQuery selectpayment;
  selectpayment.prepare("SELECT * FROM apselect WHERE apselect_apopen_id = :apopen_id;");
  selectpayment.bindValue(":apopen_id", _apopen->id());
  selectpayment.exec();
  if (selectpayment.first())
  {
    QMessageBox::critical( this, tr("Can not change Status"), tr( "You cannot set this item as On Hold.\nThis Item is already selected for payment." ) );
    return;
  }
  
  XSqlQuery onhold;
  onhold.prepare("UPDATE apopen SET apopen_status = 'H' WHERE apopen_id=:apopen_id;");
  onhold.bindValue(":apopen_id", _apopen->id());
  onhold.exec();
  sFillList();
}
