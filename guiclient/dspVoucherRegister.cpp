/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspVoucherRegister.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include "voucher.h"
#include "miscVoucher.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "glTransactionDetail.h"

#define USERNAME_COL	9
/*
 *  Constructs a dspVoucherRegister as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspVoucherRegister::dspVoucherRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedAccount, SIGNAL(toggled(bool)), _account, SLOT(setEnabled(bool)));
  connect(_showUsername, SIGNAL(toggled(bool)), this, SLOT(sShowUsername(bool)));

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
  menuThis->insertItem(tr("View..."), this, SLOT(sViewTrans()), 0);

  QTreeWidgetItem * item = _gltrans->currentItem();
  if(0 == item)
    return;

  if(item->text(2) == "VO")
    menuThis->insertItem(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->text(2) == "IN")
    menuThis->insertItem(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->text(2) == "PO")
    menuThis->insertItem(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
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
  QString sql( "SELECT gltrans_id, gltrans_date,"
               "       vend_number, vend_name,"
               "       gltrans_doctype, gltrans_docnumber, firstLine(gltrans_notes) AS reference,"
               "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
               "       CASE WHEN (gltrans_amount < 0) THEN ABS(gltrans_amount)"
               "            ELSE 0"
               "       END AS debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
               "            ELSE 0"
               "       END AS credit,"
               "       gltrans_username,"
               "       'curr' AS debit_xtnumericrole,"
               "       'curr' AS credit_xtnumericrole "
               "FROM accnt, gltrans LEFT OUTER JOIN vohead JOIN vend"
               "      ON (vohead_vend_id=vend_id)"
               "      ON (gltrans_doctype='VO' and gltrans_docnumber=vohead_number) "
               "WHERE ((gltrans_accnt_id=accnt_id)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate)"
               " AND (gltrans_source='A/P')" );

  if (_selectedAccount->isChecked())
    sql += " AND (gltrans_accnt_id=:accnt_id)";

  sql += ") "
         "ORDER BY gltrans_created DESC, gltrans_sequence, gltrans_amount;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":accnt_id", _account->id());
  q.exec();
  _gltrans->populate(q);
}

void dspVoucherRegister::sShowUsername( bool yes )
{
  if(yes)
    _gltrans->showColumn(USERNAME_COL);
  else
    _gltrans->hideColumn(USERNAME_COL);
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

