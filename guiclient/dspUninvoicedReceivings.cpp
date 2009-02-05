/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUninvoicedReceivings.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "enterPoitemReceipt.h"
#include "poLiabilityDistrib.h"
#include "postPoReturnCreditMemo.h"

dspUninvoicedReceivings::dspUninvoicedReceivings(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_agent, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_porecv, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _porecv->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter, true, "thedate");
  _porecv->addColumn(tr("By"),          _orderColumn, Qt::AlignCenter, true, "f_user" );
  _porecv->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignLeft,   true, "ponumber");
  _porecv->addColumn(tr("#"),           _whsColumn,   Qt::AlignCenter, true, "poitem_linenumber");
  _porecv->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft,   true, "vend_name");
  _porecv->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true, "itemnumber");
  _porecv->addColumn(tr("Uninvoiced"),  _qtyColumn,   Qt::AlignRight,  true, "qty");
  _porecv->addColumn(tr("Type"),        _itemColumn,  Qt::AlignLeft,   true, "type"); 
  _porecv->addColumn(tr("Value"),       _moneyColumn, Qt::AlignRight,  true, "value");

  sFillList();
}

dspUninvoicedReceivings::~dspUninvoicedReceivings()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspUninvoicedReceivings::languageChange()
{
  retranslateUi(this);
}

void dspUninvoicedReceivings::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  if(_privileges->check("MaintainUninvoicedReceipts"))
  {
    if(_porecv->altId() < 3)
    {
      menuItem = pMenu->insertItem(tr("Mark as Invoiced..."), this, SLOT(sMarkAsInvoiced()), 0);
      menuItem = pMenu->insertItem(tr("Correct Receiving..."), this, SLOT(sCorrectReceiving()), 0);
      if(_porecv->altId() == 2)
        pMenu->setItemEnabled(menuItem, false);
    }
    if (_porecv->altId() == 3)
      menuItem = pMenu->insertItem(tr("Create Credit Memo..."), this, SLOT(sCreateCreditMemo()), 0);
  }
}

void dspUninvoicedReceivings::sMarkAsInvoiced()
{
  bool update = TRUE;
  
  q.prepare("SELECT * FROM porecv "
            "WHERE ((porecv_value <> 0) "
            "AND (porecv_id=:porecv_id));");
  q.bindValue(":porecv_id",_porecv->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("porecv_id", _porecv->id());  
    poLiabilityDistrib newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Rejected)
      update = FALSE;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (update)
  {
    q.prepare("UPDATE recv "
	      "SET recv_invoiced=true "
	      "WHERE (recv_id=:porecv_id); ");
    q.bindValue(":porecv_id",_porecv->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

void dspUninvoicedReceivings::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(_porecv->id(), this) != XDialog::Rejected)
    sFillList();
}

void dspUninvoicedReceivings::sCreateCreditMemo()
{
  ParameterList params;
  params.append("poreject_id", _porecv->id());

  postPoReturnCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

bool dspUninvoicedReceivings::setParams(ParameterList &pParams)
{
  _warehouse->appendValue(pParams);
  if(_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());
  return true;
}

void dspUninvoicedReceivings::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("UninvoicedReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUninvoicedReceivings::sFillList()
{
  _porecv->clear();
  QString sql( "SELECT recv_id AS id,"
               "       CASE WHEN(poitem_status='C') THEN 2"
               "            ELSE 1"
               "       END AS doctype,"
               "       recv_date AS thedate,"
               "       recv_trans_usr_name AS f_user,"
               "       recv_order_number AS ponumber, poitem_linenumber,"
               "       vend_name,"
               "       COALESCE(item_number,"
               "       ('Misc. - ' || recv_vend_item_number)) AS itemnumber,"
               "       recv_qty AS qty, 'qty' AS qty_xtnumericrole,"
               "       'Receipt' AS type, "
               "       recv_value AS value,"
               "       'curr' AS value_xtnumericrole, 0 AS value_xttotalrole "
               "FROM recv, vend, poitem LEFT OUTER JOIN"
               "                   ( itemsite JOIN item"
               "                     ON (itemsite_item_id=item_id)"
               "                   ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ( (recv_orderitem_id=poitem_id)"
               " AND (recv_order_type='PO')"
               " AND (recv_vend_id=vend_id)"
               " AND (recv_posted)"
               " AND (recv_vohead_id IS NULL)"
               " AND (NOT recv_invoiced) "
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"agentUsername\") ?>"
	       " AND (recv_agent_username=<? value(\"agentUsername\") ?>)"
	       "<? endif ?>"
	       ") "
	       "UNION "
               "SELECT poreject_id AS id,"
               "       3 AS doctype,"
               "       poreject_date AS thedate,"
               "       poreject_trans_username AS f_user,"
               "       poreject_ponumber AS ponumber, poitem_linenumber,"
               "       vend_name,"
               "       COALESCE(item_number,"
               "       ('Misc. - ' || poreject_vend_item_number)) AS itemnumber,"
               "       poreject_qty, 'qty' AS qty_xtnumericrole,"
               "       'Return' AS type, "
               "       poreject_value * -1 AS value,"
               "       'curr' AS value_xtnumericrole, 0 AS value_xtotalrole "
               "FROM poreject, vend, poitem LEFT OUTER JOIN"
               "                   ( itemsite JOIN item"
               "                     ON (itemsite_item_id=item_id)"
               "                   ) ON (poitem_itemsite_id=itemsite_id) "
               "WHERE ( (poreject_poitem_id=poitem_id)"
               " AND (poreject_vend_id=vend_id)"
               " AND (poreject_posted)"
               " AND (NOT poreject_invoiced) "
	       "<? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       "<? endif ?>"
	       "<? if exists(\"agentUsername\") ?>"
	       " AND (poreject_agent_username=<? value(\"agentUsername\") ?>)"
	       "<? endif ?>"
	       ") "
	       "ORDER BY ponumber, poitem_linenumber;");

  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _porecv->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
